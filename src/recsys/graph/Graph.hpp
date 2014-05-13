/*
 * Graph.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef GRAPH_HPP_
#define GRAPH_HPP_
#include "Basic.hpp"
#include "MemoryContainer.hpp"

namespace recsys{
namespace graph{

/// declare EntityGraph class which puts all components togeter
template<typename NameIdContainerType = MapNameIdContainer,
		typename EntityContainerType = MapEntityContainer,
		typename EntityRelationContainerType = MapEntityRelationContainer,
		typename KeyComposerType = DefaultComposeKey,
		typename KeyDecomposerType = DefaultDecomposeKey>
class Graph {
public:
	typedef NameIdContainerType name_id_container_type;
	typedef EntityContainerType entity_container_type;
	typedef EntityRelationContainerType entity_relation_container_type;
	typedef KeyComposerType key_composer_type;
	typedef KeyDecomposerType key_decomposer_type;
public:
	Graph(NameIdContainerType const& nameIdContainer = NameIdContainerType(),
			EntityContainerType const& entityContainer = EntityContainerType(),
			EntityRelationContainerType const& entityRelationContainer = EntityRelationContainerType());
	template<typename EntityValueType>
	shared_ent_ptr index_entity(Entity::ent_type const& type,
			string const& name, EntityValueType const& value =
					EntityValueType());
	shared_ent_ptr entity_exist(Entity::ent_idx_type const& type,
			string const& name) const;
	shared_ent_ptr entity_exist(Entity::ent_idx_type const& idx) const;

	/// link two entities
	void link_entity(shared_ent_ptr const& ent1, shared_ent_ptr const& ent2);
	void adjacent_list(Entity::ent_idx_type const& idx, adj_id_list& idList) const;
	void adjacent_list(Entity::ent_idx_type const& idx,
			Entity::ent_type const& type, adj_id_list& idList) const;

	type_idx_set_map const& get_idx_set_map() const;
protected:
	name_id_container_type m_nameId_container;
	entity_container_type m_entity_container;
	entity_relation_container_type m_entity_relation_container;
};

typedef Graph<> memory_graph;

}
}

#endif /* GRAPH_HPP_ */
