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

void BayesianBiasModel::dump_model_text(string const& filePrefix) {
	//// dump the  user bias information
	string userBiasFile = filePrefix + ".user.bias.csv";
	ofstream ofs;
	ofs.open(userBiasFile.c_str(), std::ofstream::out);
	assert(ofs.good());
	for (map<int64_t, Gaussian>::iterator iter = m_user_bias.begin();
			iter != m_user_bias.end(); ++iter) {
		/// dump the bias mean and variance
		ofs << iter->first << "," << iter->second.moment(1) << ","
				<< iter->second.moment(2) << "\n";
	}
	ofs.close();

	//// dump the item bias
	string itemBiasFile = filePrefix + ".item.bias.csv";
	ofs.open(itemBiasFile.c_str(), std::ofstream::out);
	assert(ofs.good());
	for (map<int64_t, Gaussian>::iterator iter = m_item_bias.begin();
			iter != m_item_bias.end(); ++iter) {
		/// dump the bias mean and variance
		ofs << iter->first << "," << iter->second.moment(1) << ","
				<< iter->second.moment(2) << "\n";
	}
	ofs.close();

	/// dump the prior information
	string priorFile = filePrefix + ".prior.json";
	ofs.open(priorFile.c_str(), std::ofstream::out);
	assert(ofs.good());

	js::Object priorObj;
	/// global bias
	float biasMean = (float) m_global_bias.moment(1);
	float biasVar = m_global_bias.moment(2) - biasMean * biasMean;
	priorObj["global.bias"]["mean"] = js::Number(biasMean);
	priorObj["global.bias"]["var"] = js::Number(biasVar);

	/// rating var
	biasVar = m_rating_var.moment(1);
	priorObj["rating.var"] = js::Number(biasVar);
	/// user bias prior
	biasMean = m_user_bias_mean_prior.moment(1);
	biasVar = m_user_bias_var_prior.moment(1);
	priorObj["user.bias"]["mean"] = js::Number(biasMean);
	priorObj["user.bias"]["var"] = js::Number(biasVar);

	/// item bias prior
	biasMean = m_item_bias_mean_prior.moment(1);
	biasVar = m_item_bias_var_prior.moment(1);
	priorObj["item.bias"]["mean"] = js::Number(biasMean);
	priorObj["item.bias"]["var"] = js::Number(biasVar);

	/// write the object to string
	js::Writer::Write(priorObj,ofs);
	ofs.close();

	/// dump ratings
	string ratingFile = filePrefix + ".rating.csv";
	_dump_ratings(ratingFile);
}

