/*
 * MemoryContainerImpl.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef MEMORYCONTAINERIMPL_HPP_
#define MEMORYCONTAINERIMPL_HPP_
#include "MemoryContainer.hpp"

namespace recsys{
namespace graph{


/// members for MapNameIdContainer
Entity::ent_idx_type MapNameIdContainer::index(string const& name) {
	if (m_name_id_map.find(name) != m_name_id_map.end()) {
		return m_name_id_map[name];
	}
	Entity::ent_idx_type idx = next_idx();
	m_name_id_map[name] = idx;
	m_id_name_map[idx] = name;
	return idx;
}


string const& MapNameIdContainer::idx_to_name(Entity::ent_idx_type const& idx) const {
	static string nullKey = "";
	idx_name_map::const_iterator iter = m_id_name_map.find(idx);
	if (iter != m_id_name_map.end())
		return iter->second;
	return nullKey;
}

inline Entity::ent_idx_type MapNameIdContainer::next_idx() const {
	return m_name_id_map.size();
}

bool MapNameIdContainer::exist(Entity::ent_idx_type const& idx) const {
	return m_id_name_map.find(idx) != m_id_name_map.end();
}

bool MapNameIdContainer::exist(string const& name) const {
	return m_name_id_map.find(name) != m_name_id_map.end();
}

/// members for MapEntityManager
template<typename EntityValueType>
shared_ent_ptr MapEntityContainer::index(Entity::ent_idx_type const& idx,
		Entity::ent_type const& type, EntityValueType const& value) {
	shared_ent_ptr entPtr(new Entity_T<EntityValueType> (type, idx, value));
	m_entity_map[idx] = entPtr;
	m_type_idx_set_map[type].insert(idx);
	return entPtr;
}

shared_ent_ptr MapEntityContainer::exist(Entity::ent_idx_type const& idx) const {
	shared_ent_ptr resultPtr;
	shared_entity_map::const_iterator iter = m_entity_map.find(idx);
	if (iter != m_entity_map.end())
		resultPtr = iter->second;
	return resultPtr;
}

void MapEntityContainer::get_type_idx_set(Entity::ent_type const& type,
		ent_idx_set& idxSet) const {
	type_idx_set_map::const_iterator iter = m_type_idx_set_map.find(type);
	if (iter != m_type_idx_set_map.end())
		idxSet = iter->second;
}

type_idx_set_map const& MapEntityContainer::get_idx_set_map() const {
	return m_type_idx_set_map;
}

//// members for MapEntityRelationContainer

/// link two existing entities
bool MapEntityRelationContainer::link_entity(shared_ent_ptr const& ent1,
		shared_ent_ptr const& ent2) {
	Entity::ent_idx_type idx1 = ent1->m_idx;
	Entity::ent_idx_type idx2 = ent2->m_idx;
	/// bidirectional graph
	m_type_adj_map[idx1][ent2->m_type].insert(ent2->m_idx);
	m_type_adj_map[idx2][ent1->m_type].insert(ent1->m_idx);
	return true;
}

void MapEntityRelationContainer::get_adjacent_list(
		Entity::ent_idx_type const& entityIdx, Entity::ent_type const& type,
		adj_id_list& idList) const {
	ent_type_adj_set_map::const_iterator iter1 = m_type_adj_map.find(entityIdx);
	if (iter1 != m_type_adj_map.end()) {
		type_adj_set_map::const_iterator iter2 = iter1->second.find(type);
		if (iter2 != iter1->second.end())
			/// insert the result directly
			idList.insert(idList.end(), iter2->second.begin(),
					iter2->second.end());
	}
}

void MapEntityRelationContainer::get_adjacent_list(
		Entity::ent_idx_type const& entityIdx, adj_id_list& idList) const {
	ent_type_adj_set_map::const_iterator iter = m_type_adj_map.find(entityIdx);
	if (iter != m_type_adj_map.end()) {
		type_adj_set_map const& typeMap = iter->second;
		for (type_adj_set_map::const_iterator iter1 = typeMap.begin(); iter1
				!= typeMap.end(); ++iter1) {
			/// put the result into the idList
			ent_idx_set const& idSet = iter1->second;
			idList.insert(idList.end(), idSet.begin(), idSet.end());
		}
	}
}



}
}

#endif /* MEMORYCONTAINERIMPL_HPP_ */
