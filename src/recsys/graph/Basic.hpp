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

/**
 * entity type definition
 */
enum ENTITY_TYPE {
	ENT_DEFAULT, ENT_USER, ENT_ITEM, ENT_FEATURE,ENT_RATING,ENT_RATING_FEATURE,
};

/**
 * empty data value
 */
class NullValue{
};
ostream& operator<<(ostream& oss, NullValue const& val);

/**
 * Entity data type traits based on entity type
 */
template<ENTITY_TYPE type = ENT_DEFAULT>
class EntityValueTrait{
	typedef NullValue value_type;
};

/**
 * template specialization based on specific entity type
 */
template<>
struct EntityValueTrait<ENT_USER>{
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_ITEM>{
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_FEATURE>{
	typedef NullValue value_type;
};

template<>
struct EntityValueTrait<ENT_RATING_FEATURE>{
	typedef NullValue value_type;
};


template<>
struct EntityValueTrait<ENT_RATING>{
	typedef unsigned char value_type;
};



///// more specialized templates can be added /////
///// end of template specialization /////

/**
 * The value type can be derived by using the EntityValueTrait
 */
template <typename ValueType>
class Entity{
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
	string operator()(ENTITY_TYPE const& type, string const& name) const;
	template<typename T1, typename T2>
	string operator()(T1 const& t1, T2 const& t2) const;
};

/// functor that decompose a composite key into type and original name
class DefaultDecomposeKey {
public:
	void operator()(string const& cKey, ENTITY_TYPE& oType,
			string& oName) const;

};

/**
 * define interface for entity name <-> internal id container
 * use CRTP
 */
//// internal entity id type
typedef std::size_t entity_idx_type;
typedef vector<entity_idx_type> entity_id_vec;
template <typename DerivedType>
class NameIdContainer{
public:
	typedef DerivedType derived_type;
public:
	template <ENTITY_TYPE type,typename KeyComposer>
	entity_idx_type  add(string const& name, KeyComposer const& keyComposer = KeyComposer()){
		_cast_to_derive().add<type,keyComposer>(name);
	}

	template<ENTITY_TYPE type,typename KeyComposer>
	entity_idx_type retrieve(string const& name, KeyComposer const& keyComposer = KeyComposer()) const{
		//// generate the composite name
		string compositeKey = keyComposer(type,name);
		return _cast_to_derive().retrieve(compositeKey);
	}

	entity_idx_type retrieve(string const& compName) const{
		return _cast_to_derive().retrieve(compName);
	}

	string const& retrieve(entity_idx_type const& idx) const{
		return _cast_to_derive().retrieve(idx);
	}

	template<ENTITY_TYPE type, typename KeyComposer>
	bool exist(string const& name, KeyComposer const& keyComposer = KeyComposer()) const{
		//// generate the composite name
		string compositeKey = keyComposer(type,name);
		return _cast_to_derive().exist(compositeKey);
	}

	bool exist(string const& compName) const{
		return _cast_to_derive().exist(compName);
	}

	bool exist(entity_idx_type const& idx) const{
		return _cast_to_derive().exist(idx);
	}

	template<ENTITY_TYPE type>
	void retrieve(entity_id_vec& entityVec) const{
		return _cast_to_derive().retrieve<type>(entityVec);
	}

protected:
	derived_type& _cast_to_derive(){
		return static_cast<derived_type&>(*this);
	}
};

/**
 * define base class for entity management
 */
template <typename DerivedType>
class EntityContainer{
public:
	typedef DerivedType derived_type;
public:
	template<ENTITY_TYPE type>
	Entity<typename EntityValueTrait<type>::value_type > const& add(entity_idx_type const& idx, typename EntityValueTrait<type>::value_type const& value = EntityValueTrait<type>::value_type() ){
		return _cast_to_derive().add(idx,value);
	}

	template<ENTITY_TYPE type>
	typename EntityValueTrait<type>::value_type const& retrieve(entity_idx_type const& idx) const{
		return _cast_to_derive().exist(idx);
	}

	template<ENTITY_TYPE type>
	bool exist(entity_idx_type const& idx) const{
		return _cast_to_derive().exist(idx);
	}

protected:
	derived_type& _cast_to_derive(){
		return static_cast<derived_type&>(*this);
	}
};


/**
 * define base class for entity relation management
 *
 */
typedef vector<entity_idx_type> adj_id_list;

template <typename DerivedType>
class ERContainer{
public:
	typedef DerivedType derived_type;
public:
	template <ENTITY_TYPE T1, ENTITY_TYPE T2>
	bool link_entity(entity_idx_type const& idx1, entity_idx_type& idx2){
		_cast_to_derive().link_entity<T1,T2>(idx1,idx2);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2, typename AdjListType = adj_id_list>
	void get_adj_list(entity_idx_type const& idx, AdjListType& idList) const{
		_cast_to_derive().get_adj_list<T1,T2>(idx,idList);
	}

protected:
	 derived_type& _cast_to_derive(){
		return static_cast<derived_type&>(*this);
	}
};


/// define some testing functions
void test();

}
}

#include "./BasicImpl.inc"

#endif /* INTERFACE_HPP_ */
