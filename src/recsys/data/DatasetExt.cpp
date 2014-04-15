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

void DatasetExt::filter_entity_interactions(
		vector<map<int8_t, vector<Interact> > > const& entTypeInteractions) {
	/// only keep those interactions the both entities of which are in the entity id set
	for (set<int64_t>::iterator iter = ent_ids.begin(); iter
			!= ent_ids.end(); ++iter) {
		/// get the interactions
		int64_t fromEntId = *iter;
		map<int8_t, vector<Interact> > const& tmpTypeInteracts =
				entTypeInteractions[*iter];
		for (map<int8_t, vector<Interact> >::const_iterator iter1 =
				tmpTypeInteracts.begin(); iter1 != tmpTypeInteracts.end(); ++iter1) {
			int8_t intType = iter1->first;
			for (vector<Interact>::const_iterator iter2 = iter1->second.begin(); iter2
					< iter1->second.end(); ++iter2) {
				_filter_entity_interaction_helper(intType, fromEntId,
						*iter2);
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

}
