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
#include <boost/archive/text_oarchive.hpp>
#include <vb/prob/ArmadilloSerialization.h>

namespace bf = boost::filesystem;
namespace po = boost::program_options;

namespace recsys {

ostream& operator <<(ostream& oss,
		HierarchicalHybridMF::RunTimeLog const& rhs) {
	oss << "iteration:" << rhs.m_iter << ",time:" << rhs.m_iter_time
			<< ",train rmse:" << rhs.m_train_rmse << ",test rmse:"
			<< rhs.m_test_rmse << ",cs rmse:" << rhs.m_cs_rmse << endl;
	return oss;
}

void HierarchicalHybridMF::_init_training() {
	assert(!m_active_dataset.ent_type_interacts.empty());
	cout << m_model_param << "\n";
	if (!m_dataset_manager) {
		cerr << "dataset manager is not provided" << endl;
		exit(1);
	}
	/// initialize entity latent variables
	m_entity.clear();
	m_entity.reserve(
			m_dataset_manager->dataset(rt::DSType::DS_ALL).ent_type_interacts.size());
	for (size_t i = 0;
			i
					< m_dataset_manager->dataset(rt::DSType::DS_ALL).ent_type_interacts.size();
			i++) {
		m_entity.push_back(
				DiagMVGaussian(vec(m_model_param.m_lat_dim, arma::fill::randn),
						vec(m_model_param.m_lat_dim, arma::fill::ones), false,
						true));
	}
	/// initialize prior variables
	m_user_prior_mean = DiagMVGaussian(
			vec(m_model_param.m_lat_dim, fill::zeros),
			(vec(m_model_param.m_lat_dim, fill::ones)), false, true);
	m_user_prior_cov = MVInverseGamma(
			vec(m_model_param.m_lat_dim, fill::ones) * 3,
			vec(m_model_param.m_lat_dim, fill::ones) * 3);
	m_item_prior_mean = m_user_prior_mean;
	m_item_prior_cov = m_user_prior_cov;
	m_feature_prior_mean = m_user_prior_mean;
	m_feature_prior_cov = m_user_prior_cov;

	/// rating variance, big variance
	m_rating_var = InverseGamma(3, 3);
	/// initialize bias as standard Gaussian
	m_bias = Gaussian(_get_mean_rating(), 1);
	/// get the number of features for each entity
	_get_entity_feature_cnt();
}

RecModel::TrainIterLog HierarchicalHybridMF::_train_update() {
	/// set the active dataset
	assert(!m_active_dataset.ent_type_interacts.empty());
	/// get the variables
	set<int64_t> const& userIds =
			m_active_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t> const& itemIds =
			m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	set<int64_t> const& featureIds =
			m_active_dataset.type_ent_ids[Entity::ENT_FEATURE];
	vector<map<int8_t, vector<Interact> > >& type_interacts =
			m_active_dataset.ent_type_interacts;
	typedef set<int64_t>::iterator id_set_iter;

	TrainIterLog iterLog;
	/// update user profile
	for (id_set_iter iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t entityId = *iter;
		map<int8_t, vector<Interact> > & tmpEntityInteracts =
				type_interacts[entityId];
		_update_entity(entityId, Entity::ENT_USER, tmpEntityInteracts);
	}

	/// update item profile
	for (id_set_iter iter = itemIds.begin(); iter != itemIds.end(); ++iter) {
		int64_t entityId = *iter;
		/// get user rating and feature interactions
		map<int8_t, vector<Interact> > & tmpEntityInteracts =
				type_interacts[entityId];
		_update_entity(entityId, Entity::ENT_ITEM, tmpEntityInteracts);
	}

	/// update feature profile
	if (m_model_param.m_use_feature) {
		for (id_set_iter iter = featureIds.begin(); iter != featureIds.end();
				++iter) {
			int64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > & tmpEntityInteracts =
					type_interacts[entityId];
			_update_feature(entityId, tmpEntityInteracts);
		}
	}
	/// update prior information
	_update_user_prior();
	_update_item_prior();
	if (m_model_param.m_use_feature) {
		_update_feature_prior();
	}
	//// update rating variance and global rating bias
	_update_rating_var();
	_update_bias();
	return iterLog;
}

void HierarchicalHybridMF::_update_entity_from_prior(int64_t const& entityId,
		int8_t entityType) {
	DistParamBundle message(2);
	DistParam upCovSuff2 = (
			entityType == Entity::ENT_USER ?
					m_user_prior_cov.suff_mean(2) :
					m_item_prior_cov.suff_mean(2));
	vec entityLatMean = (
			entityType == Entity::ENT_USER ?
					m_user_prior_mean.moment(1).m_vec :
					m_item_prior_mean.moment(1).m_vec);
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

void HierarchicalHybridMF::_update_entity_from_prior_helper(
		int64_t const& entityId, int8_t entityType,
		vector<Interact>& featureInteracts) {
	DistParamBundle message(2);
	DistParam upCovSuff2 = (
			entityType == Entity::ENT_USER ?
					m_user_prior_cov.suff_mean(2) :
					m_item_prior_cov.suff_mean(2));
	vec entityLatMean = (
			entityType == Entity::ENT_USER ?
					m_user_prior_mean.moment(1).m_vec :
					m_item_prior_mean.moment(1).m_vec);
	/// number of content features for current entity
	size_t entityFeatCnt = featureInteracts.size();
	/// add content feature prior information
	if (m_model_param.m_use_feature && entityFeatCnt > 0) {
		vec contentFeatPrior = 1 / sqrt(entityFeatCnt)
				* _entity_feature_mean_sum(featureInteracts);
		entityLatMean += contentFeatPrior;
	}
	message[0] = (vec) (upCovSuff2.m_vec % (entityLatMean));
	message[1] = (vec) (-0.5 * upCovSuff2.m_vec);
	m_entity[entityId] += message;

}

void HierarchicalHybridMF::_update_entity_from_ratings(int64_t const& entityId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	/// first reset the natural parameter of user vector
	/// update with rating feedback
	vector<Interact> & ratingInteracts =
			typeInteracts[EntityInteraction::RATE_ITEM];
	float rvsuff2 = (float) m_rating_var.suff_mean(2);
	/// update from ratings
	//// note: important to indicate the parameter as canonical form
	DistParamBundle message(2);
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
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
	updateMessage[0] = vec(
			featCovSuff2.m_vec % m_feature_prior_mean.moment(1).m_vec);
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
	for (vector<Interact>::const_iterator iter = interacts.begin();
			iter < interacts.end(); ++iter) {
		int64_t entityId = iter->ent_id;
		int8_t entityType = m_active_dataset.m_id_type_map[entityId];
		assert(entityType > 0);
		DistParam entityCovSuff2 = (
				entityType == Entity::ENT_USER ?
						m_user_prior_cov.suff_mean(2) :
						m_item_prior_cov.suff_mean(2));
		size_t numFeats = m_feat_cnt_map[entityId];
		DiagMVGaussian & entityLat = m_entity[entityId];
		///
		vec tmpDiff = entityLat.moment(1).m_vec
				- (entityType == Entity::ENT_USER ?
						m_user_prior_mean.moment(1).m_vec :
						m_item_prior_mean.moment(1).m_vec);
		vec otherFeatureMean = _entity_feature_mean_sum(entityId)
				- m_entity[featId].moment(1).m_vec;
		tmpDiff -= (1 / sqrt(numFeats) * otherFeatureMean);
		updateMessage[0] +=
				(1 / sqrt(numFeats) * tmpDiff % entityCovSuff2.m_vec);
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
	_update_entity_from_ratings(entityId, typeInteracts);
//	m_entity[entityId].moment(1).m_vec.print(cout);
	_update_entity_from_prior(entityId, entityType);
}

vec HierarchicalHybridMF::_entity_feature_mean_sum(int64_t const& entityId) {
	vector<Interact> const & featureInteracts =
			m_active_dataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
	return _entity_feature_mean_sum(featureInteracts);
}

vec HierarchicalHybridMF::_entity_feature_mean_sum(
		vector<Interact> const& featureInteracts) {
	vec meanSum(m_model_param.m_lat_dim, fill::zeros);
	for (vector<Interact>::const_iterator iter1 = featureInteracts.begin();
			iter1 < featureInteracts.end(); ++iter1) {
		int64_t featId = iter1->ent_id;
		meanSum += m_entity[featId].moment(1).m_vec;
	}
	return meanSum;
}

vec HierarchicalHybridMF::_entity_feature_cov_sum(int64_t const& entityId) {
	vector<Interact> const & featureInteracts =
			m_active_dataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
	vec covSum(m_model_param.m_lat_dim * m_model_param.m_lat_dim, fill::zeros);
	for (vector<Interact>::const_iterator iter1 = featureInteracts.begin();
			iter1 < featureInteracts.end(); ++iter1) {
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
	m_feat_cnt_map.clear();
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> mergedIds;
	mergedIds.insert(mergedIds.end(), userIds.begin(), userIds.end());
	mergedIds.insert(mergedIds.end(), itemIds.begin(), itemIds.end());
	for (vector<int64_t>::const_iterator iter = mergedIds.begin();
			iter < mergedIds.end(); ++iter) {
		vector<Interact> const & featureInteracts =
				m_active_dataset.ent_type_interacts[*iter][EntityInteraction::ADD_FEATURE];
		m_feat_cnt_map[*iter] = featureInteracts.size();
	}
}

void HierarchicalHybridMF::_update_feature(int64_t const& featId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	m_entity[featId].reset();
	_update_feature_from_prior(featId);
	_update_feature_from_entities(featId, typeInteracts);
}

void HierarchicalHybridMF::_update_user_prior_mean() {
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	/// update user prior mean and covariance matrix
	DistParamBundle userPriorUpdateMessage(2);
	vec covSuff2 = m_user_prior_cov.suff_mean(2);
	userPriorUpdateMessage[1] = vec(-0.5 * numUsers * covSuff2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		size_t numFeats = m_feat_cnt_map[userId];
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatMean = userLat.moment(1);
		if (m_model_param.m_use_feature && numFeats > 0) {
			/// include contribution from content feature
			userLatMean -= (1 / sqrt(numFeats)
					* _entity_feature_mean_sum(userId));
		}
		/// update userPriorUpdateMessage
		userPriorUpdateMessage[0] += (userLatMean);
	}
	userPriorUpdateMessage[0].m_vec %= covSuff2;
	m_user_prior_mean = userPriorUpdateMessage;
}

void HierarchicalHybridMF::_update_user_prior_cov() {
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	/// aggregate over the users
	DistParamBundle userCovUpdateMessage(2);
	userCovUpdateMessage[0].m_vec = vec(m_model_param.m_lat_dim, fill::ones)
			* (-0.5 * numUsers);
	vec upm1 = m_user_prior_mean.moment(1);
	vec upm2 = m_user_prior_mean.moment(2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatMean = userLat.moment(1);
		vec userLatCov = userLat.moment(2);
		vec cov = userLatCov + upm2
				- vectorise(upm1 * userLatMean.t() + userLatMean * upm1.t());
		size_t userNumFeats = m_feat_cnt_map[userId];
		/// include content feature
		if (m_model_param.m_use_feature && userNumFeats > 0) {
			vec featMeanSum = _entity_feature_mean_sum(userId);
			vec featCovSum = _entity_feature_cov_sum(userId);
			/// offset the user mean
			userLatMean -= upm1;
			cov += (1 / (float) userNumFeats * featCovSum
					- 1 / sqrt(userNumFeats)
							* vectorise(
									userLatMean * featMeanSum.t()
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
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	DistParamBundle itemPriorUpdateMessage(2);
	vec covSuff2 = m_item_prior_cov.suff_mean(2);
	itemPriorUpdateMessage[1] = vec(-0.5 * numItems * covSuff2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatMean = itemLat.moment(1);
		size_t numFeats = m_feat_cnt_map[itemId];
		if (m_model_param.m_use_feature && numFeats > 0) {
			itemLatMean -= ((1 / sqrt(numFeats)
					* _entity_feature_mean_sum(itemId)));
		}
		/// update userPriorUpdateMessage
		itemPriorUpdateMessage[0] += itemLatMean;
	}
	itemPriorUpdateMessage[0].m_vec %= covSuff2;
	m_item_prior_mean = itemPriorUpdateMessage;
}

void HierarchicalHybridMF::_update_item_prior_cov() {
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	DistParamBundle itemCovUpdateMessage(2);
	itemCovUpdateMessage[0].m_vec = (vec(m_model_param.m_lat_dim, fill::ones)
			* (-0.5) * numItems);
	vec ipm1 = m_item_prior_mean.moment(1);
	vec ipm2 = m_item_prior_mean.moment(2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatMean = itemLat.moment(1);
		vec itemLatCov = itemLat.moment(2);
		vec cov = itemLatCov + ipm2
				- vectorise(ipm1 * itemLatMean.t() + itemLatMean * ipm1.t());
		size_t itemNumFeats = m_feat_cnt_map[itemId];
		/// include content feature
		if (m_model_param.m_use_feature && itemNumFeats > 0) {
			vec featMeanSum = _entity_feature_mean_sum(itemId);
			vec featCovSum = _entity_feature_cov_sum(itemId);
			/// offset item latent mean
			itemLatMean -= ipm1;
			cov += (1 / (float) itemNumFeats * featCovSum
					- 1 / sqrt(itemNumFeats)
							* vectorise(
									itemLatMean * featMeanSum.t()
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
	set<int64_t> & featIds = m_active_dataset.type_ent_ids[Entity::ENT_FEATURE];
	size_t numFeats = featIds.size();
	if (numFeats > 0) {
		DistParamBundle updateMessage(2);
		vec fpCovSuff2 = m_feature_prior_cov.suff_mean(2);
		updateMessage[1] = (vec) (-0.5 * numFeats * fpCovSuff2);
		for (set<int64_t>::iterator iter = featIds.begin();
				iter != featIds.end(); ++iter) {
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
	set<int64_t> & featIds = m_active_dataset.type_ent_ids[Entity::ENT_FEATURE];
	size_t numFeats = featIds.size();
	if (numFeats > 0) {
		/// update the covariance
		DistParamBundle updateMessage(2);
		updateMessage[0] = (vec) (-0.5 * numFeats
				* vec(m_model_param.m_lat_dim, fill::ones));
		vec fpm1 = m_feature_prior_mean.moment(1);
		vec fpm2 = m_feature_prior_mean.moment(2);
		for (set<int64_t>::iterator iter = featIds.begin();
				iter != featIds.end(); ++iter) {
			int64_t featId = *iter;
			DiagMVGaussian& featLat = m_entity[featId];
			vec featLatMean = featLat.moment(1);
			vec featLatCov = featLat.moment(2);
			vec cov = featLatCov + fpm2
					- vectorise(
							featLatMean * fpm1.t() + fpm1 * featLatMean.t());
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

float HierarchicalHybridMF::_pred_error(int64_t const& userId, DatasetExt& dataset){
	float rmse = 0;
	vector<Interact>& userFeatureInteracts = dataset.ent_type_interacts[userId][EntityInteraction::ADD_FEATURE];
	for (vector<Interact>::iterator iter = userFeatureInteracts.begin();
			iter < userFeatureInteracts.end(); ++iter) {
		int64_t entityId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(entityId)
				== m_active_dataset.ent_ids.end()) {
			m_entity[entityId].reset();
			_update_feature_from_prior(entityId);
		}
	}
	if (m_active_dataset.ent_ids.find(userId)
			== m_active_dataset.ent_ids.end()) {
		m_entity[userId].reset();
		_update_entity_from_prior_helper(userId,Entity::ENT_USER,userFeatureInteracts);
		//// the following update from prior only applies to entities in the training dataset
//		_update_entity_from_prior(userId, Entity::ENT_USER);
	}
	vector<Interact>& ratingInteracts = dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		int64_t itemId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(itemId)
				== m_active_dataset.ent_ids.end()) {
			m_entity[itemId].reset();
			vector<Interact>& itemFeatureInteracts = dataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM];
			_update_entity_from_prior_helper(itemId,Entity::ENT_ITEM,itemFeatureInteracts);
//			_update_entity_from_prior(itemId, Entity::ENT_ITEM);
		}
	}
	DiagMVGaussian& userLat = m_entity[userId];
//	vec userMean = userLat.moment(1).m_vec;
//	userMean.print(cout);
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		float ratingVal = iter->ent_val;
		int64_t itemId = iter->ent_id;
		DiagMVGaussian& itemLat = m_entity[itemId];
//		vec itemMean = itemLat.moment(1).m_vec;
//		itemMean.print(cout);
		float predRating = accu(
				itemLat.moment(1).m_vec % userLat.moment(1).m_vec)
				+ (float) m_bias.moment(1);
		float diff = predRating - ratingVal;
		rmse += (diff * diff);
	}
	return rmse;
}

float HierarchicalHybridMF::_pred_error(int64_t const& userId,
		map<int8_t, vector<Interact> >& entityInteractMap) {
	/// evaluate the RMSE over the training dataset
	float rmse = 0;
	vector<Interact>& featureInteracts =
			entityInteractMap[EntityInteraction::ADD_FEATURE];
	for (vector<Interact>::iterator iter = featureInteracts.begin();
			iter < featureInteracts.end(); ++iter) {
		int64_t entityId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(entityId)
				== m_active_dataset.ent_ids.end()) {
			m_entity[entityId].reset();
			_update_feature_from_prior(entityId);
		}
	}
	if (m_active_dataset.ent_ids.find(userId)
			== m_active_dataset.ent_ids.end()) {
		m_entity[userId].reset();
		_update_entity_from_prior_helper(userId,Entity::ENT_USER,featureInteracts);
		//// the following update from prior only applies to entities in the training dataset
//		_update_entity_from_prior(userId, Entity::ENT_USER);
	}
	vector<Interact>& ratingInteracts =
			entityInteractMap[EntityInteraction::RATE_ITEM];
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		int64_t itemId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(itemId)
				== m_active_dataset.ent_ids.end()) {
			m_entity[itemId].reset();
			_update_entity_from_prior_helper(userId,Entity::ENT_USER,featureInteracts);
//			_update_entity_from_prior(itemId, Entity::ENT_ITEM);
		}
	}
	DiagMVGaussian& userLat = m_entity[userId];
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		float ratingVal = iter->ent_val;
		int64_t itemId = iter->ent_id;
		DiagMVGaussian& itemLat = m_entity[itemId];
		float predRating = accu(
				itemLat.moment(1).m_vec % userLat.moment(1).m_vec)
				+ (float) m_bias.moment(1);
		float diff = predRating - ratingVal;
		rmse += (diff * diff);
	}
	return rmse;
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
	secondMoment = accu(lat1M1.t() * lat2M1 * lat2M1.t() * lat1M1)
			+ accu(lat2M2Mat.diag() % lat1M2Mat.diag());
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
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> &ratings =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];

		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			numRatings++;
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian & itemLat = m_entity[itemId];
			double rating = iter1->ent_val;
			float rating1stMoment, rating2ndMoment;
			_rating_bias_moments(rating, rating1stMoment, rating2ndMoment);
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			updateMessage[1] += (rating2ndMoment
					- 2 * rating1stMoment * ip1stMoment + ip2ndMoment);
		}
	}
	updateMessage[0] = float(-0.5) * numRatings;
	updateMessage[1].m_vec *= float(-0.5);
	m_rating_var = updateMessage;
}

float HierarchicalHybridMF::_get_mean_rating() {
	float avgRating = 0;
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		vector<Interact> const & ratings =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
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
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> const & ratings =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
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

void HierarchicalHybridMF::_add_new_entity(int64_t const& entityId,
		int8_t const& entityType) {
	m_active_dataset.m_id_type_map[entityId] = entityType;
	m_active_dataset.ent_ids.insert(entityId);
	m_active_dataset.type_ent_ids[entityType].insert(entityId);
	/// allocate model space for the given entity if it does not exist yet
	size_t nextEntityId = m_entity.size();
	for (int64_t i = nextEntityId; i <= entityId; i++) {
		m_entity.push_back(
				DiagMVGaussian(vec(m_model_param.m_lat_dim, arma::fill::randn),
						vec(m_model_param.m_lat_dim, arma::fill::ones), false,
						true));
	}
}

string HierarchicalHybridMF::model_summary() {
	stringstream ss;
	ss << "# of entities: " << m_entity.size() << "\n";
	ss << "# of users: "
			<< m_active_dataset.type_ent_ids[Entity::ENT_USER].size() << "\n";
	ss << "# of items: "
			<< m_active_dataset.type_ent_ids[Entity::ENT_ITEM].size() << "\n";
	ss << "# of features: "
			<< m_active_dataset.type_ent_ids[Entity::ENT_FEATURE].size()
			<< "\n";
	/// dump model information
	ss << "global rating bias mean:" << (float) m_bias.moment(1) << "\n";
	ss << "global rating variance: " << (float) m_rating_var.moment(1) << "\n";
	float rmse = _dataset_rmse(m_active_dataset);
	ss << "rmse on the training dataset:" << rmse << "\n";
	return ss.str();
}

void HierarchicalHybridMF::dump_prior_information(string const& fileName) {
	/// dump prior information to text file
	ofstream ofs;
	ofs.open(fileName.c_str(), std::ofstream::out);
	assert(ofs.good());
	cout << ">>> dump prior information to: " << fileName << endl;
	vec userPriorMean = m_user_prior_mean.moment(1).m_vec;
	dump_vec_tsv(ofs, userPriorMean);
	vec itemPriorMean = m_item_prior_mean.moment(1).m_vec;
	dump_vec_tsv(ofs, itemPriorMean);
	vec featurePriorMean = m_feature_prior_mean.moment(1);
	dump_vec_tsv(ofs, featurePriorMean);
	ofs.close();
}

void HierarchicalHybridMF::dump_entity_profile(string const& fileName,
		int64_t const& entityId) {
	/// dump prior information to text file
	ofstream ofs;
	ofs.open(fileName.c_str(), std::ofstream::out | std::ofstream::app);
	assert(ofs.good());
	cout << "dump the latent vector of entity -[" << entityId << "] to file: "
			<< fileName << endl;
	/// dump as tab separated fields
	vec entLatMean = m_entity[entityId].moment(1).m_vec;
	dump_vec_tsv(ofs, entLatMean);
	ofs.close();
	cout << ">>> done!" << endl;
}

void HierarchicalHybridMF::dump_entity_profile(string const& fileName, vector<int64_t> const& entityIds){
	/// dump prior information to text file
	ofstream ofs;
	ofs.open(fileName.c_str(), std::ofstream::out | std::ofstream::app);
	assert(ofs.good());
	cout << "dump the entity latent vectors to file: "
			<< fileName << endl;
	/// dump as tab separated fields
	for(size_t i = 0; i < entityIds.size(); i++){
		vec entLatMean = m_entity[entityIds[i]].moment(1).m_vec;
		dump_vec_tsv(ofs, entLatMean);
	}
	ofs.close();
	cout << ">>> done!" << endl;
}

void HierarchicalHybridMF::dump_vec_tsv(ostream& oss, arma::vec const& v) {
	for (size_t i = 0; i < v.size(); i++) {
		oss << (i == 0 ? "" : "\t") << v(i);
	}
	oss << endl;
}

void HierarchicalHybridMF::dump_model_profile(string const& fileName){
	/// dump all entity latent mean vectors as csv file
	ofstream ofs;
	ofs.open(fileName.c_str(), std::ofstream::out|std::ofstream::app);
	assert(ofs.good());
	cout << ">>> start to dump model entity profile to file:" << fileName << endl;
	for(size_t i = 0; i < m_entity.size(); i++){
		vec entityLatMean = m_entity[i].moment(1).m_vec;
		for(size_t j = 0; j < entityLatMean.size(); j++){
			ofs << (j == 0 ? "" : ",") << entityLatMean(j);
		}
		ofs << "\n";
	}
	ofs.close();
	cout << ">>> done!" << endl;
}

void HierarchicalHybridMF::dump_recommendation_json(string const& fileName, int64_t const& userId, vector<Interact>& featureInteracts, vector<int64_t>& recList){
	ofstream ofs;
	ofs.open(fileName.c_str(), std::ofstream::out|std::ofstream::app);
	assert(ofs.good());
	js::Object obj;
	obj["userId"] = js::String(lexical_cast<string>(userId));
	js::Array features;
	for(vector<Interact>::iterator iter = featureInteracts.begin(); iter < featureInteracts.end(); ++iter){
		int64_t featId = iter->ent_id;
		features.Insert(js::String(lexical_cast<string,int>(featId)));
	}
	obj["features"] = features;
	/// insert the recommendation list
	js::Array recArr;
	for(vector<int64_t>::iterator iter = recList.begin(); iter < recList.end(); ++iter){
		recArr.Insert(js::String(lexical_cast<string,int>(*iter)));
	}
	obj["recList"] = recArr;
	/// convert to string
	stringstream ss;
	js::Writer::Write(obj,ss);
	string jsonStr = ss.str();
	jsonStr.erase(
			std::remove_if(jsonStr.begin(), jsonStr.end(), ::isspace),
			jsonStr.end());
	ofs << jsonStr << "\n";
	ofs.close();
}

vector<rt::Recommendation> HierarchicalHybridMF::recommend(
		int64_t const& userId,
		map<int8_t, vector<rt::Interact> >& userInteracts) {
	/// note: not consider new items
	if (userId >= m_entity.size()) {
		cout << "add new user:" << userId << endl;
		_add_new_entity(userId, Entity::ENT_USER);
		vector<rt::Interact>& featureInteracts =
				userInteracts[EntityInteraction::ADD_FEATURE];
		for (vector<rt::Interact>::iterator iter = featureInteracts.begin();
				iter < featureInteracts.end(); ++iter) {
			int64_t entId = iter->ent_id;
			if (entId >= m_entity.size()) {
				_add_new_entity(entId, Entity::ENT_FEATURE);
				m_entity[entId].reset();
				_update_feature_from_prior(entId);
			}
		}
		/// initialize user profile
		m_entity[userId].reset();
		_update_entity_from_prior_helper(userId, Entity::ENT_USER,
				featureInteracts);
//		_update_entity_from_ratings(userId, userInteracts);
	}
	/// now make recommendation
	/// naive strategy: predict the rating for each item and sort the recommendation result
	DiagMVGaussian& userLat = m_entity[userId];
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<rt::Recommendation> recList;
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		DiagMVGaussian& itemLat = m_entity[*iter];
		/// calculate the inner product
		rt::Recommendation rec;
		rec.score = accu(itemLat.moment(1).m_vec % userLat.moment(1).m_vec)
				+ (float) m_bias.moment(1);
		rec.id = lexical_cast<string>(*iter);
		rec.type = 0;
		recList.push_back(rec);
	}
	/// sort by score
	std::sort(recList.begin(), recList.end(), RecommendationComparator());
	/// only keep the top 20 results
	vector<rt::Recommendation> topRecList;
	for (size_t i = 0; i < 50 && i < recList.size(); i++) {
		topRecList.push_back(recList[i]);
	}
	return topRecList;
}

HierarchicalHybridMF::HierarchicalHybridMF() {
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
