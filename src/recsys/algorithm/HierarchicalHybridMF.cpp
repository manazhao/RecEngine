/*
 * HierarchicalHybridMF.cpp
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/timer.hpp>
#include <vb/prob/DistParamBundle.h>
#include <algorithm>
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace bf = boost::filesystem ;
namespace po = boost::program_options;

namespace recsys {

ostream& operator <<(ostream& oss, HierarchicalHybridMF::RunTimeLog const& rhs) {
	oss << "iteration:" << rhs.m_iter << ",time:" << rhs.m_iter_time
			<< ",train rmse:" << rhs.m_train_rmse << ",test rmse:"
			<< rhs.m_test_rmse << ",cs rmse:" << rhs.m_cs_rmse << endl;
	return oss;
}

void HierarchicalHybridMF::_init() {
	/// allocate space for model variables
	_prepare_model_variables();
	/// we are all set, embark the fun journey!
}

void HierarchicalHybridMF::_prepare_model_variables() {
	cout << m_model_param << "\n\n";

	/// initialize entity latent variables
	m_entity.reserve(
			m_dataset_manager->dataset(rt::DSType::DS_ALL).ent_type_interacts.size());
	for (size_t i = 0; i
			< m_dataset_manager->dataset(rt::DSType::DS_ALL).ent_type_interacts.size(); i++) {
		m_entity.push_back(DiagMVGaussian(vec(m_model_param.m_lat_dim,
				arma::fill::randn), vec(m_model_param.m_lat_dim,
				arma::fill::ones), false, true));
	}

	/// initialize prior variables
	m_user_prior_mean = DiagMVGaussian(
			vec(m_model_param.m_lat_dim, fill::zeros), (vec(
					m_model_param.m_lat_dim, fill::ones)), false, true);
	m_user_prior_cov = MVInverseGamma(vec(m_model_param.m_lat_dim, fill::ones)
			* 3, vec(m_model_param.m_lat_dim, fill::ones) * 3);
	m_item_prior_mean = m_user_prior_mean;
	m_item_prior_cov = m_user_prior_cov;
	m_feature_prior_mean = m_user_prior_mean;
	m_feature_prior_cov = m_user_prior_cov;

	/// rating variance, big variance
	m_rating_var = InverseGamma(3, 3);

	/// initialize bias as standard Gaussian
	float meanRating = _get_mean_rating();
	m_bias = Gaussian(meanRating, 1);

	/// get the number of features for each entity
	_get_entity_feature_cnt();
}

void HierarchicalHybridMF::_update_entity_from_prior(int64_t const& entityId,
		int8_t entityType) {
	DistParamBundle message(2);
	DistParam upCovSuff2 =
			(entityType == Entity::ENT_USER ? m_user_prior_cov.suff_mean(2)
					: m_item_prior_cov.suff_mean(2));
	vec entityLatMean =
			(entityType == Entity::ENT_USER ? m_user_prior_mean.moment(1).m_vec
					: m_item_prior_mean.moment(1).m_vec);
	/// number of content features for current entity
	size_t entityFeatCnt = m_feat_cnt_map[entityId];
	/// add content feature prior information
	if (m_model_param.m_use_feature && entityFeatCnt > 0) {
		vec contentFeatPrior = 1 / sqrt(entityFeatCnt)
				* _entity_feature_mean_sum(entityId);
		entityLatMean += contentFeatPrior;
	}
	message[0] = (vec) (upCovSuff2.m_vec % (entityLatMean));
	message[1] = (vec) (-0.5 * upCovSuff2.m_vec);
	m_entity[entityId] += message;
}

void HierarchicalHybridMF::_update_entity_from_ratings(int64_t const& entityId,
		int8_t entityType, map<int8_t, vector<Interact> > & typeInteracts) {
	/// first reset the natural parameter of user vector
	/// update with rating feedback
	vector<Interact> & ratingInteracts =
			typeInteracts[EntityInteraction::RATE_ITEM];
	float rvsuff2 = (float) m_rating_var.suff_mean(2);
	/// update from ratings
	//// note: important to indicate the parameter as canonical form
	DistParamBundle message(2);
	for (vector<Interact>::iterator iter = ratingInteracts.begin(); iter
			< ratingInteracts.end(); ++iter) {
		Interact & tmpInteract = *iter;
		float tmpRating = tmpInteract.ent_val;
		int64_t itemId = tmpInteract.ent_id;
		DiagMVGaussian & itemLat = m_entity[itemId];
		float tmpRating1 = tmpRating - m_bias.moment(1);
		vec itemLatMean = itemLat.moment(1);
		vec itemLatCov = itemLat.moment(2);
		vec update1 = (tmpRating1 * rvsuff2 * itemLatMean);
		vec update2 = (-0.5 * rvsuff2 * itemLatCov);
		message[0] += update1;
		message[1] += update2;
	}
	if (ratingInteracts.size() > 0)
		m_entity[entityId] += message;
}

void HierarchicalHybridMF::_update_feature_from_prior(int64_t const& featId) {
	DistParamBundle updateMessage(2);
	/// consider feature prior
	DistParam featCovSuff2 = m_feature_prior_cov.suff_mean(2);
	updateMessage[0] = vec(featCovSuff2.m_vec
			% m_feature_prior_mean.moment(1).m_vec);
	updateMessage[1] = vec(-0.5 * featCovSuff2.m_vec);
	/// apply the update
	m_entity[featId] += updateMessage;
}

void HierarchicalHybridMF::_update_feature_from_entities(int64_t const& featId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	vector<Interact> const& interacts =
			typeInteracts[EntityInteraction::ADD_FEATURE];
	DistParamBundle updateMessage(2);
	//	DistParam featLatMean = m_entity[featId].moment(1);
	for (vector<Interact>::const_iterator iter = interacts.begin(); iter
			< interacts.end(); ++iter) {
		int64_t entityId = iter->ent_id;
		int8_t
				entityType =
						m_dataset_manager->dataset(rt::DSType::DS_TRAIN).m_id_type_map[entityId];
		assert(entityType > 0);
		DistParam entityCovSuff2 =
				(entityType == Entity::ENT_USER ? m_user_prior_cov.suff_mean(2)
						: m_item_prior_cov.suff_mean(2));
		size_t numFeats = m_feat_cnt_map[entityId];
		DiagMVGaussian & entityLat = m_entity[entityId];
		///
		vec tmpDiff = entityLat.moment(1).m_vec - (entityType
				== Entity::ENT_USER ? m_user_prior_mean.moment(1).m_vec
				: m_item_prior_mean.moment(1).m_vec);
		vec otherFeatureMean = _entity_feature_mean_sum(entityId) - m_entity[featId].moment(1).m_vec;
		tmpDiff -= (1 / sqrt(numFeats) * otherFeatureMean);
		updateMessage[0] += (1 / sqrt(numFeats) * tmpDiff
				% entityCovSuff2.m_vec);
		updateMessage[1] += (-0.5 / (float) numFeats * entityCovSuff2.m_vec);
	}
	if (interacts.size() > 0) {
		m_entity[featId] += updateMessage;
	}

}

void HierarchicalHybridMF::_update_entity(int64_t const& entityId,
		int8_t entityType, map<int8_t, vector<Interact> > & typeInteracts) {
	/// first reset the natural parameter of user vector
	m_entity[entityId].reset();
	_update_entity_from_ratings(entityId, entityType, typeInteracts);
	_update_entity_from_prior(entityId, entityType);
}

void HierarchicalHybridMF::_init_entity_feature_moment_cache() {
	/// update the user/item prior mean
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	set<int64_t>
			& itemIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> mergedIds;
	mergedIds.insert(mergedIds.end(), userIds.begin(), userIds.end());
	mergedIds.insert(mergedIds.end(), itemIds.begin(), itemIds.end());
	for (vector<int64_t>::const_iterator iter = mergedIds.begin(); iter
			< mergedIds.end(); ++iter) {
		m_feat_mean_sum[*iter] = vec(m_model_param.m_lat_dim, fill::zeros);
		m_feat_cov_sum[*iter] = vec(m_model_param.m_lat_dim
				* m_model_param.m_lat_dim, fill::zeros);
	}
}

void HierarchicalHybridMF::_update_entity_feature_moments() {
	/// update the user/item prior mean
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	set<int64_t>
			& itemIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> mergedIds;
	mergedIds.insert(mergedIds.end(), userIds.begin(), userIds.end());
	mergedIds.insert(mergedIds.end(), itemIds.begin(), itemIds.end());
	for (vector<int64_t>::const_iterator iter = mergedIds.begin(); iter
			< mergedIds.end(); ++iter) {
		m_feat_mean_sum[*iter].fill(0);
		m_feat_cov_sum[*iter].fill(0);
		vector<Interact> const
				& featureInteracts =
						m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[*iter][EntityInteraction::ADD_FEATURE];
		m_feat_cnt_map[*iter] = featureInteracts.size();
		for (vector<Interact>::const_iterator iter1 = featureInteracts.begin(); iter1
				< featureInteracts.end(); ++iter1) {
			int64_t featId = iter1->ent_id;
			m_feat_mean_sum[*iter] += m_entity[featId].moment(1).m_vec;
			/// second moment calculation is an approximate
			m_feat_cov_sum[*iter] += m_entity[featId].moment(2).m_vec;
		}
		for (size_t i = 0; i < featureInteracts.size(); i++) {
			int64_t featI = featureInteracts[i].ent_id;
			vec iMean = m_entity[featI].moment(1);
			for (size_t j = i + 1; j < featureInteracts.size(); j++) {
				int64_t featJ = featureInteracts[j].ent_id;
				vec jMean = m_entity[featJ].moment(1);
				m_feat_cov_sum[*iter] += vectorise(iMean * jMean.t() + jMean
						* iMean.t());
			}
		}
	}
}

vec HierarchicalHybridMF::_entity_feature_mean_sum(int64_t const& entityId) {
	vector<Interact> const
			& featureInteracts =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
	vec meanSum(m_model_param.m_lat_dim, fill::zeros);
	for (vector<Interact>::const_iterator iter1 = featureInteracts.begin(); iter1
			< featureInteracts.end(); ++iter1) {
		int64_t featId = iter1->ent_id;
		meanSum += m_entity[featId].moment(1).m_vec;
	}
	return meanSum;
}

vec HierarchicalHybridMF::_entity_feature_cov_sum(int64_t const& entityId) {
	vector<Interact> const
			& featureInteracts =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
	vec covSum(m_model_param.m_lat_dim * m_model_param.m_lat_dim, fill::zeros);
	for (vector<Interact>::const_iterator iter1 = featureInteracts.begin(); iter1
			< featureInteracts.end(); ++iter1) {
		int64_t featId = iter1->ent_id;
		/// second moment calculation is an approximate
		covSum += m_entity[featId].moment(2).m_vec;
	}
	for (size_t i = 0; i < featureInteracts.size(); i++) {
		int64_t featI = featureInteracts[i].ent_id;
		vec iMean = m_entity[featI].moment(1);
		for (size_t j = i + 1; j < featureInteracts.size(); j++) {
			int64_t featJ = featureInteracts[j].ent_id;
			vec jMean = m_entity[featJ].moment(1);
			covSum += vectorise(iMean * jMean.t() + jMean * iMean.t());
		}
	}
	return covSum;
}

void HierarchicalHybridMF::_get_entity_feature_cnt() {
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	set<int64_t>
			& itemIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> mergedIds;
	mergedIds.insert(mergedIds.end(), userIds.begin(), userIds.end());
	mergedIds.insert(mergedIds.end(), itemIds.begin(), itemIds.end());
	for (vector<int64_t>::const_iterator iter = mergedIds.begin(); iter
			< mergedIds.end(); ++iter) {
		vector<Interact> const
				& featureInteracts =
						m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[*iter][EntityInteraction::ADD_FEATURE];
		m_feat_cnt_map[*iter] = featureInteracts.size();
	}
}

void HierarchicalHybridMF::_update_feature(int64_t const& featId, map<int8_t,
		vector<Interact> > & typeInteracts) {
	m_entity[featId].reset();
	_update_feature_from_prior(featId);
	_update_feature_from_entities(featId, typeInteracts);
}

void HierarchicalHybridMF::_update_user_prior_mean() {
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	/// update user prior mean and covariance matrix
	DistParamBundle userPriorUpdateMessage(2);
	vec covSuff2 = m_user_prior_cov.suff_mean(2);
	userPriorUpdateMessage[1] = vec(-0.5 * numUsers * covSuff2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		size_t numFeats = m_feat_cnt_map[userId];
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatMean = userLat.moment(1);
		if (m_model_param.m_use_feature && numFeats > 0) {
			/// include contribution from content feature
			userLatMean -= (1 / sqrt(numFeats) * _entity_feature_mean_sum(
					userId));
		}
		/// update userPriorUpdateMessage
		userPriorUpdateMessage[0] += (userLatMean);
	}
	userPriorUpdateMessage[0].m_vec %= covSuff2;
	m_user_prior_mean = userPriorUpdateMessage;
}

void HierarchicalHybridMF::_update_user_prior_cov() {
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	/// aggregate over the users
	DistParamBundle userCovUpdateMessage(2);
	userCovUpdateMessage[0].m_vec = vec(m_model_param.m_lat_dim, fill::ones)
			* (-0.5 * numUsers);
	vec upm1 = m_user_prior_mean.moment(1);
	vec upm2 = m_user_prior_mean.moment(2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatMean = userLat.moment(1);
		vec userLatCov = userLat.moment(2);
		vec cov = userLatCov + upm2 - vectorise(upm1 * userLatMean.t()
				+ userLatMean * upm1.t());
		size_t userNumFeats = m_feat_cnt_map[userId];
		/// include content feature
		if (m_model_param.m_use_feature && userNumFeats > 0) {
			vec featMeanSum = _entity_feature_mean_sum(userId);
			vec featCovSum = _entity_feature_cov_sum(userId);
			/// offset the user mean
			userLatMean -= upm1;
			cov += (1 / (float) userNumFeats * featCovSum - 1 / sqrt(
					userNumFeats) * vectorise(userLatMean * featMeanSum.t()
					+ featMeanSum * userLatMean.t()));
		}
		mat covMat(cov);
		covMat.reshape(m_model_param.m_lat_dim, m_model_param.m_lat_dim);
		userCovUpdateMessage[1] += (covMat.diag());
	}
	userCovUpdateMessage[1].m_vec *= (-0.5);
	m_user_prior_cov = userCovUpdateMessage;
}

void HierarchicalHybridMF::_update_item_prior_mean() {
	set<int64_t>
			& itemIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	DistParamBundle itemPriorUpdateMessage(2);
	vec covSuff2 = m_item_prior_cov.suff_mean(2);
	itemPriorUpdateMessage[1] = vec(-0.5 * numItems * covSuff2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end(); ++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatMean = itemLat.moment(1);
		size_t numFeats = m_feat_cnt_map[itemId];
		if (m_model_param.m_use_feature && numFeats > 0) {
			itemLatMean -= ((1 / sqrt(numFeats) * _entity_feature_mean_sum(
					itemId)));
		}
		/// update userPriorUpdateMessage
		itemPriorUpdateMessage[0] += itemLatMean;
	}
	itemPriorUpdateMessage[0].m_vec %= covSuff2;
	m_item_prior_mean = itemPriorUpdateMessage;
}

void HierarchicalHybridMF::_update_item_prior_cov() {
	set<int64_t>
			& itemIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	DistParamBundle itemCovUpdateMessage(2);
	itemCovUpdateMessage[0].m_vec = (vec(m_model_param.m_lat_dim, fill::ones)
			* (-0.5) * numItems);
	vec ipm1 = m_item_prior_mean.moment(1);
	vec ipm2 = m_item_prior_mean.moment(2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end(); ++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatMean = itemLat.moment(1);
		vec itemLatCov = itemLat.moment(2);
		vec cov = itemLatCov + ipm2 - vectorise(ipm1 * itemLatMean.t()
				+ itemLatMean * ipm1.t());
		size_t itemNumFeats = m_feat_cnt_map[itemId];
		/// include content feature
		if (m_model_param.m_use_feature && itemNumFeats > 0) {
			vec featMeanSum = _entity_feature_mean_sum(itemId);
			vec featCovSum = _entity_feature_cov_sum(itemId);
			/// offset item latent mean
			itemLatMean -= ipm1;
			cov += (1 / (float) itemNumFeats * featCovSum - 1 / sqrt(
					itemNumFeats) * vectorise(itemLatMean * featMeanSum.t()
					+ featMeanSum * itemLatMean.t()));
		}
		mat covMat(cov);
		covMat.reshape(m_model_param.m_lat_dim, m_model_param.m_lat_dim);
		itemCovUpdateMessage[1] += (covMat.diag());
	}
	itemCovUpdateMessage[1].m_vec *= (-0.5);
	m_item_prior_cov = itemCovUpdateMessage;
}

void HierarchicalHybridMF::_update_feature_prior_mean() {
	//	/// update the mean and covariance sequentially
	set<int64_t>
			& featIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_FEATURE];
	size_t numFeats = featIds.size();
	if (numFeats > 0) {
		DistParamBundle updateMessage(2);
		vec fpCovSuff2 = m_feature_prior_cov.suff_mean(2);
		updateMessage[1] = (vec) (-0.5 * numFeats * fpCovSuff2);
		for (set<int64_t>::iterator iter = featIds.begin(); iter
				!= featIds.end(); ++iter) {
			int64_t featId = *iter;
			DiagMVGaussian & featLat = m_entity[featId];
			vec featLatMean = featLat.moment(1).m_vec;
			updateMessage[0] += featLatMean;
		}
		updateMessage[0].m_vec %= fpCovSuff2;
		m_feature_prior_mean = updateMessage;
	}
}

void HierarchicalHybridMF::_update_feature_prior_cov() {
	//	/// update the mean and covariance sequentially
	set<int64_t>
			& featIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_FEATURE];
	size_t numFeats = featIds.size();
	if (numFeats > 0) {
		/// update the covariance
		DistParamBundle updateMessage(2);
		updateMessage[0] = (vec) (-0.5 * numFeats * vec(
				m_model_param.m_lat_dim, fill::ones));
		vec fpm1 = m_feature_prior_mean.moment(1);
		vec fpm2 = m_feature_prior_mean.moment(2);
		for (set<int64_t>::iterator iter = featIds.begin(); iter
				!= featIds.end(); ++iter) {
			int64_t featId = *iter;
			DiagMVGaussian& featLat = m_entity[featId];
			vec featLatMean = featLat.moment(1);
			vec featLatCov = featLat.moment(2);
			vec cov = featLatCov + fpm2 - vectorise(featLatMean * fpm1.t()
					+ fpm1 * featLatMean.t());
			mat covMat(cov);
			covMat.reshape(m_model_param.m_lat_dim, m_model_param.m_lat_dim);
			updateMessage[1] += covMat.diag();
		}
		updateMessage[1].m_vec *= (-0.5);
		m_feature_prior_cov = updateMessage;
	}
}

void HierarchicalHybridMF::_update_user_prior() {
	_update_user_prior_mean();
	_update_user_prior_cov();
}

void HierarchicalHybridMF::_update_item_prior() {
	_update_item_prior_mean();
	_update_item_prior_cov();
}

void HierarchicalHybridMF::_update_feature_prior() {
	_update_feature_prior_mean();
	_update_feature_prior_cov();
}

void HierarchicalHybridMF::_lat_ip_moments(DiagMVGaussian & lat1,
		DiagMVGaussian & lat2, float & firstMoment, float & secondMoment) {
	/// calculate the first and second moment of the result of inner product of two Multivariate Gaussian variables (diagonal covariance matrix)
	vec lat1M1 = lat1.moment(1);
	vec lat1M2 = lat1.moment(2);
	mat lat1M2Mat = lat1M2;
	lat1M2Mat.reshape(m_model_param.m_lat_dim, m_model_param.m_lat_dim);
	vec lat2M1 = lat2.moment(1);
	vec lat2M2 = lat2.moment(2);
	mat lat2M2Mat = lat2M2;
	lat2M2Mat.reshape(m_model_param.m_lat_dim, m_model_param.m_lat_dim);
	firstMoment = accu(lat1M1 % lat2M1);
	//	secondMoment = pow(accu(lat1M1 % lat2M1),2) + accu(lat1Cov % lat2Cov + lat1M1 % lat1M1 % lat2Cov + lat2M1 % lat2M1 % lat1Cov);
	secondMoment = accu(lat1M1.t() * lat2M1 * lat2M1.t() * lat1M1) + accu(
			lat2M2Mat.diag() % lat1M2Mat.diag());
}

void HierarchicalHybridMF::_rating_bias_moments(float rating,
		float & firstMoment, float& secondMoment) {
	float bias1stMoment = m_bias.moment(1);
	float bias2ndMoment = m_bias.moment(2);
	firstMoment = rating - bias1stMoment;
	secondMoment = rating * rating - 2 * bias1stMoment * rating + bias2ndMoment;
}

void HierarchicalHybridMF::_update_rating_var() {
	/// update the rating variance
	/// go through all ratings
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin(); iter
			!= userIds.end(); ++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact>
				& ratings =
						m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[userId][EntityInteraction::RATE_ITEM];

		for (vector<Interact>::const_iterator iter1 = ratings.begin(); iter1
				< ratings.end(); ++iter1) {
			numRatings++;
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian & itemLat = m_entity[itemId];
			double rating = iter1->ent_val;
			float rating1stMoment, rating2ndMoment;
			_rating_bias_moments(rating, rating1stMoment, rating2ndMoment);
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			updateMessage[1] += (rating2ndMoment - 2 * rating1stMoment
					* ip1stMoment + ip2ndMoment);
		}
	}
	updateMessage[0] = float(-0.5) * numRatings;
	updateMessage[1].m_vec *= float(-0.5);
	m_rating_var = updateMessage;
}

float HierarchicalHybridMF::_get_mean_rating() {
	float avgRating = 0;
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		vector<Interact> const
				& ratings =
						m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin(); iter1
				< ratings.end(); ++iter1) {
			double rating = iter1->ent_val;
			avgRating += rating;
			numRatings++;
		}
	}
	return avgRating / numRatings;
}

void HierarchicalHybridMF::_update_bias() {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	set<int64_t>
			& userIds =
					m_dataset_manager->dataset(rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> const
				& ratings =
						m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin(); iter1
				< ratings.end(); ++iter1) {
			int64_t itemId = iter1->ent_id;
			double rating = iter1->ent_val;
			DiagMVGaussian & itemLat = m_entity[itemId];
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			updateMessage[0].m_vec += (rating - ip1stMoment);
			numRatings++;
		}
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	m_bias = updateMessage;
}

float HierarchicalHybridMF::dataset_rmse(DatasetExt& dataset) {
	/// evaluate the RMSE over the training dataset
	float rmse = 0;
	size_t numRatings = 0;
	/// first update feature entities
	set<int64_t>& featIds = dataset.type_ent_ids[Entity::ENT_FEATURE];
	for (set<int64_t>::iterator iter = featIds.begin(); iter != featIds.end(); ++iter) {
		int64_t entityId = *iter;
		if (m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_ids.find(
				entityId)
				== m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_ids.end()) {
			m_entity[entityId].reset();
			_update_feature_from_prior(entityId);
		}
	}
	set<int64_t>& userIds = dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t>& itemIds = dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> userItemIds;
	userItemIds.insert(userItemIds.end(), userIds.begin(), userIds.end());
	userItemIds.insert(userItemIds.end(), itemIds.begin(), itemIds.end());
	for (vector<int64_t>::iterator iter = userItemIds.begin(); iter
			< userItemIds.end(); ++iter) {
		int64_t entityId = *iter;
		int8_t entityType = dataset.m_id_type_map[entityId];
		assert(entityType > 0);
		/// new entity, initialize its latent vector by prior information
		if (m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_ids.find(
				entityId)
				== m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_ids.end()) {
			m_entity[entityId].reset();
			_update_entity_from_prior(entityId, entityType);
		}
	}
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		DiagMVGaussian& userLat = m_entity[*iter];
		vector<Interact> & ratingInteracts =
				dataset.ent_type_interacts[*iter][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin(); iter1
				< ratingInteracts.end(); ++iter1) {
			float ratingVal = iter1->ent_val;
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian& itemLat = m_entity[itemId];
			float predRating = accu(itemLat.moment(1).m_vec
					% userLat.moment(1).m_vec) + (float) m_bias.moment(1);
			float diff = predRating - ratingVal;
			rmse += (diff * diff);
			numRatings++;
		}
	}
	rmse = sqrt(rmse / numRatings);
	return rmse;
}

float HierarchicalHybridMF::train_rmse() {
	return dataset_rmse(get_train_ds());
}

float HierarchicalHybridMF::test_rmse() {
	return dataset_rmse(get_test_ds());
}

float HierarchicalHybridMF::cs_rmse() {
	return dataset_rmse(get_cs_ds());
}

void HierarchicalHybridMF::save_model(){
	if(m_model_file.empty()){
		/// generate a file name
		stringstream ss;
		ss << (string)(*this) << "_" << (string)(m_model_param) << "_model.bin";
		string fileName = ss.str();
		/// get working directory
		boost::filesystem::path cwd(boost::filesystem::current_path());
		m_model_file = string(cwd.c_str()) + "/" + fileName;
	}
	//// create an archive
	cout << "start to write the model to file: " << m_model_file << endl;
	std::ofstream ofs(m_model_file.c_str());
	boost::archive::binary_oarchive oa(ofs);
	oa << *this;
	cout << "done!" << endl;
}

void HierarchicalHybridMF::load_model(){
	cout << "load model from file: " << m_model_file << endl;
	ifstream ifs(m_model_file.c_str());
	boost::archive::binary_iarchive ia(ifs);
	ia >> *this;
	cout << "done!" << endl;
}


void HierarchicalHybridMF::train() {
	/// train Bayesian model on the training dataset
	set<int64_t> const& userIds = m_dataset_manager->dataset(
			rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_USER];
	set<int64_t> const& itemIds = m_dataset_manager->dataset(
			rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_ITEM];
	set<int64_t> const& featureIds = m_dataset_manager->dataset(
			rt::DSType::DS_TRAIN).type_ent_ids[Entity::ENT_FEATURE];
	vector<map<int8_t, vector<Interact> > >& type_interacts =
			m_dataset_manager->dataset(rt::DSType::DS_TRAIN).ent_type_interacts;
	typedef set<int64_t>::iterator id_set_iter;
	////
	cout << "----------- Variational Bayesian Message Inference -----------"
			<< endl;
	for (size_t m_iter = 1; m_iter <= m_model_param.m_max_iter; m_iter++) {
		timer iterTimer;
		RunTimeLog rtl;
//		if (m_model_param.m_use_feature)
//			_update_entity_feature_moments();
		for (id_set_iter iter = userIds.begin(); iter != userIds.end(); ++iter) {
			int64_t entityId = *iter;
			map<int8_t, vector<Interact> > & tmpEntityInteracts =
					type_interacts[entityId];
			_update_entity(entityId, Entity::ENT_USER, tmpEntityInteracts);
		}
		for (id_set_iter iter = itemIds.begin(); iter != itemIds.end(); ++iter) {
			int64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > & tmpEntityInteracts =
					type_interacts[entityId];
			_update_entity(entityId, Entity::ENT_ITEM, tmpEntityInteracts);
		}
		if (m_model_param.m_use_feature) {
			for (id_set_iter iter = featureIds.begin(); iter
					!= featureIds.end(); ++iter) {
				int64_t entityId = *iter;
				/// get user rating and feature interactions
				map<int8_t, vector<Interact> > & tmpEntityInteracts =
						type_interacts[entityId];
				_update_feature(entityId, tmpEntityInteracts);
			}
		}
		_update_user_prior();
		_update_item_prior();
		if (m_model_param.m_use_feature) {
			_update_feature_prior();
		}
		_update_rating_var();
		_update_bias();
		///
		rtl.m_iter = m_iter;
		rtl.m_iter_time = iterTimer.elapsed();
		rtl.m_train_rmse = train_rmse();
		rtl.m_test_rmse = test_rmse();
		rtl.m_cs_rmse = cs_rmse();
		cout << rtl;
	}
	cout << "----------- Done! -----------\n\n";

}

HierarchicalHybridMF::HierarchicalHybridMF(ModelParams const& modelParam,
		shared_ptr<DatasetManager> datasetManager, string const& modelFile) :
	Model(modelParam, datasetManager, modelFile) {
	_init();
}

HierarchicalHybridMF::HierarchicalHybridMF(string const& modelFile)
:Model(ModelParams(), shared_ptr<DatasetManager>(), modelFile){

}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
