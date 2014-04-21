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
		m_entity.push_back(DiagMVGaussian(vec(m_lat_dim, arma::fill::randn),vec(m_lat_dim, arma::fill::ones),false,true));
	}
	/// initialize prior variables
	m_user_prior_mean = DiagMVGaussian(vec(m_lat_dim, fill::zeros),
			(vec(m_lat_dim, fill::ones)),false,true);
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
		vec update1 =  (tmpRating1 * rvsuff2
				* itemLatMean);
		vec update2 = (-0.5
				* rvsuff2 * itemLatCov);
		message[0] += update1;
		message[1] += update2;
	}
	m_entity[entityId] = message;
	/// reset the message
	prob::DistParam upCovSuff2 = (
			entityType == Entity::ENT_USER ?
					m_user_prior_cov.suff_mean(2) :
					m_item_prior_cov.suff_mean(2));
	vec entityLatMean = (entityType == Entity::ENT_USER ? m_user_prior_mean.moment(1).m_vec : m_item_prior_mean.moment(1).m_vec);
	message[0] = (vec)(upCovSuff2.m_vec % entityLatMean);
	message[1] = (vec)(-0.5 * upCovSuff2.m_vec);
	m_entity[entityId] += message;
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

void HierarchicalHybridMF::_dump_interact_array(vector<Interact> const& vec){
	for(size_t i = 0; i < vec.size(); i++){
		Interact const& tmpInteract = vec[i];
		cout << tmpInteract.ent_id << ",";
	}
	cout << endl;
}

void HierarchicalHybridMF::_update_feature(int64_t const& featId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	vector<Interact> const& interacts =
			typeInteracts[EntityInteraction::ADD_FEATURE];
	DistParamBundle updateMessage(2);
	DistParam featLatMean = m_entity[featId].moment(1);
	for (vector<Interact>::const_iterator iter = interacts.begin();
			iter < interacts.end(); ++iter) {
		int64_t entityId = iter->ent_id;
		int8_t entityType = m_train_dataset.m_id_type_map[entityId];
		DistParam upCovSuff2 = (
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
			updateMessage[0].m_vec += 1 / sqrt(numFeats)
					* tmpDiff % upCovSuff2.m_vec;
			updateMessage[1].m_vec += (-0.5
					/ (float) numFeats * upCovSuff2.m_vec);
		}
	}
	/// apply the update
	m_entity[featId] = updateMessage;
}

void HierarchicalHybridMF::_update_user_prior_mean(){
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	/// update user prior mean and covariance matrix
	DistParamBundle userPriorUpdateMessage(2);
	vec covSuff2 = m_user_prior_cov.suff_mean(2);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatMean = userLat.moment(1);
		vec userLatCov = userLat.moment(2);
		mat userLatCovMat = userLatCov;
		userLatCovMat.reshape(m_lat_dim,m_lat_dim);
		vec covDiag = userLatCovMat.diag();
		/// update userPriorUpdateMessage
		userPriorUpdateMessage[0] += covSuff2 % userLatMean;
//		userPriorUpdateMessage[1] += (covSuff2);
	}
	userPriorUpdateMessage[1].m_vec = vec(-0.5 * numUsers * covSuff2);
	m_user_prior_mean = userPriorUpdateMessage;
}

void HierarchicalHybridMF::_update_user_prior_cov(){
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	/// aggregate over the users
	DistParamBundle userCovUpdateMessage(2);
	userCovUpdateMessage[0].m_vec = vec(m_lat_dim,fill::ones) * (-0.5 * numUsers);
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vec userLatCov = userLat.moment(2);
		mat userLatCovMat = userLatCov;
		userLatCovMat.reshape(m_lat_dim,m_lat_dim);
		vec covDiag = userLatCovMat.diag();
		userCovUpdateMessage[1]  += (covDiag);
	}
	userCovUpdateMessage[1].m_vec *= (-0.5);
	m_user_prior_cov = userCovUpdateMessage;
}

