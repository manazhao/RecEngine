/*
 * Dataset.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: manazhao
 */

#include "DatasetExt.h"

namespace recsys {

DatasetExt::DatasetExt(int64_t const& numEntities){
	ent_type_interacts.resize(numEntities);
}

void DatasetExt::add_entity(int8_t const& type, int64_t const& id) {
	type_ent_ids[type].insert(id);
	ent_ids.insert(id);
	m_id_type_map[id] = type;
}


void DatasetExt::prepare_id_type_map(){
	for(map<int8_t,set<int64_t> >::iterator iter = type_ent_ids.begin(); iter != type_ent_ids.end(); ++iter){
		for(set<int64_t>::iterator iter1 = iter->second.begin(); iter1 != iter->second.end(); ++iter1){
			m_id_type_map[*iter1] = iter->first;
		}
	}
}

void DatasetExt::dump_rating_interact(){
	set<int64_t>& userIds = type_ent_ids[Entity::ENT_USER];
	for(set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end(); ++iter){
		int64_t userId = *iter;
		cout << "user id:" << userId << " - ";
		vector<Interact>& ratingInteracts = ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for(vector<Interact>::iterator iter1 = ratingInteracts.begin(); iter1 < ratingInteracts.end(); ++iter1){
			cout  << iter1->ent_id << ",";
		}
		cout << "\n";
		break;
	}
}

void DatasetExt::verify_interaction(){
	/// check whether the user id and item id are different
	/// very basic checking
	typedef vector<std::map<int8_t, std::vector<Interact> > >::iterator interact_vec_iter;
	for(interact_vec_iter iter = ent_type_interacts.begin(); iter < ent_type_interacts.end(); ++iter){
		map<int8_t, std::vector<Interact> >& interactMap = *iter;
		for(map<int8_t, std::vector<Interact> >::iterator iter1 = interactMap.begin(); iter1 != interactMap.end(); ++iter1){
			vector<Interact>& interactVec = iter1->second;
			for(vector<Interact>::iterator iter2 = interactVec.begin(); iter2 < interactVec.end(); ++iter2){
				assert(iter2->ent_val);
			}
		}
	}
}

void DatasetExt::_filter_entity_interaction_helper(int8_t const& type,
		int64_t const& from_ent_id, Interact const& interact) {
	/// check the existence of interaction
	if (entity_exist(from_ent_id) && entity_exist(interact.ent_id)) {
		ent_type_interacts[from_ent_id][type].push_back(interact);
	}
}

ostream& operator<< (ostream& oss, DatasetExt const& rhs){
	DatasetExt& rhs1 = const_cast<DatasetExt&>(rhs);
	oss << "# of users:" << rhs1.type_ent_ids[Entity::ENT_USER].size() << endl;
	oss << "# of items:" << rhs1.type_ent_ids[Entity::ENT_ITEM].size() << endl;
	oss << "# of features:" << rhs1.type_ent_ids[Entity::ENT_FEATURE].size() << endl;
	return oss;
}

}
