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
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace boost;

namespace recsys {
namespace graph {

/**
 * entity type definition
 */
enum ENTITY_TYPE {
	ENT_DEFAULT,
	ENT_USER,
	ENT_ITEM,
	ENT_FEATURE,
	ENT_RATING,
};

template<ENTITY_TYPE t>
class Enum2Type {
	enum {
		val = t
	};
};

/**
 * empty data value
 */
class NullValue {
};
//ostream& operator<<(ostream& oss, NullValue const& val);

/**
 * Entity data type traits based on entity type
 */
template<ENTITY_TYPE type = ENT_DEFAULT>
class EntityValueTrait {
	typedef NullValue value_type;
};

/**
 * template specialization based on specific entity type
 */
template<>
struct EntityValueTrait<ENT_USER> {
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_ITEM> {
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_FEATURE> {
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_RATING_FEATURE> {
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_RATING> {
	typedef unsigned char value_type;
};

///// more specialized templates can be added /////
///// end of template specialization /////

/**
 * The value type can be derived by using the EntityValueTrait
 */
template<typename ValueType>
class Entity {
public:
	typedef ValueType value_type;
public:
	Entity(value_type const& value);
protected:
	ValueType m_value;
};

/// functor that composes key based on name and type
class DefaultComposeKey {
public:
	string operator()(ENTITY_TYPE const& type, string const& name) const {
		// simply paste type and name by _
		stringstream ss;
		ss << type << "_" << name;
		return ss.str();
	}
	template<typename T1, typename T2>
	string operator()(T1 const& t1, T2 const& t2) const{
		// simply paste type and name by _
		stringstream ss;
		ss << t1 << "_" << t2;
		return ss.str();
	}
};

/// functor that decompose a composite key into type and original name
class DefaultDecomposeKey {
public:
	void
	operator()(string const& cKey, ENTITY_TYPE& oType, string& oName) const{
		vector < string > splits;
		boost::split(splits, cKey, boost::is_any_of("_"));
		stringstream ss;
		for (size_t i = 1; i < splits.size(); i++) {
			ss << (i == 1 ? "" : "_") << splits[i];
		}
		oName = ss.str();
		int type = boost::lexical_cast<int>(splits[0]);
		oType = static_cast<ENTITY_TYPE> (type);
	}

};

/**
 * define interface for entity name <-> internal id container
 * use CRTP
 */
//// internal entity id type
typedef std::size_t entity_idx_type;

enum {INVALID_ENTITY_ID = 0};

typedef vector<entity_idx_type> entity_id_vec;
template<typename DerivedType>
class NameIdContainer {
public:
	typedef DerivedType derived_type;
public:

	template<ENTITY_TYPE t>
	entity_idx_type add(string const& name) {
		return _cast_to_derive().add<t> (name);
	}

	template<ENTITY_TYPE t>
	entity_idx_type retrieve(string const& name) const {
		//// generate the composite name
		string compositeKey = DefaultComposeKey(t, name);
		return _cast_to_derive().retrieve(compositeKey);
	}

	entity_idx_type retrieve(string const& compName) const {
		return _cast_to_derive().retrieve(compName);
	}

	string const& retrieve(entity_idx_type const& idx) const {
		return _cast_to_derive().retrieve(idx);
	}

	template<ENTITY_TYPE t>
	bool exist(string const& name) const {
		//// generate the composite name
		string compositeKey = DefaultComposeKey(t, name);
		return _cast_to_derive().exist(compositeKey);
	}

	bool exist(string const& compName) const {
		return _cast_to_derive().exist(compName);
	}

	bool exist(entity_idx_type const& idx) const {
		return _cast_to_derive().exist(idx);
	}

	template<ENTITY_TYPE t>
	void retrieve(entity_id_vec& entityVec) const {
		return _cast_to_derive().retrieve<t> (entityVec);
	}

protected:
	derived_type& _cast_to_derive() {
		return static_cast<derived_type&> (*this);
	}
	derived_type const& _cast_to_derive()const {
		return static_cast<derived_type const&> (*this);
	}
};

/**
 * define base class for entity management
 */
template<typename DerivedType>
class EntityContainer {
public:
	typedef DerivedType derived_type;
public:
	template<ENTITY_TYPE type>
	void add(
			entity_idx_type const& idx,
			typename EntityValueTrait<type>::value_type const& value =
					typename EntityValueTrait<type>::value_type()) {
		 _cast_to_derive().template add<type>(idx, value);
	}

	template<ENTITY_TYPE type>
	typename EntityValueTrait<type>::value_type const& retrieve(
			entity_idx_type const& idx) const {
		return _cast_to_derive().retrieve<type> (idx);
	}

	template<ENTITY_TYPE t>
	bool exist(entity_idx_type const& idx) const {
		return _cast_to_derive().exist<t> (idx);
	}

protected:
	derived_type& _cast_to_derive() {
		return static_cast<derived_type&> (*this);
	}

	derived_type const& _cast_to_derive() const{
		return static_cast<derived_type const&> (*this);
	}

};

/**
 * define base class for entity relation management
 *
 */
typedef set<entity_idx_type> entity_set;
typedef vector<entity_idx_type> entity_list;

template<ENTITY_TYPE t1, ENTITY_TYPE t2>
struct AdjListType{
	typedef entity_list type;
};

template<>
struct AdjListType<ENT_RATING,ENT_USER>{
	typedef entity_idx_type type;
};

template<>
struct AdjListType<ENT_RATING,ENT_ITEM>{
	typedef entity_idx_type type;
};

template<typename DerivedType>
class ERContainer {
public:
	typedef DerivedType derived_type;
public:
	template<ENTITY_TYPE t1, ENTITY_TYPE t2>
	void link_entity(entity_idx_type const& idx1, entity_idx_type const& idx2) {
		return _cast_to_derive().template link_entity<t1, t2> (idx1, idx2);
	}

	template<ENTITY_TYPE t1, ENTITY_TYPE t2>
	void get_adj_list(entity_idx_type const& idx, typename AdjListType<t1,t2>::type& idList) const {
		_cast_to_derive().template get_adj_list<t1, t2> (idx, idList);
	}

protected:
	derived_type& _cast_to_derive() {
		return static_cast<derived_type&> (*this);
	}

	derived_type const& _cast_to_derive() const {
		return static_cast<derived_type const&> (*this);
	}

};

/// define some testing functions
void test();

}
}

#endif /* INTERFACE_HPP_ */
