/*
 * Dataset.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: manazhao
 */

#include "Dataset.h"

namespace recsys {

Dataset::Dataset(int64_t const& maxId) :
	m_ent_type_interacts(maxId) {
}

void Dataset::add_entity(int8_t const& type, int64_t const& id) {
	m_type_ent_ids[type].insert(id);
	m_ent_ids.insert(id);
}

void Dataset::filter_interaction(
		vector<map<int8_t, vector<Interact> > > const& entTypeInteractions) {
	/// only keep those interactions the both entities of which are in the entity id set
	for (set<int64_t>::iterator iter = m_ent_ids.begin(); iter
			!= m_ent_ids.end(); ++iter) {
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

void Dataset::_filter_entity_interaction_helper(int8_t const& type,
		int64_t const& from_ent_id, Interact const& interact) {
	/// check the existence of interaction
	if (entity_exist(from_ent_id) && entity_exist(interact.ent_id)) {
		m_ent_type_interacts[from_ent_id][type].push_back(interact);
	}
}

}
