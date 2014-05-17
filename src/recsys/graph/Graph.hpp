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
template<typename NameIdContainerType = MemoryNameIdContainer,
		typename EntityContainerType = MemoryEntityContainer,
		typename EntityRelationContainerType = MemoryERContainer,
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

	template<ENTITY_TYPE type>
	Entity<typename EntityValueTrait<type>::value_type> const& index_entity(string const& name,  typename EntityValueTrait<type>::value_type const& value =
			typename EntityValueTrait<type>::value_type()){
		/// check the existence of the entity
		entity_idx_type const& idx = m_nameId_container.add<type,key_composer_type>(name);
		/// add the value
		return m_entity_container.add<type>(idx,value);
	}

	template<ENTITY_TYPE type>
	bool entity_exist(string const& name) const{
		return m_nameId_container.exist<type>(name);
	}

	bool entity_exist(entity_idx_type const& idx) const{
		return m_nameId_container.exist(idx);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2>
	void link_entity(entity_idx_type const& idx1, entity_idx_type const& idx2){
		m_entity_relation_container.link_entity<T1,T2>(idx1,idx2);
		/// reverse edge
		m_entity_relation_container.link_entity<T2,T1>(idx2,idx1);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2, typename AdjListType = adj_id_list>
	void get_adj_list(entity_idx_type const& idx, AdjListType& idList) const{
		return m_entity_relation_container.get_adj_list<T1,T2>(idx,idList);
	}

protected:
	name_id_container_type m_nameId_container;
	entity_container_type m_entity_container;
	entity_relation_container_type m_entity_relation_container;
};

typedef Graph<> memory_graph;

}
}

#endif /* GRAPH_HPP_ */
