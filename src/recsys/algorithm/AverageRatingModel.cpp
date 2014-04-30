/*
 * AverageRatingModel.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/AverageRatingModel.h>

namespace recsys {

void AverageRatingModel::_init_training() {
	/// one iteration is enough to get the mean rating
	m_model_param.m_max_iter = 1;
}

RecModel::TrainIterLog AverageRatingModel::_train_update() {
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	m_avg_rating = 0;
	size_t numRatings = 0;
	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		vector<rt::Interact>& ratingInteracts =
				m_active_dataset.ent_type_interacts[*iter][EntityInteraction::RATE_ITEM];
		for (vector<rt::Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
			float rating = iter1->ent_val;
			m_avg_rating += rating;
			numRatings++;
		}
	}
	m_avg_rating /= numRatings;
	return RecModel::TrainIterLog();
}

float AverageRatingModel::_pred_error(int64_t const& entityId,
		map<int8_t, vector<Interact> >& entityInteractMap) {
	float error = 0;
	vector<Interact>& ratingInteracts =
			entityInteractMap[EntityInteraction::RATE_ITEM];
	for (vector<Interact>::iterator iter = ratingInteracts.begin();
			iter < ratingInteracts.end(); ++iter) {
		float rating = iter->ent_val;
		float diff = rating - m_avg_rating;
		error += (diff * diff);
	}
	return error;
}

void AverageRatingModel::_add_new_entity(int64_t const& entityId,
		int8_t const& entityType) {

}

vector<rt::Recommendation> AverageRatingModel::recommend(int64_t const& userId,
		map<int8_t, vector<rt::Interact> >& userInteracts) {
	return vector<rt::Recommendation>();
}

string AverageRatingModel::model_summary() {
	stringstream ss;
	ss << "# of users: "
			<< m_active_dataset.type_ent_ids[Entity::ENT_USER].size() << "\n";
	ss << "# of items: "
			<< m_active_dataset.type_ent_ids[Entity::ENT_ITEM].size() << "\n";
	ss << "# of features: "
			<< m_active_dataset.type_ent_ids[Entity::ENT_FEATURE].size()
			<< "\n";
	ss << "average rating:" << m_avg_rating << endl;
	return ss.str();
}

AverageRatingModel::AverageRatingModel() :
		m_avg_rating(0) {
	// TODO Auto-generated constructor stub

}

AverageRatingModel::~AverageRatingModel() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
