/*
 * HierarchicalHybridMF.cpp
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/timer.hpp>
#include <algorithm>
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>

namespace recsys {

void HierarchicalHybridMF::_init() {
	/// first prepare the datasets
	_prepare_datasets();
	/// allocate space for model variables
	_prepare_model_variables();
	/// we are all set, embark the fun journey!
}

void HierarchicalHybridMF::_prepare_model_variables() {
	cout << "############## initialize model variables ##############" << endl;
	timer t;
	/// initialize entity latent variables
	m_entity.reserve(m_dataset.ent_type_interacts.size());
	for (size_t i = 0; i < m_dataset.ent_type_interacts.size(); i++) {
		m_entity.push_back(
				DiagMVGaussian(vec(m_lat_dim, arma::fill::zeros),
						vec(m_lat_dim, arma::fill::ones)));
	}
	/// initialize prior variables
	m_user_prior_mean = DiagMVGaussian(vec(m_lat_dim, fill::zeros),
			vec(m_lat_dim, fill::ones) * 100);
	m_user_prior_cov = MVInverseGamma(vec(m_lat_dim, fill::ones) * 3,
			vec(m_lat_dim, fill::ones) * 3);
	m_item_prior_mean = m_user_prior_mean;
	m_item_prior_cov = m_user_prior_cov;
	m_feature_prior_mean = m_user_prior_mean;
	m_feature_prior_cov = m_user_prior_cov;
	/// rating variance, big variance
	m_rating_var = InverseGamma(3, 3);
	/// initialize bias as standard Gaussian
	float meanRating = _get_mean_rating();
	cout << "mean rating:" << meanRating << endl;
	m_bias = Gaussian(meanRating, 1);
	cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed() << " >>>>>>>>>>>>>>"
			<< endl;

}

void HierarchicalHybridMF::_update_user_or_item(int64_t const& entityId, int8_t entityType,
		map<int8_t, vector<Interact> > & typeInteracts) {
	/// first reset the natural parameter of user vector
	m_entity[entityId].reset();
	/// update with rating feedback
	vector<Interact> & ratingInteracts =
			typeInteracts[EntityInteraction::RATE_ITEM];
	/// sufficient  1/\sigma^2
	float rvsuff2 = (float) m_rating_var.suff_mean(2);
	/// update from ratings
	//// note: important to indicate the parameter as canonical form
	NatParamVec updateMessage(2 * m_lat_dim, true);
	updateMessage.m_vec.fill(0);
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		Interact & tmpInteract = *iter;
		float tmpRating = tmpInteract.ent_val;
		int64_t itemId = tmpInteract.ent_id;
		DiagMVGaussian & itemLat = m_entity[itemId];
		float tmpRating1 = tmpRating - m_bias.moment(1);
		vec itemLatMean = itemLat.moment(1);
		vec itemLatCov = itemLat.moment(2);
		vec update1 =  (tmpRating1 * rvsuff2
				* itemLatMean);
		vec update2 = (-0.5
				* rvsuff2 * itemLatCov);
		updateMessage.m_vec.rows(0, m_lat_dim - 1) += update1;
		/// second moment of item latent variable
		updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) += update2;
	}
	m_entity[entityId] += updateMessage;

	/// reset the message
	updateMessage.m_vec.fill(0);
	/// update from the prior
	vector<Interact> & featureInteracts =
			typeInteracts[EntityInteraction::ADD_FEATURE];
	NatParamVec upCovSuff2 = (
			entityType == Entity::ENT_USER ?
					m_user_prior_cov.suff_mean(2) :
					m_item_prior_cov.suff_mean(2));
	/// user prior mean
	NatParamVec upMean = (
			entityType == Entity::ENT_USER ?
					m_user_prior_mean.moment(1) : m_item_prior_mean.moment(1));
	size_t numFeats = featureInteracts.size();
	for (vector<Interact>::iterator iter = featureInteracts.begin();
			iter < featureInteracts.end(); ++iter) {
		int64_t featId = iter->ent_id;
		DiagMVGaussian & featLat = m_entity[featId];
		updateMessage.m_vec.rows(0, m_lat_dim - 1) += featLat.moment(1).m_vec;
	}
	if (numFeats > 0) {
		updateMessage.m_vec.rows(0, m_lat_dim - 1) *= 1 / sqrt(numFeats);
	}
	updateMessage.m_vec.rows(0, m_lat_dim - 1) += upMean.m_vec;
	updateMessage.m_vec.rows(0, m_lat_dim - 1) %= upCovSuff2.m_vec;
	updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) = (-0.5
			* upCovSuff2.m_vec);

	/// apply the update
	m_entity[entityId] += updateMessage;
	cout << m_entity[entityId] << endl;

}

void HierarchicalHybridMF::_update_entity_feature_moments() {
	/// update the user/item prior mean
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t> & itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> mergedIds;
	mergedIds.insert(mergedIds.end(), userIds.begin(), userIds.end());
	mergedIds.insert(mergedIds.end(), itemIds.begin(), itemIds.end());
	for (vector<int64_t>::const_iterator iter = mergedIds.begin();
			iter < mergedIds.end(); ++iter) {
		m_feat_mean_sum[*iter] = vec(m_lat_dim, fill::zeros);
		m_feat_cov_sum[*iter] = vec(m_lat_dim, fill::zeros);
		vector<Interact> const& featureInteracts =
				m_train_dataset.ent_type_interacts[*iter][EntityInteraction::ADD_FEATURE];
		m_feat_cnt_map[*iter] = featureInteracts.size();
		for (vector<Interact>::const_iterator iter1 = featureInteracts.begin();
				iter1 != featureInteracts.end(); ++iter1) {
			int64_t featId = iter1->ent_id;
			m_feat_mean_sum[*iter] += m_entity[featId].moment(1).m_vec;
			m_feat_cov_sum[*iter] += m_entity[featId].moment(2).m_vec;
		}
	}
}

void HierarchicalHybridMF::_update_feature(int64_t const& featId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	vector<Interact> const& interacts =
			typeInteracts[EntityInteraction::ADD_FEATURE];
	NatParamVec updateMessage(2 * m_lat_dim, true);
	NatParamVec featLatMean = m_entity[featId].moment(1);
	for (vector<Interact>::const_iterator iter = interacts.begin();
			iter < interacts.end(); ++iter) {
		int64_t entityId = iter->ent_id;
		int8_t entityType = m_train_dataset.m_id_type_map[entityId];
		NatParamVec upCovSuff2 = (
				entityType == Entity::ENT_USER ?
						m_user_prior_cov.suff_mean(2) :
						m_item_prior_cov.suff_mean(2));
		size_t numFeats = m_feat_cnt_map[entityId];
		DiagMVGaussian & entityLat = m_entity[entityId];
		///
		vec tmpDiff = entityLat.moment(1).m_vec
				- (entityType == Entity::ENT_USER ?
						m_user_prior_mean.moment(1) :
						m_item_prior_mean.moment(1));
		if (numFeats > 0) {
			tmpDiff -= 1 / sqrt(numFeats)
					* (m_feat_mean_sum[entityId] - featLatMean.m_vec);
		}
		if (numFeats > 0) {
			updateMessage.m_vec.rows(0, m_lat_dim - 1) += 1 / sqrt(numFeats)
					* tmpDiff % upCovSuff2.m_vec;
			updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) += (-0.5
					/ (float) numFeats * upCovSuff2.m_vec);
		}
	}
	/// apply the update
	m_entity[featId] = updateMessage;
}

void HierarchicalHybridMF::_update_user_prior() {
	/// update the mean of the prior
	m_user_prior_mean.reset();
	NatParamVec updateMessage(2 * m_lat_dim, true);
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	NatParamVec upCovSuff2 = m_user_prior_cov.suff_mean(2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		size_t numFeats = m_feat_cnt_map[userId];
		vec tmpUpdate = userLat.moment(1);
		if (numFeats > 0) {
			tmpUpdate -= (1 / sqrt(numFeats) * m_feat_mean_sum[userId]);
		}
		updateMessage.m_vec.rows(0, m_lat_dim - 1) += tmpUpdate;
	}
	updateMessage.m_vec.rows(0, m_lat_dim - 1) %= upCovSuff2.m_vec;
	updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) = (-0.5 * numUsers
			* upCovSuff2.m_vec);
	m_user_prior_mean = updateMessage;
	/// update the diagonal covariance matrix, each entry of which is InverseGamma distribution
	m_user_prior_cov.reset();
	/// aggregate over the users
	NatParamVec covNatParam(2 * m_lat_dim, true);
	vec tmpVec(m_lat_dim);
	tmpVec.fill(-0.5);
	covNatParam.m_vec.rows(0, m_lat_dim - 1) = tmpVec * userIds.size();
	vec userPriorMean = m_user_prior_mean.moment(1);
	vec userPriorCov = m_user_prior_mean.moment(2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		size_t numFeats = m_feat_cnt_map[userId];
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatMean = userLat.moment(1);
		vec userLatCov = userLat.moment(2);
		vec & userFeatMeanSum = m_feat_mean_sum[userId];
		vec & userFeatCovSum = m_feat_cov_sum[userId];
		vec userFeatMeanSum1 = userPriorMean;
		if (numFeats > 0) {
			userFeatMeanSum1 += 1 / sqrt(numFeats) * userFeatMeanSum;
		}
		/// considering the covariance
		vec userFeatCovSum1 = userPriorCov;
		if (numFeats > 0)
			userFeatCovSum1 += (1 / (float) numFeats * userFeatCovSum
					+ 2 / sqrt(numFeats) * userPriorMean % userFeatMeanSum);
		covNatParam.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) += (userLatCov
				+ userFeatCovSum1 - 2 * userLatMean % userFeatMeanSum1);
	}
	m_user_prior_cov = covNatParam;
}

void HierarchicalHybridMF::_update_item_prior() {
	/// update the mean of the prior
	/// update the mean of the prior
	m_item_prior_mean.reset();
	NatParamVec updateMessage(2 * m_lat_dim, true);
	set<int64_t> & itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	NatParamVec upCovSuff2 = m_item_prior_cov.suff_mean(2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		size_t numFeats = m_feat_cnt_map[itemId];
		vec tmpUpdate = itemLat.moment(1);
		if(numFeats){
			tmpUpdate -= (1 / sqrt(numFeats) * m_feat_mean_sum[itemId]);
		}
		updateMessage.m_vec.rows(0, m_lat_dim - 1) += tmpUpdate;
	}
	updateMessage.m_vec.rows(0, m_lat_dim - 1) %= upCovSuff2.m_vec;
	updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) = (-0.5 * numItems
			* upCovSuff2.m_vec);
	m_item_prior_mean = updateMessage;
	/// update the diagonal covariance matrix, each entry of which is InverseGamma distribution
	m_item_prior_cov.reset();
	/// aggregate over the users
	NatParamVec covNatParam(2 * m_lat_dim, true);
	vec tmpVec(m_lat_dim);
	tmpVec.fill(-0.5);
	covNatParam.m_vec.rows(0, m_lat_dim - 1) = tmpVec * itemIds.size();
	vec itemPriorMean = m_item_prior_mean.moment(1);
	vec itemPriorCov = m_item_prior_mean.moment(2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		size_t numFeats = m_feat_cnt_map[itemId];
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatMean = itemLat.moment(1);
		vec itemLatCov = itemLat.moment(2);
		vec & itemFeatMeanSum = m_feat_mean_sum[itemId];
		vec & itemFeatCovSum = m_feat_cov_sum[itemId];
		vec itemFeatMeanSum1 = itemPriorMean;
		if (numFeats > 0)
			itemFeatMeanSum1 += 1 / sqrt(numFeats) * itemFeatMeanSum;
		/// considering the covariance
		vec itemFeatCovSum1 = itemPriorCov;
		if (numFeats > 0) {
			itemFeatCovSum1 += (1 / (float) numFeats * itemFeatCovSum
					+ 2 * 1 / sqrt(numFeats) * itemPriorMean % itemFeatMeanSum);

		}
		covNatParam.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) += (itemLatCov
				+ itemFeatCovSum1 - 2 * itemLatMean % itemFeatMeanSum1);
	}
	m_item_prior_cov = covNatParam;
}

void HierarchicalHybridMF::_update_feature_prior() {
	/// update the mean and covariance sequentially
	set<int64_t> & featIds = m_train_dataset.type_ent_ids[Entity::ENT_FEATURE];
	size_t numFeats = featIds.size();
	if(numFeats > 0){
		NatParamVec updateMessage(2 * m_lat_dim, true);
		vec fpCovSuff2 = m_feature_prior_cov.suff_mean(2);
		updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) = (-0.5 * numFeats
				* fpCovSuff2);
		for (set<int64_t>::iterator iter = featIds.begin(); iter != featIds.end();
				++iter) {
			int64_t featId = *iter;
			DiagMVGaussian & featLat = m_entity[featId];
			vec featLatMean = featLat.moment(1);
			updateMessage.m_vec.rows(0, m_lat_dim - 1) +=
					(fpCovSuff2 % featLatMean);
		}
		m_feature_prior_mean = updateMessage;
		/// update the covariance
		/// reset the updateMessage
		updateMessage.m_vec.fill(0);
		vec tmpVec(m_lat_dim);
		tmpVec.fill(-0.5);
		updateMessage.m_vec.rows(0, m_lat_dim - 1) = tmpVec * numFeats;
		vec featPriorMean = m_feature_prior_mean.moment(1);
		vec featPriorCov = m_feature_prior_mean.moment(2);
		for (set<int64_t>::iterator iter = featIds.begin(); iter != featIds.end();
				++iter) {
			int64_t featId = *iter;
			DiagMVGaussian& featLat = m_entity[featId];
			vec featLatMean = featLat.moment(1);
			vec featLatCov = featLat.moment(2);
			updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) +=
					(-0.5
							* (featLatCov + featPriorCov
									- 2 * featLatMean % featPriorMean));
		}
		m_feature_prior_cov = updateMessage;
	}
}

void HierarchicalHybridMF::_lat_ip_moments(DiagMVGaussian & lat1,
		DiagMVGaussian & lat2, float & firstMoment, float & secondMoment) {
	/// calculate the first and second moment of the result of inner product of two Multivariate Gaussian variables (diagonal covariance matrix)
	vec lat1M1 = lat1.moment(1);
	vec lat1Cov = lat1.moment(2) - lat1M1 % lat1M1;
	vec lat2M1 = lat2.moment(1);
	vec lat2Cov = lat2.moment(2) - lat2M1 % lat2M1;

	firstMoment = accu(lat1M1 % lat2M1);
	secondMoment = accu(
			arma::pow(lat1M1 % lat2M1, 2) + lat1Cov % lat2Cov
					+ lat1M1 % lat2Cov % lat1M1 + lat2M1 % lat1Cov % lat2M1);
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
	NatParamVec updateMessage((size_t) 2, true);
	updateMessage.m_vec.fill(0);
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> & ratings =
				m_train_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];

		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian & itemLat = m_entity[itemId];
			double rating = iter1->ent_val;
			updateMessage.m_vec[0] += (-0.5);
			float rating1stMoment, rating2ndMoment;
			_rating_bias_moments(rating, rating1stMoment, rating2ndMoment);
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			updateMessage.m_vec[1] += (-0.5
					* (rating2ndMoment - 2 * rating1stMoment * ip1stMoment
							+ ip2ndMoment));
		}
	}
	m_rating_var = updateMessage;
	cout << "rating var:" << m_rating_var << endl;
}

float HierarchicalHybridMF::_get_mean_rating(){
	float avgRating = 0;
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		vector<Interact> const& ratings =
				m_train_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			int64_t itemId = iter1->ent_id;
			double rating = iter1->ent_val;
			avgRating += rating;
			numRatings++;
		}
	}
	return avgRating/numRatings;
}

void HierarchicalHybridMF::_update_bias() {
	float rvSuff2 = (float)m_rating_var.suff_mean(2);
	NatParamVec updateMessage((size_t) 2, true);
	updateMessage.m_vec.fill(0);
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> const& ratings =
				m_train_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian & itemLat = m_entity[itemId];
			double rating = iter1->ent_val;
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			updateMessage.m_vec[0] += (rating - ip1stMoment);
			numRatings++;
		}
	}
	updateMessage.m_vec[0] *= (rvSuff2);
	updateMessage.m_vec[1] = -0.5 * numRatings * rvSuff2;
	m_bias = updateMessage;
	cout << "bias :" << m_bias << endl;
}

float HierarchicalHybridMF::train_rmse() {
	/// evaluate the RMSE over the training dataset
	float rmse = 0;
	size_t numRatings = 0;
	set<int64_t>& userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		DiagMVGaussian& userLat = m_entity[*iter];
		vector<Interact>& ratingInteracts =
				m_train_dataset.ent_type_interacts[*iter][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
			float ratingVal = iter1->ent_val;
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian& itemLat = m_entity[itemId];
			float predRating = accu(
					itemLat.moment(1).m_vec % userLat.moment(1).m_vec)
					+ (float) m_bias.moment(1);
			float diff = predRating - ratingVal;
			rmse += (diff * diff);
			numRatings++;
		}
	}
	rmse = sqrt(rmse / numRatings);
	return rmse;
}

void HierarchicalHybridMF::train_model() {
	/// train Bayesian model on the training dataset
	const size_t maxIter = 20;
	set<int64_t> const& userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t> const& itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	set<int64_t> const& featureIds =
			m_train_dataset.type_ent_ids[Entity::ENT_FEATURE];
	vector<map<int8_t, vector<Interact> > >& type_interacts =
			m_train_dataset.ent_type_interacts;
	typedef set<int64_t>::iterator id_set_iter;
	////
	timer iterTimer;
	for ( m_iter = 1; m_iter <= maxIter; m_iter++) {
		iterTimer.restart();
		_update_entity_feature_moments();
		/// update user entities
		cout << ">>>>> update user latent vectors >>>>>" << endl;
		timer t;
		t.restart();
		for (id_set_iter iter = userIds.begin(); iter != userIds.end();
				++iter) {
			int64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > & tmpEntityInteracts =
					type_interacts[entityId];
			_update_user_or_item(entityId, Entity::ENT_USER, tmpEntityInteracts);
		}
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		cout << ">>>>> update item latent vectors >>>>>" << endl;
		t.restart();
		for (id_set_iter iter = itemIds.begin(); iter != itemIds.end();
				++iter) {
			int64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > & tmpEntityInteracts =
					type_interacts[entityId];
			_update_user_or_item(entityId, Entity::ENT_ITEM, tmpEntityInteracts);
		}
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		cout << ">>>>> update feature latent vectors >>>>>" << endl;
		t.restart();
		for (id_set_iter iter = featureIds.begin(); iter != featureIds.end();
				++iter) {
			int64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > & tmpEntityInteracts =
					type_interacts[entityId];
			_update_feature(entityId, tmpEntityInteracts);
		}
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		/// update prior
		t.restart();
		cout << ">>>>> update user prior >>>>>" << endl;
		_update_user_prior();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		t.restart();
		cout << ">>>>> update item prior >>>>>" << endl;
		_update_item_prior();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		t.restart();
		cout << ">>>>> update feature prior >>>>>" << endl;
		_update_feature_prior();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		t.restart();
		cout << ">>>>> update global bias >>>>>" << endl;
		_update_bias();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		t.restart();
		cout << ">>>>> update rating variance >>>>>" << endl;
		_update_rating_var();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		///
		float rmse = train_rmse();
		cout << ">>>>>>>>>>>>>>>>> rmse:" << rmse << endl;
		cout << ">>>>>>>>>>>>>>>>> time for single iteration:"
				<< iterTimer.elapsed() << endl;
	}
}

void HierarchicalHybridMF::_prepare_datasets() {
	try {
		m_transport->open();
		cout
				<< "############## retrieve datasets from data host  ##############"
				<< endl;
		timer t;
		m_client.get_dataset(m_dataset, rt::DSType::DS_ALL);
		m_client.get_dataset(m_train_dataset, rt::DSType::DS_TRAIN);
		m_client.get_dataset(m_test_dataset, rt::DSType::DS_TEST);
		m_client.get_dataset(m_cs_dataset, rt::DSType::DS_CS);
		m_dataset.prepare_id_type_map();
		m_train_dataset.prepare_id_type_map();
		m_test_dataset.prepare_id_type_map();
		m_cs_dataset.prepare_id_type_map();
		cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed()
				<< " >>>>>>>>>>>>>>" << endl;
		m_transport->close();
	} catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
	}
}

HierarchicalHybridMF::HierarchicalHybridMF() :
		m_num_users(0), m_num_items(0), m_num_features(0), m_socket(
				new TSocket("localhost", 9090)), m_transport(
				new TBufferedTransport(m_socket)), m_protocol(
				new TBinaryProtocol(m_transport)), m_client(m_protocol), m_lat_dim(
				10),m_iter(0) {
	_init();
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