void HierarchicalHybridMF::_update_item_prior_mean(){
	set<int64_t> & itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	DistParamBundle itemPriorUpdateMessage(2);
	vec covSuff2 = m_item_prior_cov.suff_mean(2);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatMean = itemLat.moment(1);
		/// update userPriorUpdateMessage
		itemPriorUpdateMessage[0] += covSuff2 % itemLatMean;
//		itemPriorUpdateMessage[1] += (covSuff2);
	}
	itemPriorUpdateMessage[1] = vec(-0.5 * numItems * covSuff2);
	m_item_prior_mean  = itemPriorUpdateMessage;
}

void HierarchicalHybridMF::_update_item_prior_cov(){
	set<int64_t> & itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	DistParamBundle covNatParam(2);
	covNatParam[0].m_vec = (vec(m_lat_dim,fill::ones) * (-0.5) * numItems);
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		DiagMVGaussian & itemLat = m_entity[itemId];
		vec itemLatCov = itemLat.moment(2);
		mat itemLatCovMat(itemLatCov);
		itemLatCovMat.reshape(m_lat_dim,m_lat_dim);
		/// update item prior mean
		vec covDiag = itemLatCovMat.diag();
		covNatParam[1] += (covDiag);
	}
	covNatParam[1].m_vec *= (-0.5);
	m_item_prior_cov = covNatParam;
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
//	/// update the mean and covariance sequentially
//	set<int64_t> & featIds = m_train_dataset.type_ent_ids[Entity::ENT_FEATURE];
//	size_t numFeats = featIds.size();
//	if(numFeats > 0){
//		DistParam updateMessage(2 * m_lat_dim, true);
//		vec fpCovSuff2 = m_feature_prior_cov.suff_mean(2);
//		updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) = (-0.5 * numFeats
//				* fpCovSuff2);
//		for (set<int64_t>::iterator iter = featIds.begin(); iter != featIds.end();
//				++iter) {
//			int64_t featId = *iter;
//			DiagMVGaussian & featLat = m_entity[featId];
//			vec featLatMean = featLat.moment(1);
//			updateMessage.m_vec.rows(0, m_lat_dim - 1) +=
//					(fpCovSuff2 % featLatMean);
//		}
//		m_feature_prior_mean = updateMessage;
//		/// update the covariance
//		/// reset the updateMessage
//		updateMessage.m_vec.fill(0);
//		vec tmpVec(m_lat_dim);
//		tmpVec.fill(-0.5);
//		updateMessage.m_vec.rows(0, m_lat_dim - 1) = tmpVec * numFeats;
//		vec featPriorMean = m_feature_prior_mean.moment(1);
//		vec featPriorCov = m_feature_prior_mean.moment(2);
//		for (set<int64_t>::iterator iter = featIds.begin(); iter != featIds.end();
//				++iter) {
//			int64_t featId = *iter;
//			DiagMVGaussian& featLat = m_entity[featId];
//			vec featLatMean = featLat.moment(1);
//			vec featLatCov = featLat.moment(2);
//			updateMessage.m_vec.rows(m_lat_dim, 2 * m_lat_dim - 1) +=
//					(-0.5
//							* (featLatCov + featPriorCov
//									- 2 * featLatMean % featPriorMean));
//		}
//		m_feature_prior_cov = updateMessage;
//	}
}

