/*
 * MemoryContainer.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef MEMORYCONTAINER_HPP_
#define MEMORYCONTAINER_HPP_
#include "Basic.hpp"

namespace recsys{
namespace graph{

class MapNameIdContainer {
public:
	Entity::ent_idx_type index(string const& name);
	bool exist(Entity::ent_idx_type const& idx) const;
	bool exist(string const& name) const;
	string const& idx_to_name(Entity::ent_idx_type const& id) const;
	Entity::ent_idx_type  next_idx() const;
protected:
	map<string, Entity::ent_idx_type> m_name_id_map;
	map<Entity::ent_idx_type, string> m_id_name_map;
};

class MapEntityContainer {
public:
	template<typename EntityValueType>
	shared_ent_ptr index(Entity::ent_idx_type const& idx,
			Entity::ent_type const& type, EntityValueType const& value =
					EntityValueType());
	shared_ent_ptr exist(Entity::ent_idx_type const& idx) const;
	void get_type_idx_set(Entity::ent_type const& type,
			ent_idx_set& idxSet) const;
	type_idx_set_map const& get_idx_set_map() const;
protected:
	map<Entity::ent_idx_type, shared_ent_ptr> m_entity_map;
	map<Entity::ent_type, ent_idx_set> m_type_idx_set_map;
};

/// TODO: implementation through SQL database

/// define interface for EntityRelationManager
///
class MapEntityRelationContainer {
public:
	/// add link between two entities
	bool link_entity(shared_ent_ptr const& ent1, shared_ent_ptr const& ent2);
	void get_adjacent_list(Entity::ent_idx_type const& entityIdx,
			Entity::ent_type const& type, adj_id_list& idList) const;
	void get_adjacent_list(Entity::ent_idx_type const& entityIdx, adj_id_list& idList) const;

protected:
	ent_type_adj_set_map m_type_adj_map;
};

}
}


#endif /* MEMORYCONTAINER_HPP_ */
