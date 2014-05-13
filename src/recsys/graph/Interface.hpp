/*
 * Elements.hpp
 *
 *  Created on: May 12, 2014
 *      Author: qzhao2
 */

#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_
#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace boost;

namespace recsys {
namespace graph {

/// forward declaration
class EntityContainer;

class NullValue{
};

ostream& operator<<(ostream& oss, NullValue const& val);

class Entity {
	friend class MapEntityContainer;
	friend class MapEntityRelationContainer;
public:
	typedef std::size_t ent_idx_type;
	enum ENTITY_TYPE {
		ENT_DEFAULT, ENT_USER, ENT_ITEM, ENT_FEATURE,ENT_RATING,ENT_RATING_FEATURE
	};
	typedef ENTITY_TYPE ent_type;
protected:
	ent_type m_type;
	ent_idx_type m_idx;
protected:
	/// only the EntityManager is allowed to create the Entity
	Entity(ent_type const& type, ent_idx_type const& idx);
public:
	ent_idx_type const& get_idx() const;
	ent_type const& get_type() const;
	virtual ~Entity(){}
};

typedef set<Entity::ent_idx_type> ent_idx_set;

template<typename DataTypeT>
class Entity_T: public Entity {
	friend class MapEntityContainer;
public:
	typedef DataTypeT ent_value_type;
protected:
	Entity_T(Entity::ent_type const& type, Entity::ent_idx_type const& idx,
			ent_value_type const& value = ent_value_type());
protected:
	ent_value_type m_value;
public:
	ent_value_type const& get_value() const;
};

template<typename DataValueT>
ostream& operator << (ostream& oss, Entity_T<DataValueT> const& ent);


/// adjacent entity list
typedef vector<Entity::ent_idx_type> adj_id_list;
typedef const Entity* const_ent_ptr;
typedef shared_ptr<Entity> shared_ent_ptr;

/// functor that composes key based on name and type
class DefaultComposeKey {
public:
	string operator()(Entity::ent_type const& type, string const& name) const;
};

/// functor that decompose a composite key into type and original name
class DefaultDecomposeKey {
public:
	void operator()(string const& cKey, Entity::ent_type& oType,
			string& oName) const;
};

/// /define interface for entity name <-> id mapper
typedef map<string, Entity::ent_idx_type> name_idx_map;
typedef map<Entity::ent_idx_type, string> idx_name_map;
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

/// implementation of EntityManager through STL map container
typedef map<Entity::ent_idx_type, shared_ent_ptr> shared_entity_map;
typedef map<Entity::ent_type, ent_idx_set> type_idx_set_map;
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
typedef vector<const_ent_ptr> adj_const_ptr_list;
typedef vector<shared_ent_ptr> adj_shared_ptr_list;
typedef map<Entity::ent_type, ent_idx_set> type_adj_set_map;
typedef map<Entity::ent_idx_type, type_adj_set_map> ent_type_adj_set_map;

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

template<typename _NIC_T, typename _EC_T, typename _ERC_T, typename _KC_T,
		typename _KD_T>
ostream& operator<< (ostream& oss, Graph<_NIC_T, _EC_T, _ERC_T, _KC_T, _KD_T> const& graph);

typedef Graph<> memory_graph;
/// define some testing functions
void test();

}
}

#include "Implementation.inl"

#endif /* INTERFACE_HPP_ */
