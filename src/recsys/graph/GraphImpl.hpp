/*
 * GraphImpl.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef GRAPHIMPL_HPP_
#define GRAPHIMPL_HPP_
#include "Graph.hpp"

namespace recsys{
namespace graph{

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
ostream& operator<< (ostream& oss, Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T> const& graph);

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
inline Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::Graph(
		_NIC_T const& nameIdContainer, _EC_T const& entityContainer,
		_ERC_T const& entityRelationContainer) :
	m_nameId_container(nameIdContainer), m_entity_container(entityContainer),
			m_entity_relation_container(entityRelationContainer) {

}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
template<typename _EV_T>
shared_ent_ptr Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::index_entity(
		Entity::ent_type const& type, string const& name, _EV_T const& value) {
	/// first index the entity
	_KC_T kcFunc;
	string cKey = kcFunc(type, name);
	Entity::ent_idx_type entIdx = m_nameId_container.index(cKey);
	/// index the Entity
	shared_ent_ptr entPtr = m_entity_container.index(entIdx, type, value);
	return entPtr;
}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
shared_ent_ptr Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::entity_exist(
		Entity::ent_idx_type const& type, string const& name) const {
	shared_ent_ptr resultPtr;
	/// get the comosite key
	_KC_T kcFunc;
	string cKey = kcFunc(type, name);
	bool keyExist = m_nameId_container.exist(cKey);
	if (keyExist) {
		Entity::ent_idx_type entIdx = m_nameId_container.index(cKey);
		resultPtr = m_entity_container.exist(entIdx);
	}
	return resultPtr;
}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
shared_ent_ptr Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::entity_exist(
		Entity::ent_idx_type const& idx) const {
	shared_ent_ptr resultPtr;
	bool keyExist = m_nameId_container.exist(idx);
	if (keyExist) {
		resultPtr = m_entity_container.exist(idx);
	}
	return resultPtr;
}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
void Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::link_entity(
		shared_ent_ptr const& ent1, shared_ent_ptr const& ent2) {
	/// simply call EntityRelationContainer
	m_entity_relation_container.link_entity(ent1, ent2);
}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
void Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::adjacent_list(
		Entity::ent_idx_type const& idx, Entity::ent_type const& type,
		adj_id_list& idList) const {
	m_entity_relation_container.get_adjacent_list(idx, type, idList);

}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
void Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::adjacent_list(
		Entity::ent_idx_type const& idx, adj_id_list& idList) const {
	m_entity_relation_container.get_adjacent_list(idx, idList);

}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
type_idx_set_map const& Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::get_idx_set_map() const {
	return m_entity_container.get_idx_set_map();
}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
ostream& operator<<(ostream& oss,
		Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T> const& graph) {
	//// first dump all entities
	type_idx_set_map const& typeIdxSet = graph.get_idx_set_map();
	adj_id_list allIdVec;
	oss << ">>> entity information\n";
	/// simply dump the entity ids of each type
	for (type_idx_set_map::const_iterator iter = typeIdxSet.begin(); iter
			!= typeIdxSet.end(); ++iter) {
		if (!(iter->second.empty())) {
			allIdVec.insert(allIdVec.end(), iter->second.begin(),
					iter->second.end());
			Entity::ent_type type = iter->first;
			oss << type << ":";
			for (ent_idx_set::const_iterator iter1 = iter->second.begin(); iter1
					!= iter->second.end(); ++iter1) {
				/// retrieve the entity
				shared_ent_ptr entPtr = graph.entity_exist(*iter1);
				switch (type) {
				case Entity::ENT_USER:
				case Entity::ENT_ITEM: {
					Entity_T<NullValue> const& entRef = dynamic_cast<Entity_T<
							NullValue> const&> (*entPtr);
					oss << (iter1
							== iter->second.begin() ? "" : ",") << "[" << entRef << "]" ;
					break;
				}
				case Entity::ENT_FEATURE:
				case Entity::ENT_RATING: {
					Entity_T<char>& entRef =
							dynamic_cast<Entity_T<char>&> (*entPtr);
					oss  << (iter1
							== iter->second.begin() ? "" : ",") << "[" << entRef << "]";
					break;
				}
				default:
					break;
				}
			}
			oss << endl;
		}
	}
	/// dump entity link information
	oss << ">>> entity links" << endl;
	for (adj_id_list::const_iterator iter = allIdVec.begin(); iter
			< allIdVec.end(); ++iter) {
		Entity::ent_idx_type entId = *iter;
		adj_id_list entAdjList;
		graph.adjacent_list(entId, entAdjList);
		/// dump the adjacent entities
		oss << entId << ":";
		for (adj_id_list::const_iterator iter1 = entAdjList.begin(); iter1
				< entAdjList.end(); ++iter1) {
			oss << (iter1 == entAdjList.begin() ? "" : ",") << *iter1;
		}
		oss << endl;
	}
	return oss;
}
}
}

#endif /* GRAPHIMPL_HPP_ */