vector<rt::Recommendation> BayesianBiasModel::recommend(int64_t const& userId,
		map<int8_t, vector<rt::Interact> >& userInteracts) {
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	if (userIds.find(userId) == userIds.end()) {
		cout << "add new user:" << userId << endl;
		_add_new_entity(userId, Entity::ENT_USER);

		vector<rt::Interact>& featureInteracts =
				userInteracts[EntityInteraction::ADD_FEATURE];
		for (vector<rt::Interact>::iterator iter = featureInteracts.begin();
				iter < featureInteracts.end(); ++iter) {
			int64_t entId = iter->ent_id;
			/// TODO: modeling bias by considering feature
		}

		/// initialize user bias from prior mean
		m_user_bias[userId].reset();
		_update_user_bias_from_prior(userId, featureInteracts);

		/// update bias from ratings
		_update_user_bias_from_ratings(userId,
				userInteracts[EntityInteraction::RATE_ITEM]);
	}

	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<rt::Recommendation> recList;
	for (set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end();
			++iter) {
		int64_t itemId = *iter;
		/// calculate the inner product
		rt::Recommendation rec;
		rec.score = (float) m_global_bias.moment(1)
				+ (float) m_user_bias[userId].moment(1)
				+ (float) m_item_bias[itemId].moment(1);
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

void BayesianBiasModel::_init_global_bias() {
	float avgRating = 0;
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		vector<Interact> const & ratings =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		if (ratings.empty())
			continue;
		float userBias = m_user_bias[userId].m_mean;
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
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

void BayesianBiasModel::_init_training() {

	/// initialize user bias variables
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		m_user_bias[*iter] = Gaussian(0, 1);
	}

	/// initialize item bias variables
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	for (set<int64_t>::const_iterator iter = itemIds.begin();
			iter != itemIds.end(); ++iter) {
		m_item_bias[*iter] = Gaussian(0, 1);
	}

	/// initialize prior for user and bias terms
	m_user_bias_mean_prior = Gaussian(0, 1);
	m_user_bias_var_prior = InverseGamma(3, 3);
	m_item_bias_mean_prior = Gaussian(0, 1);
	m_item_bias_var_prior = InverseGamma(3, 3);
	m_rating_var = InverseGamma(3,3);
	/// initialize user and item bias through alternative update
	_init_bias();
}

void BayesianBiasModel::_update_global_bias() {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		float userBias = m_user_bias[userId].moment(1);
		vector<Interact> const & ratings =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			int64_t itemId = iter1->ent_id;
			float itemBias = m_item_bias[itemId].moment(1);
			double rating = iter1->ent_val;
			/// the second moment is not used for this variable
			updateMessage[0].m_vec += (rating - userBias - itemBias);
			numRatings++;
		}
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	m_global_bias = updateMessage;
}

void BayesianBiasModel::_update_user_bias_prior_mean() {
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	if (numUsers > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float) 0.0;
		float fpCovSuff2 = m_user_bias_var_prior.suff_mean(2);
		updateMessage[1] = (-0.5 * numUsers * fpCovSuff2);
		for (set<int64_t>::iterator iter = userIds.begin();
				iter != userIds.end(); ++iter) {
			int64_t userId = *iter;
			Gaussian & userBias = m_user_bias[userId];
			float userBiasMean = userBias.moment(1);
			updateMessage[0] += userBiasMean;
		}
		updateMessage[0].m_vec *= fpCovSuff2;
		m_user_bias_mean_prior = updateMessage;
	}
}

void BayesianBiasModel::_update_item_bias_prior_mean() {
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	if (numItems > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float) 0.0;
		float fpCovSuff2 = m_item_bias_var_prior.suff_mean(2);
		updateMessage[1] = (-0.5 * numItems * fpCovSuff2);
		for (set<int64_t>::iterator iter = itemIds.begin();
				iter != itemIds.end(); ++iter) {
			int64_t itemId = *iter;
			Gaussian & itemBias = m_item_bias[itemId];
			float itemBiasMean = itemBias.moment(1);
			updateMessage[0] += itemBiasMean;
		}
		updateMessage[0].m_vec *= fpCovSuff2;
		m_item_bias_mean_prior = updateMessage;
	}
}

void BayesianBiasModel::_update_user_bias_prior_var() {
	set<int64_t> & userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numUsers = userIds.size();
	if (numUsers > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float) 0.0;
		/// the second component of sufficient statistics of the InverseGamma distribution
		updateMessage[0] = -0.5 * numUsers;
		for (set<int64_t>::iterator iter = userIds.begin();
				iter != userIds.end(); ++iter) {
			int64_t userId = *iter;
			/// from individual user bias posterior distribution
			float userBiasMeanFirstMoment = m_user_bias[userId].moment(1);
			float userBiasSecondMoment = m_user_bias[userId].moment(2);

			/// from user bias mean prior distribution
			float priorMeanFirstMoment = m_user_bias_mean_prior.moment(1);
			float priorMeanSecondMoment = m_user_bias_mean_prior.moment(2);
			updateMessage[1] += (userBiasSecondMoment + priorMeanSecondMoment
					- 2 * userBiasMeanFirstMoment * priorMeanFirstMoment);
		}
		/// don't forget the coefficient -0.5
		updateMessage[1].m_vec *= (-0.5);
		m_user_bias_var_prior = updateMessage;
	}
}

