/*
 * PopularityModel.cpp
 *
 *  Created on: Apr 30, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/PopularityModel.h>

namespace recsys {

PopularityModel::PopularityModel() {
	// TODO Auto-generated constructor stub

}

PopularityModel::~PopularityModel() {
	// TODO Auto-generated destructor stub
}

void PopularityModel::_init_training(){
	m_model_param.m_max_iter = 1;
}

PopularityModel::TrainIterLog PopularityModel::_train_update(){
	//// sort the items by decreasing number of ratings
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	m_pop_sort_items.clear();
	for(set<int64_t>::iterator iter = itemIds.begin(); iter != itemIds.end(); ++iter){
		int64_t itemId = *iter;
		vector<Interact>& ratingInteracts = m_active_dataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM];
		Recommendation tmpRec;
		tmpRec.score = ratingInteracts.size();
		tmpRec.id = lexical_cast<string>(itemId);
		tmpRec.type = 0;
		m_pop_sort_items.push_back(tmpRec);
	}
	sort(m_pop_sort_items.begin(),m_pop_sort_items.end(),RecommendationComparator());

	return TrainIterLog();
}

float PopularityModel::_pred_error(int64_t const& entityId,
		map<int8_t, vector<Interact> >& entityInteractMap){
	return 0;
}

void PopularityModel::_add_new_entity(int64_t const& entityId,
		int8_t const& entityType){

}

 vector<rt::Recommendation> PopularityModel::recommend(int64_t const& userId, map<int8_t, vector<rt::Interact> >& userInteracts){
	 /// return top 20 results
	 vector<Recommendation> recList;
	 recList.insert(recList.end(),m_pop_sort_items.begin(),m_pop_sort_items.begin()  + 20);
	 return recList;
 }

 string PopularityModel::model_summary(){
		stringstream ss;
		ss << "# of users: "
				<< m_active_dataset.type_ent_ids[Entity::ENT_USER].size() << "\n";
		ss << "# of items: "
				<< m_active_dataset.type_ent_ids[Entity::ENT_ITEM].size() << "\n";
		ss << "# of features: "
				<< m_active_dataset.type_ent_ids[Entity::ENT_FEATURE].size()
				<< "\n";
		ss << "------------- Most popular items ----------------" << endl;
		/// print the top t recommendations
		for(size_t i = 0; i < 5; i++){
			Recommendation& tmpRec = m_pop_sort_items[i];
			cout << "id:" << tmpRec.id << " , score:" << tmpRec.score << endl;
		}

		return ss.str();
 }


} /* namespace recsys */
