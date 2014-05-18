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
#include <iostream>
using namespace std;

namespace recsys{
namespace graph{


/// declare EntityGraph class which puts all components togeter
template<typename _T1 = MemoryNameIdContainer,
		typename _T2 = MemoryEntityContainer,
		typename _T3 = MemoryERContainer>
class Graph {
public:
	template<typename t1, typename t2, typename t3>
	friend ostream& operator<<(ostream&, Graph<t1,t2,t3> const&);
public:
	typedef _T1 name_id_container_type;
	typedef _T2 entity_container_type;
	typedef _T3 entity_relation_container_type;
public:

	template<ENTITY_TYPE t>
	entity_idx_type index_entity(string const& name,  typename EntityValueTrait<t>::value_type const& value =
			typename EntityValueTrait<t>::value_type()){
		/// check the existence of the entity
		entity_idx_type idx = _cast_to_base_nameid().template add<t>(name);
		/// add the value
		_cast_to_base_entity().template add< t >(idx,value);
		return idx;
	}

	template<ENTITY_TYPE type>
	bool entity_exist(string const& name) const{
		return _cast_to_base_nameid().exist<type>(name);
	}

	bool entity_exist(entity_idx_type const& idx) const{
		return _cast_to_base_nameid().exist(idx);
	}

	template<ENTITY_TYPE t>
	void retrieve_entity(entity_list& entityList) const{
		_cast_to_base_nameid().template retrieve<t>(entityList);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2>
	void link_entity(entity_idx_type const& idx1, entity_idx_type const& idx2){
		_cast_to_base_er().template link_entity<T1,T2>(idx1,idx2);
		/// reverse edge
		_cast_to_base_er().template link_entity<T2,T1>(idx2,idx1);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2>
	void get_adj_list(entity_idx_type const& idx, typename AdjListType<T1,T2>::type& idList) const{
		_cast_to_base_er().get_adj_list<T1,T2>(idx,idList);
	}

protected:
	NameIdContainer<name_id_container_type>& _cast_to_base_nameid(){
		return static_cast<NameIdContainer<name_id_container_type>& >(m_nameId_container);
	}

	NameIdContainer<name_id_container_type>const & _cast_to_base_nameid() const{
		return static_cast<NameIdContainer<name_id_container_type>const& >(m_nameId_container);
	}

	EntityContainer<entity_container_type>& _cast_to_base_entity(){
		return static_cast<EntityContainer<entity_container_type>& >(m_entity_container);
	}

	EntityContainer<entity_container_type> const& _cast_to_base_entity() const{
		return static_cast<EntityContainer<entity_container_type>const& >(m_entity_container);
	}

	ERContainer<entity_relation_container_type>& _cast_to_base_er(){
		return static_cast<ERContainer<entity_relation_container_type>& >(m_entity_relation_container);
	}

	ERContainer<entity_relation_container_type>const& _cast_to_base_er() const{
		return static_cast<ERContainer<entity_relation_container_type> const& >(m_entity_relation_container);
	}
protected:
	name_id_container_type m_nameId_container;
	entity_container_type m_entity_container;
	entity_relation_container_type m_entity_relation_container;
};

typedef Graph<> memory_graph;

template<typename _T1,
		typename _T2,
		typename _T3>
ostream& operator<<(ostream& oss, Graph<_T1,_T2,_T3> const& graph);

ostream& operator<<(ostream& oss, entity_list const& entityList);


}
}

#include "./GraphImpl.inc"

#endif /* GRAPH_HPP_ */
