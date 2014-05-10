/*
 * HHMFBias.cpp
 *
 *  Created on: May 7, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HHMFBias.h>

namespace recsys {

HHMFBias::HHMFBias() {
	// TODO Auto-generated constructor stub

}

void HHMFBias::_init_global_bias() {
	float avgRating = 0;
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		vector<Interact> const
				& ratings =
						m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		if (ratings.empty())
			continue;
		float userBias = m_user_bias[userId].m_mean;
		for (vector<Interact>::const_iterator iter1 = ratings.begin(); iter1
				< ratings.end(); ++iter1) {
			double rating = iter1->ent_val;
			int64_t itemId = iter1->ent_id;
			float itemBias = m_item_bias[itemId].m_mean;
			avgRating += (rating - userBias - itemBias);
			numRatings++;
		}
	}
	avgRating /= numRatings;
	m_global_bias.m_mean = avgRating;
}

void HHMFBias::_init_training() {
	/// initialize model variables
	/// initialize parent class
	HierarchicalHybridMF::_init_training();
	/// initialize user and item bias terms
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	for (set<int64_t>::const_iterator iter = userIds.begin(); iter
			!= userIds.end(); ++iter) {
		m_user_bias[*iter] = Gaussian(0, 1);
	}
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	for (set<int64_t>::const_iterator iter = itemIds.begin(); iter
			!= itemIds.end(); ++iter) {
		m_item_bias[*iter] = Gaussian(0, 1);
	}
	/// initialize prior for user and bias terms
	m_user_bias_mean_prior = Gaussian(0, 1);
	m_user_bias_var_prior = InverseGamma(3, 3);
	m_item_bias_mean_prior = Gaussian(0, 1);
	m_item_bias_var_prior = InverseGamma(3, 3);
	/// initialize user and item bias through alternative update
	_init_bias();
}

void HHMFBias::_update_global_bias() {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		float userBias = m_user_bias[userId].moment(1);
		DiagMVGaussian & userLat = m_entity[userId];
		vector<Interact> const
				& ratings =
						m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin(); iter1
				< ratings.end(); ++iter1) {
			int64_t itemId = iter1->ent_id;
			float itemBias = m_item_bias[itemId].moment(1);
			double rating = iter1->ent_val;
			DiagMVGaussian & itemLat = m_entity[itemId];
			float ip1stMoment, ip2ndMoment;
			_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
			/// the second moment is not used for this variable
			updateMessage[0].m_vec += (rating - ip1stMoment - userBias
					- itemBias);
			numRatings++;
		}
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	m_global_bias = updateMessage;
}

void HHMFBias::_update_user_bias_prior_mean(){
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	if (numUsers > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float)0.0;
		float fpCovSuff2 = m_user_bias_var_prior.suff_mean(2);
		updateMessage[1] = (vec) (-0.5 * numUsers * fpCovSuff2);
		for (set<int64_t>::iterator iter = userIds.begin();
				iter != userIds.end(); ++iter) {
			int64_t userId = *iter;
			Gaussian & userBias = m_user_bias[userId];
			float userBiasMean = userBias.moment(1);
			updateMessage[0] += userBiasMean;
		}
		updateMessage[0] *= fpCovSuff2;
		m_user_bias_mean_prior = updateMessage;
	}
}

void HHMFBias::_update_item_bias_prior_mean(){
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	if (numItems > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float)0.0;
		float fpCovSuff2 = m_item_bias_var_prior.suff_mean(2);
		updateMessage[1] = (vec) (-0.5 * numItems * fpCovSuff2);
		for (set<int64_t>::iterator iter = itemIds.begin();
				iter != itemIds.end(); ++iter) {
			int64_t itemId = *iter;
			Gaussian & itemBias = m_item_bias[itemId];
			float itemBiasMean = itemBias.moment(1);
			updateMessage[0] += itemBiasMean;
		}
		updateMessage[0] *= fpCovSuff2;
		m_item_bias_mean_prior = updateMessage;
	}
}

void HHMFBias::_update_user_bias_prior_var(){
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	if (numUsers > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float)0.0;
		/// the second component of sufficient statistics of the InverseGamma distribution
		updateMessage[1] = -0.5 * numUsers;
		for (set<int64_t>::iterator iter = userIds.begin();
				iter != userIds.end(); ++iter) {
			int64_t userId = *iter;
			/// from individual user bias posterior distribution
			float userBiasMeanFirstMoment = m_user_bias[userId].moment(1);
			float userBiasSecondMoment = m_user_bias[userId].moment(2);

			/// from user bias mean prior distribution
			float priorMeanFirstMoment = m_user_bias_mean_prior.moment(1);
			float priorMeanSecondMoment = m_user_bias_mean_prior.moment(2);
			updateMessage[0] += (userBiasSecondMoment + priorMeanSecondMoment - 2 * userBiasMeanFirstMoment * priorMeanFirstMoment);
		}
		/// don't forget the coefficient -0.5
		updateMessage[0] *= (-0.5);
		m_user_bias_var_prior = updateMessage;
	}
}

void HHMFBias::_update_item_bias_prior_var(){
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	if (numItems > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float)0.0;

		/// the second component of sufficient statistics of the InverseGamma distribution
		updateMessage[1] = -0.5 * numItems;
		for (set<int64_t>::iterator iter = itemIds.begin();
				iter != itemIds.end(); ++iter) {
			int64_t itemId = *iter;
			/// from individual item bias posterior distribution
			float itemBiasMeanFirstMoment = m_item_bias[itemId].moment(1);
			float itemBiasSecondMoment = m_item_bias[itemId].moment(2);

			/// from item bias mean prior distribution
			float priorMeanFirstMoment = m_item_bias_mean_prior.moment(1);
			float priorMeanSecondMoment = m_item_bias_mean_prior.moment(2);
			updateMessage[0] += (itemBiasSecondMoment + priorMeanSecondMoment - 2 * itemBiasMeanFirstMoment * priorMeanFirstMoment);
		}
		/// don't forget the coefficient -0.5
		updateMessage[0] *= (-0.5);
		m_item_bias_var_prior = updateMessage;
	}
}


void HHMFBias::_update_user_bias_prior(){
	/// update user bias prior distribution
	_update_user_bias_prior_mean();
	_update_user_bias_prior_var();
}

void HHMFBias::_update_item_bias_prior(){
	/// update item prior bias distribution
	_update_item_bias_prior_mean();
	_update_item_bias_prior_var();
}


void HHMFBias::_update_user_bias_from_rating(int64_t const& userId,	vector<Interact>& ratingInteracts) {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	size_t numRatings = ratingInteracts.size();
	DiagMVGaussian & userLat = m_entity[userId];
	for (vector<Interact>::const_iterator iter1 = ratingInteracts.begin(); iter1
			< ratingInteracts.end(); ++iter1) {
		int64_t itemId = iter1->ent_id;
		double rating = iter1->ent_val;
		DiagMVGaussian & itemLat = m_entity[itemId];
		float ip1stMoment, ip2ndMoment;
		_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
		/// substract innerproduct mean, global bias mean and item bias mean from the rating
		updateMessage[0].m_vec += (rating - ip1stMoment - m_item_bias[itemId].moment(1) - m_global_bias.moment(1));
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	/// update the bias
	m_user_bias += updateMessage;
}

void HHMFBias::_update_item_bias_from_rating(int64_t const& itemId, vector<Interact>& ratingInteracts) {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	size_t numRatings = ratingInteracts.size();
	DiagMVGaussian & itemLat = m_entity[itemId];
	for (vector<Interact>::const_iterator iter1 = ratingInteracts.begin(); iter1
			< ratingInteracts.end(); ++iter1) {
		int64_t userId = iter1->ent_id;
		double rating = iter1->ent_val;
		DiagMVGaussian & userLat = m_entity[userId];
		float ip1stMoment, ip2ndMoment;
		_lat_ip_moments(userLat, itemLat, ip1stMoment, ip2ndMoment);
		/// substract inner product mean, global bias mean and user bias mean from the rating
		updateMessage[0].m_vec += (rating - ip1stMoment - m_user_bias[userId].moment(1) - m_global_bias.moment(1));
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	/// update the bias
	m_item_bias += updateMessage;
}

void HHMFBias::_update_user_bias_from_prior(int64_t const& userId,
		vector<Interact>& featureInteracts) {
	DistParamBundle message(2);
	float upCovSuff2 = m_user_bias_var_prior.suff_mean(2);
	float userBiasMean = m_user_bias_mean_prior.moment(1);
	/// not considering mapping feature to bias for now
	message[0] = upCovSuff2 * userBiasMean;
	message[1] = (-0.5 * upCovSuff2);
	m_user_bias[userId] += message;
}

void HHMFBias::_update_item_bias_from_prior(int64_t const& itemId,
		vector<Interact>& featureInteracts) {
	DistParamBundle message(2);
	float upCovSuff2 = m_item_bias_var_prior.suff_mean(2);
	float itemBiasMean = m_item_bias_mean_prior.moment(1);
	/// not considering mapping feature to bias for now
	message[0] = upCovSuff2 * itemBiasMean;
	message[1] = (-0.5 * upCovSuff2);
	m_item_bias[itemId] += message;
}

void HHMFBias::_update_user_bias(int64_t const& userId, map<int8_t, vector<
		Interact> > & typeInteracts) {
	_update_user_bias_from_prior(userId,
			typeInteracts[itemId][EntityInteraction::ADD_FEATURE]);
	_update_user_bias_from_rating(userId,
			typeInteracts[itemId][EntityInteraction::RATE_ITEM]);

}

void HHMFBias::_update_item_bias(int64_t const& itemId, map<int8_t, vector<
		Interact> > & typeInteracts) {
	_update_item_bias_from_prior(itemId,
			typeInteracts[itemId][EntityInteraction::ADD_FEATURE]);
	_update_item_bias_from_rating(itemId,
			typeInteracts[itemId][EntityInteraction::RATE_ITEM]);
}


//// TODO: not finish yet.
void HHMFBias::_rating_bias_moments(float rating,int64_t const& userId, int64_t const& itemId,
		float & firstMoment, float& secondMoment){
	float bias1stMoment = m_global_bias.moment(1);
	float bias2ndMoment = m_global_bias.moment(2);
	firstMoment = rating - bias1stMoment;
	secondMoment = rating * rating - 2 * bias1stMoment * rating + bias2ndMoment;

}

void HHMFBias::_update_rating_var() {
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

void HHMFBias::_update_entity_from_ratings(int64_t const& entityId, map<int8_t,
		vector<Interact> > & typeInteracts) {
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
		/// consider user and item bias
		tmpRating -= (m_global_bias.moment(1) - (float)m_user_bias[userId].moment(1) - (float)m_item_bias[itemId].moment(1));
		vec itemLatMean = itemLat.moment(1);
		vec itemLatCov = itemLat.moment(2);
		vec update1 = (tmpRating * rvsuff2 * itemLatMean);
		vec update2 = (-0.5 * rvsuff2 * itemLatCov);
		message[0] += update1;
		message[1] += update2;
	}
	if (ratingInteracts.size() > 0)
		m_entity[entityId] += message;
}

RecModel::TrainIterLog HHMFBias::_train_update() {

}

void HHMFBias::_init_user_bias() {
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	float averageUserBias = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin(); iter
			!= userIds.end(); ++iter) {
		/// get rating interactions
		int64_t userId = *iter;
		vector<Interact>
				& ratingInteracts =
						m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		float userBias = 0;
		if (ratingInteracts.empty())
			continue;
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin(); iter1
				< ratingInteracts.end(); ++iter1) {
			float rating = iter1->ent_val;
			int64_t itemId = iter1->ent_id;
			float itemBias = m_item_bias[itemId].m_mean;
			userBias += (rating - m_global_bias.m_mean - itemBias);
		}
		userBias /= ratingInteracts.size();
		/// update it
		m_user_bias[userId].m_mean = userBias;
		averageUserBias += userBias;
	}
	averageUserBias /= userIds.size();
	m_user_bias_mean_prior.m_mean = averageUserBias;
}

void HHMFBias::_init_item_bias() {
	/// update item bias
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	float averageItemBias = 0;
	for (set<int64_t>::const_iterator iter = itemIds.begin(); iter
			!= itemIds.end(); ++iter) {
		/// get rating interactions
		int64_t itemId = *iter;
		vector<Interact>
				& ratingInteracts =
						m_active_dataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM];
		if (ratingInteracts.empty())
			continue;
		float itemBias = 0;

		for (vector<Interact>::iterator iter1 = ratingInteracts.begin(); iter1
				< ratingInteracts.end(); ++iter1) {
			float rating = iter1->ent_val;
			int64_t userId = iter1->ent_id;
			float userBias = m_user_bias[userId].m_mean;
			itemBias += (rating - m_global_bias.m_mean - userBias);
		}
		itemBias /= ratingInteracts.size();
		/// update it
		m_item_bias[itemId] = Gaussian(itemBias, 1);
		averageItemBias += itemBias;
	}
	averageItemBias /= itemIds.size();
	m_item_bias_mean_prior.m_mean = averageItemBias;
}

float HHMFBias::_pred_error(int64_t const& userId, DatasetExt& dataset){
	float rmse = 0;
	vector<Interact>& userFeatureInteracts = dataset.ent_type_interacts[userId][EntityInteraction::ADD_FEATURE];
	for (vector<Interact>::iterator iter = userFeatureInteracts.begin();
			iter < userFeatureInteracts.end(); ++iter) {
		int64_t featId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(featId)
				== m_active_dataset.ent_ids.end()) {
			m_entity[featId].reset();
			_update_feature_from_prior(featId);
			/// no bias introduced for feature yet
		}
	}
	if (m_active_dataset.ent_ids.find(userId)
			== m_active_dataset.ent_ids.end()) {
		m_entity[userId].reset();
		_update_entity_from_prior_helper(userId,Entity::ENT_USER,userFeatureInteracts);
		/// update the user bias prior
		_update_user_bias_from_prior(userId,userFeatureInteracts);
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
			/// update item bias from prior
			_update_item_bias_from_prior(itemId,itemFeatureInteracts);
			/// not considering features yet
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
				+ (float) m_global_bias.moment(1) + (float)m_user_bias[userId].moment(1) + (float)m_item_bias[itemId].moment(1);
		float diff = predRating - ratingVal;
		rmse += (diff * diff);
	}
	return rmse;
}


void HHMFBias::_init_bias() {
	/// update global bias term
	cout << ">>> initialize bias terms through alternative updating" << endl;
	for (size_t iter_cnt = 0; iter_cnt < 20; iter_cnt++) {
		cout << "iteration: " << iter_cnt + 1;
		_init_global_bias();
		cout << ", global bias:" << m_global_bias.m_mean;
		_init_user_bias();
		cout << ", user bias mean:" << m_user_bias_mean_prior.m_mean;
		_init_item_bias();
		cout << ", item bias mean:" << m_item_bias_mean_prior.m_mean << endl;
	}
	cout << ">>> done!" << endl;
	///
}

HHMFBias::~HHMFBias() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