void BayesianBiasModel::_update_item_bias_prior_var() {
	set<int64_t> & itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	size_t numItems = itemIds.size();
	if (numItems > 0) {
		DistParamBundle updateMessage(2);
		updateMessage[0] = updateMessage[1] = (float) 0.0;

		/// the second component of sufficient statistics of the InverseGamma distribution
		updateMessage[0] = -0.5 * numItems;
		for (set<int64_t>::iterator iter = itemIds.begin();
				iter != itemIds.end(); ++iter) {
			int64_t itemId = *iter;
			/// from individual item bias posterior distribution
			float itemBiasMeanFirstMoment = m_item_bias[itemId].moment(1);
			float itemBiasSecondMoment = m_item_bias[itemId].moment(2);

			/// from item bias mean prior distribution
			float priorMeanFirstMoment = m_item_bias_mean_prior.moment(1);
			float priorMeanSecondMoment = m_item_bias_mean_prior.moment(2);
			updateMessage[1] += (itemBiasSecondMoment + priorMeanSecondMoment
					- 2 * itemBiasMeanFirstMoment * priorMeanFirstMoment);
		}
		/// don't forget the coefficient -0.5
		updateMessage[1].m_vec *= (-0.5);
		m_item_bias_var_prior = updateMessage;
	}
}

void BayesianBiasModel::_update_user_bias_prior() {
	/// update user bias prior distribution
	_update_user_bias_prior_mean();
	_update_user_bias_prior_var();
}

void BayesianBiasModel::_update_item_bias_prior() {
	/// update item prior bias distribution
	_update_item_bias_prior_mean();
	_update_item_bias_prior_var();
}

void BayesianBiasModel::_update_user_bias_from_ratings(int64_t const& userId,
		vector<Interact>& ratingInteracts) {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	size_t numRatings = ratingInteracts.size();
	for (vector<Interact>::const_iterator iter1 = ratingInteracts.begin();
			iter1 < ratingInteracts.end(); ++iter1) {
		int64_t itemId = iter1->ent_id;
		double rating = iter1->ent_val;
		/// substract innerproduct mean, global bias mean and item bias mean from the rating
		updateMessage[0].m_vec += (rating - m_item_bias[itemId].moment(1)
				- m_global_bias.moment(1));
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	/// update the bias
	m_user_bias[userId] += updateMessage;
}

void BayesianBiasModel::_update_item_bias_from_ratings(int64_t const& itemId,
		vector<Interact>& ratingInteracts) {
	float rvSuff2 = (float) m_rating_var.suff_mean(2);
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	size_t numRatings = ratingInteracts.size();
	for (vector<Interact>::const_iterator iter1 = ratingInteracts.begin();
			iter1 < ratingInteracts.end(); ++iter1) {
		int64_t userId = iter1->ent_id;
		double rating = iter1->ent_val;
		/// substract inner product mean, global bias mean and user bias mean from the rating
		updateMessage[0].m_vec += (rating - m_user_bias[userId].moment(1)
				- m_global_bias.moment(1));
	}
	updateMessage[0].m_vec *= (rvSuff2);
	updateMessage[1].m_vec = -0.5 * numRatings * rvSuff2;
	/// update the bias
	m_item_bias[itemId] += updateMessage;
}

void BayesianBiasModel::_update_user_bias_from_prior(int64_t const& userId,
		vector<Interact>& featureInteracts) {
	DistParamBundle message(2);
	float upCovSuff2 = m_user_bias_var_prior.suff_mean(2);
	float userBiasMean = m_user_bias_mean_prior.moment(1);
	/// not considering mapping feature to bias for now
	message[0] = upCovSuff2 * userBiasMean;
	message[1] = (-0.5 * upCovSuff2);
	m_user_bias[userId] += message;
}

void BayesianBiasModel::_update_item_bias_from_prior(int64_t const& itemId,
		vector<Interact>& featureInteracts) {
	DistParamBundle message(2);
	float upCovSuff2 = m_item_bias_var_prior.suff_mean(2);
	float itemBiasMean = m_item_bias_mean_prior.moment(1);
	/// not considering mapping feature to bias for now
	message[0] = upCovSuff2 * itemBiasMean;
	message[1] = (-0.5 * upCovSuff2);
	m_item_bias[itemId] += message;
}

void BayesianBiasModel::_update_user_bias(int64_t const& userId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	m_user_bias[userId].reset();
	_update_user_bias_from_prior(userId,
			typeInteracts[EntityInteraction::ADD_FEATURE]);
	_update_user_bias_from_ratings(userId,
			typeInteracts[EntityInteraction::RATE_ITEM]);

}

void BayesianBiasModel::_update_item_bias(int64_t const& itemId,
		map<int8_t, vector<Interact> > & typeInteracts) {
	m_item_bias[itemId].reset();
	_update_item_bias_from_prior(itemId,
			typeInteracts[EntityInteraction::ADD_FEATURE]);
	_update_item_bias_from_ratings(itemId,
			typeInteracts[EntityInteraction::RATE_ITEM]);
}

void BayesianBiasModel::_rating_bias_moments(float rating,
		int64_t const& userId, int64_t const& itemId, float & firstMoment,
		float& secondMoment) {
	float rM1 = rating;
	float rM2 = rating * rating;
	float gbM1 = m_global_bias.moment(1);
	float gbM2 = m_global_bias.moment(2);
	float ubM1 = m_user_bias[userId].moment(1);
	float ubM2 = m_user_bias[userId].moment(2);
	float ibM1 = m_item_bias[itemId].moment(1);
	float ibM2 = m_item_bias[itemId].moment(2);
	/// initialize first moment and second moment of result
	firstMoment = rM1;
	secondMoment = rM2;
	/// evaluate it recursively
	sub_moments(firstMoment, secondMoment, gbM1, gbM2, firstMoment,
			secondMoment);
	sub_moments(firstMoment, secondMoment, ubM1, ubM2, firstMoment,
			secondMoment);
	sub_moments(firstMoment, secondMoment, ibM1, ibM2, firstMoment,
			secondMoment);
}

void BayesianBiasModel::_update_rating_var() {
	/// update the rating variance
	/// go through all ratings
	DistParamBundle updateMessage(2);
	updateMessage[0] = updateMessage[1] = (float) 0;
	set<int64_t> &userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	size_t numRatings = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		int64_t userId = *iter;
		vector<Interact> &ratings =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];

		/// go through the ratings
		for (vector<Interact>::const_iterator iter1 = ratings.begin();
				iter1 < ratings.end(); ++iter1) {
			numRatings++;
			int64_t itemId = iter1->ent_id;
			double rating = iter1->ent_val;
			float rating1stMoment, rating2ndMoment;
			/// considering global bias, user bias and item bias
			_rating_bias_moments(rating, userId, itemId, rating1stMoment,
					rating2ndMoment);
			updateMessage[1] += rating2ndMoment;
		}

	}
	updateMessage[0] = float(-0.5) * numRatings;
	updateMessage[1].m_vec *= float(-0.5);
	m_rating_var = updateMessage;
}

