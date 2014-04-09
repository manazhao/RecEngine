/*
 * Dataset.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: manazhao
 */

#include "Dataset.h"

namespace recsys {

Dataset::Dataset(int64_t const& maxId) :
	m_ent_type_interacts(maxId),
			m_id_id_map(maxId, set<int64_t> ()) {
}

void Dataset::add_entity(ushort const& type, int64_t const& id) {
	m_type_ent_ids[type].insert(id);
}

void Dataset::build_interaction_lookup(){
	m_id_id_map.clear();
	m_id_id_map.resize(m_ent_type_interacts.size());
	for(size_t i = 0; i < m_ent_type_interacts.size(); i++){
		map<int8_t,vector<Interact> >& typeInteracts = m_ent_type_interacts[i];
		for(map<int8_t, vector<Interact> >::iterator iter = typeInteracts.begin(); iter != typeInteracts.end(); ++iter){
			for(vector<Interact>::iterator iter2 = iter->second.begin(); iter2 < iter->second.end(); ++iter2){
				m_id_id_map[i].insert(iter2->ent_id);
			}
		}
	}
}

void Dataset::add_entity_interaction(ushort const& type, int64_t const& from_ent_id,
		Interact const& interact) {
	/// check the existence of interaction
	if (m_id_id_map[from_ent_id].find(interact.ent_id)
			!= m_id_id_map[from_ent_id].end()) {
		m_id_id_map[from_ent_id].insert(interact.ent_id);
		/// add the entity interaction
		m_ent_type_interacts[from_ent_id][type].push_back(interact);
	}
}

}
