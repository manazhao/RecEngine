#include "boost/lexical_cast.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace recsys {
namespace graph {

ostream& operator<<(ostream& oss, NullValue const& val) {
	return oss;
}

//// member functions for Entity class

inline Entity::Entity(ent_type const& type, ent_idx_type const& idx) :
		m_type(type), m_idx(idx) {
}

inline Entity::ent_idx_type const& Entity::get_idx() const {
	return m_idx;
}

inline Entity::ent_type const& Entity::get_type() const {
	return m_type;
}

/// members for Entity<T>
template<typename DataTypeT>
Entity_T<DataTypeT>::Entity_T(Entity::ent_type const& type,
		Entity::ent_idx_type const& idx, ent_value_type const& value) :
		Entity(type, idx), m_value(value) {
}

template<typename DataTypeT>
inline typename Entity_T<DataTypeT>::ent_value_type const& Entity_T<DataTypeT>::get_value() const {
	return m_value;
}

template<typename DataValueT>
ostream& operator <<(ostream& oss, Entity_T<DataValueT> const& ent) {
	oss << ent.get_idx() << "," << ent.get_type() << "," << ent.get_value();
	return oss;
}

/// members for DefaultComposeKey

string DefaultComposeKey::operator()(Entity::ent_type const& type,
		string const& name) const {
	// simply paste type and name by _
	int typeInt = static_cast<int>(type);
	stringstream ss;
	ss << type << "_" << lexical_cast<string, int>(typeInt) << name;
	return ss.str();
}

/// define operator() for DefaultDecomposeKey
void DefaultDecomposeKey::operator()(string const& cKey,
		Entity::ent_type& oType, string& oName) const {
	vector < string > splits;
	boost::split(splits, cKey, boost::is_any_of("_"));
	oName = splits[1];
	int type = boost::lexical_cast<int>(splits[0]);
	oType = static_cast<Entity::ent_type>(type);
}

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

string const& MapNameIdContainer::idx_to_name(
		Entity::ent_idx_type const& idx) const {
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
	shared_ent_ptr entPtr(new Entity_T<EntityValueType>(type, idx, value));
	m_entity_map[idx] = entPtr;
	m_type_idx_set_map[type].insert(idx);
	return entPtr;
}

shared_ent_ptr MapEntityContainer::exist(
		Entity::ent_idx_type const& idx) const {
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
		for (type_adj_set_map::const_iterator iter1 = typeMap.begin();
				iter1 != typeMap.end(); ++iter1) {
			/// put the result into the idList
			ent_idx_set const& idSet = iter1->second;
			idList.insert(idList.end(), idSet.begin(), idSet.end());
		}
	}
}

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
inline Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T>::Graph(
		_NIC_T const& nameIdContainer, _EC_T const& entityContainer,
		_ERC_T const& entityRelationContainer) :
		m_nameId_container(nameIdContainer), m_entity_container(
				entityContainer), m_entity_relation_container(
				entityRelationContainer) {

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
	for (type_idx_set_map::const_iterator iter = typeIdxSet.begin();
			iter != typeIdxSet.end(); ++iter) {
		if (!(iter->second.empty())) {
			allIdVec.insert(allIdVec.end(), iter->second.begin(),
					iter->second.end());
			Entity::ent_type type = iter->first;
			oss << type << ":";
			for (ent_idx_set::const_iterator iter1 = iter->second.begin();
					iter1 != iter->second.end(); ++iter1) {
				/// retrieve the entity
				shared_ent_ptr entPtr = graph.entity_exist(*iter1);
				switch (type) {
				case Entity::ENT_USER:
				case Entity::ENT_ITEM: {
					Entity& entRef1 = *entPtr;
					Entity_T<NullValue> const& entRef = dynamic_cast<Entity_T<
							NullValue> const&>(entRef1);
					oss << "[" << entRef << "], ";
				}
					break;
				case Entity::ENT_RATING: {
					Entity_T<char>& entRef =
							dynamic_cast<Entity_T<char>&>(*entPtr);
					oss << "[" << entRef << "], ";
				}
					break;
				default:
					break;
				}
			}
			oss << endl;
		}
	}
	/// dump entity link information
	oss << ">>> entity links" << endl;
	for (adj_id_list::const_iterator iter = allIdVec.begin();
			iter < allIdVec.end(); ++iter) {
		Entity::ent_idx_type entId = *iter;
		adj_id_list entAdjList;
		graph.adjacent_list(entId, entAdjList);
		/// dump the adjacent entities
		oss << entId << ":";
		for (adj_id_list::const_iterator iter1 = entAdjList.begin();
				iter1 < entAdjList.end(); ++iter1) {
			oss << *iter1 << ",";
		}
		oss << endl;
	}
	return oss;
}

}
}