RecModel::TrainIterLog BayesianBiasModel::_train_update() {
	/// set the active dataset
	assert(!m_active_dataset.ent_type_interacts.empty());
	/// get the variables
	set<int64_t> const& userIds =
			m_active_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t> const& itemIds =
			m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<map<int8_t, vector<Interact> > >& type_interacts =
			m_active_dataset.ent_type_interacts;

	typedef set<int64_t>::iterator id_set_iter;
	TrainIterLog iterLog;

	/// update user bias
	for (id_set_iter iter = userIds.begin(); iter != userIds.end(); ++iter) {
		int64_t entityId = *iter;
		map<int8_t, vector<Interact> > & tmpEntityInteracts =
				type_interacts[entityId];
		_update_user_bias(entityId, tmpEntityInteracts);
	}
	/// update user bias prior
	_update_user_bias_prior();

	/// update item bias
	for (id_set_iter iter = itemIds.begin(); iter != itemIds.end(); ++iter) {
		int64_t entityId = *iter;
		map<int8_t, vector<Interact> > & tmpEntityInteracts =
				type_interacts[entityId];
		_update_item_bias(entityId, tmpEntityInteracts);
	}
	/// update item bias prior
	_update_item_bias_prior();

	//// update rating variance and global rating bias
	_update_global_bias();
	_update_rating_var();
	cout << "rating var:" << m_rating_var << endl;
	return iterLog;
}

