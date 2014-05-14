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
typedef boost::shared_ptr<Entity> shared_ent_ptr;

/// functor that composes key based on name and type
class DefaultComposeKey {
public:
	string operator()(Entity::ent_type const& type, string const& name) const;
	string operator()(Entity const& ent1, Entity const& ent2) const;
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
typedef vector<const_ent_ptr> adj_const_ptr_list;
typedef vector<shared_ent_ptr> adj_shared_ptr_list;
typedef map<Entity::ent_type, ent_idx_set> type_adj_set_map;
typedef map<Entity::ent_idx_type, type_adj_set_map> ent_type_adj_set_map;
/// implementation of EntityManager through STL map container
typedef map<Entity::ent_idx_type, shared_ent_ptr> shared_entity_map;
typedef map<Entity::ent_type, ent_idx_set> type_idx_set_map;


/// define some testing functions
void test();

}
}


#endif /* INTERFACE_HPP_ */
