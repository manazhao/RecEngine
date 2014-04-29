/*
 * AverageRatingModel.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/AverageRatingModel.h>

namespace recsys {

 void AverageRatingModel::_init_training(){
	 /// one iteration is enough to get the mean rating
	 m_model_param.m_max_iter = 1;
 }

 TrainIterLog AverageRatingModel::_train_update(){
	 set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	 m_avg_rating = 0;
	 size_t numRatings = 0;
	 for(set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter){
		 vector<rt::Interact>& ratingInteracts = m_active_dataset.ent_type_interacts[*iter][EntityInteraction::RATE_ITEM];
		 for(vector<rt::Interact>::iterator iter1 = ratingInteracts.begin(); iter1 < ratingInteracts.end(); ++iter1){
			 float rating = iter1->ent_val;
			 m_avg_rating += rating;
			 numRatings++;
		 }
	 }
	 m_avg_rating /= numRatings;
 }

 float AverageRatingModel::_pred_error(int64_t const& entityId, map<int8_t, vector<Interact> >& entityInteractMap){

 }

 void AverageRatingModel::_add_new_entity(int64_t const& entityId, int8_t const& entityType){

 }


AverageRatingModel::AverageRatingModel():m_avg_rating(0) {
	// TODO Auto-generated constructor stub

}

AverageRatingModel::~AverageRatingModel() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
