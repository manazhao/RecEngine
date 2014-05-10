/*
 * BayesianBiasModel.cpp
 *
 *  Created on: May 9, 2014
 *      Author: manazhao
 */

#include "BayesianBiasModel.h"

namespace recsys {

BayesianBiasModel::BayesianBiasModel() {
	// TODO Auto-generated constructor stub

}

BayesianBiasModel::~BayesianBiasModel() {
	// TODO Auto-generated destructor stub
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


}