void BayesianBiasModel::_init_user_bias() {
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	float averageUserBias = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		/// get rating interactions
		int64_t userId = *iter;
		vector<Interact> & ratingInteracts =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		float userBias = 0;
		if (ratingInteracts.empty())
			continue;
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
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

void BayesianBiasModel::_init_item_bias() {
	/// update item bias
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	float averageItemBias = 0;
	for (set<int64_t>::const_iterator iter = itemIds.begin();
			iter != itemIds.end(); ++iter) {
		/// get rating interactions
		int64_t itemId = *iter;
		vector<Interact> & ratingInteracts =
				m_active_dataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM];
		if (ratingInteracts.empty())
			continue;
		float itemBias = 0;

		for (vector<Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
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

float BayesianBiasModel::_pred_error(int64_t const& userId,
		DatasetExt& dataset) {
	float rmse = 0;
	vector<Interact>& userFeatureInteracts =
			dataset.ent_type_interacts[userId][EntityInteraction::ADD_FEATURE];
	for (vector<Interact>::iterator iter = userFeatureInteracts.begin();
			iter < userFeatureInteracts.end(); ++iter) {
		int64_t featId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(featId)
				== m_active_dataset.ent_ids.end()) {
			// TODO: modeling feature on bias
		}
	}
	if (m_active_dataset.ent_ids.find(userId)
			== m_active_dataset.ent_ids.end()) {
		/// update the user bias prior
		m_user_bias[userId] = Gaussian(0, 1);
		m_user_bias[userId].reset();
		_update_user_bias_from_prior(userId, userFeatureInteracts);
	}

	vector<Interact>& ratingInteracts =
			dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		int64_t itemId = iter->ent_id;
		if (m_active_dataset.ent_ids.find(itemId)
				== m_active_dataset.ent_ids.end()) {
			/// update item bias from prior
			m_item_bias[itemId] = Gaussian(0, 1);
			m_item_bias[itemId].reset();
			_update_item_bias_from_prior(itemId,
					dataset.ent_type_interacts[itemId][EntityInteraction::ADD_FEATURE]);
		}
	}

	/// evaluate prediction error in terms of rmse
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		float ratingVal = iter->ent_val;
		int64_t itemId = iter->ent_id;
		float predRating = (float) m_global_bias.moment(1)
				+ (float) m_user_bias[userId].moment(1)
				+ (float) m_item_bias[itemId].moment(1);
		float diff = predRating - ratingVal;
		rmse += (diff * diff);
	}
	return rmse;
}

void BayesianBiasModel::_add_new_entity(int64_t const& entityId,
		int8_t const& entityType) {
	/// add bias information
	switch (entityType) {
	case Entity::ENT_USER:
		m_user_bias[entityId] = Gaussian(0, 1);
		break;
	case Entity::ENT_ITEM:
		m_item_bias[entityId] = Gaussian(0, 1);
		break;
	default:
		break;
	}
}


void BayesianBiasModel::_dump_ratings(string const& fileName){
	ofstream ofs;
	ofs.open(fileName.c_str());
	assert(ofs.good());
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	for(set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter){
		int64_t userId = *iter;
		vector<Interact>& ratings = m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for(size_t i = 0; i < ratings.size(); i++){
			Interact& ratingInt = ratings[i];
			ofs << userId << "," << ratingInt.ent_id << "," << ratingInt.ent_val << "\n";
		}
	}
	ofs.close();
}


string BayesianBiasModel::model_summary() {
	stringstream ss;
	float biasVarMean = m_user_bias_var_prior.moment(1);
	ss << "user bias mean:" << m_user_bias_mean_prior.moment(1) << ", variance:"
			<< biasVarMean << ", variance of variance, "
			<< m_user_bias_var_prior.moment(1) - biasVarMean * biasVarMean
			<< "\n";
	biasVarMean = m_item_bias_var_prior.moment(1);
	ss << "item bias mean:" << m_item_bias_mean_prior.moment(1) << ", variance:"
			<< biasVarMean << ", variance of variance, "
			<< m_item_bias_var_prior.moment(1) - biasVarMean * biasVarMean
			<< "\n";
	float gBiasMean = m_global_bias.moment(1);
	biasVarMean = m_rating_var.moment(1);
//	ss << "rating variance:" << m_rating_var << "\n";
	ss << "global bias mean:" << gBiasMean << ", variance:"
			<< biasVarMean << ", variance of variance:" << m_rating_var.moment(2) - biasVarMean * biasVarMean << "\n";
	return ss.str();
}

void BayesianBiasModel::_init_bias() {
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


}