void HierarchicalHybridMF::_lat_ip_moments(DiagMVGaussian & lat1,
		DiagMVGaussian & lat2, float & firstMoment, float & secondMoment) {
	/// calculate the first and second moment of the result of inner product of two Multivariate Gaussian variables (diagonal covariance matrix)
	vec lat1M1 = lat1.moment(1);
	vec lat1M2 = lat1.moment(2);
	mat lat1M2Mat = lat1M2;
	lat1M2Mat.reshape(m_lat_dim,m_lat_dim);
	vec lat2M1 = lat2.moment(1);
	vec lat2M2 = lat2.moment(2);
	mat lat2M2Mat = lat2M2;
	lat2M2Mat.reshape(m_lat_dim,m_lat_dim);
	firstMoment = accu(lat1M1 % lat2M1);
//	secondMoment = pow(accu(lat1M1 % lat2M1),2) + accu(lat1Cov % lat2Cov + lat1M1 % lat1M1 % lat2Cov + lat2M1 % lat2M1 % lat1Cov);
	secondMoment = accu(lat1M1.t() * lat2M1 * lat2M1.t() * lat1M1) +  accu(lat2M2Mat.diag() % lat1M2Mat.diag());
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
	updateMessage[0] = updateMessage[1] = (float)0;
	set<int64_t> & userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> & ratings =
				m_train_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];

		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			numRatings ++;
			int64_t itemId = iter1->ent_id;
			DiagMVGaussian & itemLat = m_entity[itemId];
			double rating = iter1->ent_val;
			float rating1stMoment, rating2ndMoment;
			_rating_bias_moments(rating, rating1stMoment, rating2ndMoment);
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			updateMessage[1] += (rating2ndMoment - 2 * rating1stMoment * ip1stMoment
							+ ip2ndMoment);
		}
	}
	updateMessage[0] = float(-0.5) * numRatings;
	updateMessage[1].m_vec *= float(-0.5);
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
			double rating = iter1->ent_val;
			avgRating += rating;
			numRatings++;
		}
	}
	return avgRating/numRatings;
}

void HierarchicalHybridMF::_update_bias() {
	float rvSuff2 = (float)m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float)0;
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


float HierarchicalHybridMF::test_rmse() {
	/// evaluate the RMSE over the training dataset
	float rmse = 0;
	size_t numRatings = 0;
	set<int64_t>& testUserIds = m_test_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t>& testItemIds = m_test_dataset.type_ent_ids[Entity::ENT_ITEM];
	/// initialize cold-start entities as the prior
	for(set<int64_t>::iterator iter = testUserIds.begin(); iter != testUserIds.end(); ++iter){
		if(m_train_dataset.ent_ids.find(*iter) == m_train_dataset.ent_ids.end()){
			/// caution: the covariance is yet setup properly
			m_entity[*iter] = m_user_prior_mean;
		}
	}
	for(set<int64_t>::iterator iter = testItemIds.begin(); iter != testItemIds.end(); ++iter){
		if(m_train_dataset.ent_ids.find(*iter) == m_train_dataset.ent_ids.end()){
			/// caution: the covariance is yet setup properly
			m_entity[*iter] = m_item_prior_mean;
		}
	}

	for (set<int64_t>::iterator iter = testUserIds.begin(); iter != testUserIds.end();
			++iter) {
		DiagMVGaussian& userLat = m_entity[*iter];
		vector<Interact>& ratingInteracts =
				m_test_dataset.ent_type_interacts[*iter][EntityInteraction::RATE_ITEM];
//		vector<Interact>& ratingInteracts1 =
//				m_train_dataset.ent_type_interacts[*iter][EntityInteraction::RATE_ITEM];
//		_dump_interact_array(ratingInteracts);
//		_dump_interact_array(ratingInteracts1);
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
	const size_t maxIter = 5;
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
		cout << ">>>>> update rating variance >>>>>" << endl;
		_update_rating_var();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		t.restart();
		cout << ">>>>> update global bias >>>>>" << endl;
		_update_bias();
		cout << ">>>>> time elapsed:" << t.elapsed() << endl;
		///
		float rmse = train_rmse();
		float testRmse = test_rmse();
		cout << ">>>>>>>>>>>>>>>>> train rmse:" << rmse << endl;
		cout << ">>>>>>>>>>>>>>>>> test rmse:" << testRmse << endl;

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
		cout << m_dataset << endl;
		cout << m_train_dataset << endl;
		cout << m_test_dataset << endl;
		cout << m_cs_dataset << endl;
		cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed()
				<< " >>>>>>>>>>>>>>" << endl;
		m_transport->close();
	} catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
	}
}

HierarchicalHybridMF::HierarchicalHybridMF(size_t const& latDim, bool diagGaussian) :
		m_num_users(0), m_num_items(0), m_num_features(0), m_socket(
				new TSocket("localhost", 9090)), m_transport(
				new TBufferedTransport(m_socket)), m_protocol(
				new TBinaryProtocol(m_transport)), m_client(m_protocol), m_lat_dim(
				latDim),m_diag_gaussian(diagGaussian),m_iter(0) {
	_init();
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
