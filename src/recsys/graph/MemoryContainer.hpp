/*
 * MemoryContainer.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef MEMORYCONTAINER_HPP_
#define MEMORYCONTAINER_HPP_
#include "Basic.hpp"
#include <map>
#include <set>

using namespace std;

namespace recsys{
namespace graph{

typedef set<entity_idx_type> entity_set_type;

class MemoryNameIdContainer : public NameIdContainer<MemoryNameIdContainer>{
protected:
	entity_idx_type _next_entity_idx() const{
		return m_name_id_map.size() + 1;
	}
public:
	template <ENTITY_TYPE t>
	entity_idx_type add(string const& name){
		/// generate the composite key
		DefaultComposeKey ck;
		string compKey = ck(t,name);
		/// check existence
		entity_idx_type entityIdx = 0;
		map<string,entity_idx_type>::const_iterator resultIter = m_name_id_map.find(compKey);
		if(resultIter != m_name_id_map.end()){
			entity_idx_type nextIdx = _next_entity_idx();
			m_name_id_map[compKey] = nextIdx;
			m_id_name_map[nextIdx] = compKey;
			m_id_type_map[nextIdx] = t;
			m_type_idSet_map[t].insert(nextIdx);
			entityIdx = nextIdx;
		}else{
			entityIdx = resultIter->second;
		}
		return entityIdx;
	}

	entity_idx_type retrieve(string const& compName) const{
		map<string,entity_idx_type>::const_iterator resultIter = m_name_id_map.find(compName);
		return resultIter->second;
	}

	string const& retrieve(entity_idx_type const& idx) const{
		map<entity_idx_type, string>::const_iterator resultIter = m_id_name_map.find(idx);
		return resultIter->second;
	}

	bool exist(string const& compName) const{
		return m_name_id_map.find(compName) != m_name_id_map.end();
	}

	bool exist(entity_idx_type const& idx) const{
		return m_id_name_map.find(idx) != m_id_name_map.end();
	}

	template<ENTITY_TYPE t>
	void retrieve(entity_id_vec& entityVec) const{
		typename map<ENTITY_TYPE,entity_set_type>::const_iterator resultIter = m_type_idSet_map.find(t);
		if(resultIter != m_type_idSet_map.end()){
			entity_set_type const& idxSet = resultIter->second;
			entityVec.insert(entityVec.end(),idxSet.begin(),idxSet.end());
		}
	}

protected:
	map<string,entity_idx_type> m_name_id_map;
	map<entity_idx_type,string> m_id_name_map;
	map<entity_idx_type,ENTITY_TYPE> m_id_type_map;
	map<ENTITY_TYPE, entity_set_type > m_type_idSet_map;
};

template <typename EntityValueType>
class MapEntityContainer{
public:
	typedef EntityValueType entity_value_type;
public:
	void  add(entity_idx_type const& idx, entity_value_type const& value = entity_value_type()){
		m_entity_map[idx] = value;
	}

	entity_value_type const& retrieve(entity_idx_type const& idx) const{
		typename entity_map_type::const_iterator iter = m_entity_map.find(idx);
		return iter->second;
	}

	bool exist(entity_idx_type const& idx) const{
		return (m_entity_map.find(idx) != m_entity_map.end());
	}

protected:
	typedef map<entity_idx_type, entity_value_type> entity_map_type;
	entity_map_type m_entity_map;
};

/**
 * specialized version of EntityMapContainer.
 * For NullValue entity value type, only store the entity id.
 */
template<>
class MapEntityContainer<NullValue>{
public:
	typedef NullValue entity_value_type;
public:
	void  add(entity_idx_type const& idx, entity_value_type const& value = entity_value_type()){
		m_entity_set.insert(idx);
	}

	entity_value_type const& retrieve(entity_idx_type const& idx) const{
		return NULL_VALUE;
	}

	bool exist(entity_idx_type const& idx) const{
		return (m_entity_set.find(idx) != m_entity_set.end());
	}

protected:
	typedef set<entity_idx_type> entity_set_type;
	static entity_value_type NULL_VALUE;
	entity_set_type m_entity_set;
};

/**
 * define Memory based entity container
 */
class MemoryEntityContainer : public EntityContainer<MemoryEntityContainer>{
protected:
	//// aggregate container for different data types
	class _ContainerAggregator : public MapEntityContainer<unsigned char>, public  MapEntityContainer<NullValue>, public MapEntityContainer<float>{

	};

	template<typename EntityValueType>
	MapEntityContainer<EntityValueType>& _cast_map_container(){
		return static_cast<MapEntityContainer<EntityValueType>& >(m_aggregated_container);
	}

public:
	template<ENTITY_TYPE type>
	void  add(entity_idx_type const& idx, typename EntityValueTrait<type>::value_type const& value = typename EntityValueTrait<type>::value_type() ){
		/// use type information to cast the aggregated container to the desired type
		typedef typename EntityValueTrait<type>::value_type entity_value_type;
		/// cast to the appropriate base MapContainer
		MapEntityContainer<entity_value_type>& mapContainer = _cast_map_container<entity_value_type>();
		mapContainer.add(idx,value);
	}

	template<ENTITY_TYPE type>
	typename EntityValueTrait<type>::value_type const& retrieve(entity_idx_type const& idx) const{
		typedef typename EntityValueTrait<type>::value_type entity_value_type;
		/// cast to the appropriate base MapContainer
		MapEntityContainer<entity_value_type>& mapContainer = _cast_map_container<entity_value_type>();
		return mapContainer.retrieve(idx);
	}

	template<ENTITY_TYPE type>
	bool exist(entity_idx_type const& idx) const{
		typedef typename EntityValueTrait<type>::value_type entity_value_type;
		/// cast to the appropriate base MapContainer
		MapEntityContainer<entity_value_type>& mapContainer = _cast_map_container<entity_value_type>();
		return mapContainer.exist(idx);
	}

protected:
	_ContainerAggregator m_aggregated_container;
};

/**
 * definition of entity relation container
 *
 */

typedef set<entity_idx_type> entity_set;
typedef vector<entity_set> entity_list;

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

template<ENTITY_TYPE T1, ENTITY_TYPE T2>
class MapERContainer{
public:
	bool link_entity(entity_idx_type const& fromId, entity_idx_type const& toId){
		m_adj_set_map[fromId].insert(toId);
		return true;
	}

	void get_adj_list(entity_idx_type const& idx, typename AdjListType<T1,T2>::type& idList) const{
		map<entity_idx_type,entity_set>::const_iterator resultIter = m_adj_set_map.find(idx);
		if(resultIter != m_adj_set_map.end()){
			entity_set const& adjSet = resultIter->second;
			idList.insert(idList.end(), adjSet.begin(), adjSet.end());
		}
	}

protected:
	map<entity_idx_type,entity_set> m_adj_set_map;
};

//// use AdjListIdx[ENT_USER] to retrieve the element of returned adjacent list
template<ENTITY_TYPE T>
struct Type2Idx{
};

template<>
struct Type2Idx<ENT_USER>{
	enum { val = 0};
};

template<>
struct Type2Idx<ENT_ITEM>{
	enum { val = 1 };
};


/**
 * specialization with rating entity,
 * T2 could be ENT_USER or ENT_ITEM
 */
template <>
class MapERContainer<ENT_RATING, ENT_USER>{
public:
	void link_entity(entity_idx_type const& fromId, entity_idx_type const& toId){
		m_rating_adj_map[fromId] = toId;
	}

	void get_adj_list(entity_idx_type const& idx, typename AdjListType<ENT_RATING,ENT_USER>::type& toId) const{
		map<entity_idx_type,entity_idx_type>::const_iterator resultIter = m_rating_adj_map.find(idx);
		if(resultIter != m_rating_adj_map.end()){
			//// simply copy the result
			toId = resultIter->second;
		}else{
			toId = 0;
		}
	}
protected:
	//// hold user and item ids
	map<entity_idx_type,entity_idx_type> m_rating_adj_map;
};

/// the same thing
template<>
class MapERContainer<ENT_RATING,ENT_ITEM> : public MapERContainer<ENT_RATING,ENT_USER>{

};

class MemoryERContainer : public ERContainer<MemoryERContainer>{
protected:
	class _ContainerAggregator : public MapERContainer<ENT_USER,ENT_FEATURE>, public MapERContainer<ENT_FEATURE,ENT_USER>,
	public MapERContainer<ENT_ITEM,ENT_FEATURE>, public MapERContainer<ENT_FEATURE,ENT_ITEM>, public MapERContainer<ENT_USER,ENT_RATING>
	, public MapERContainer<ENT_RATING,ENT_USER>, public MapERContainer<ENT_ITEM,ENT_RATING>, public MapERContainer<ENT_RATING,ENT_ITEM>, public MapERContainer<ENT_RATING,ENT_FEATURE>,
	public MapERContainer<ENT_FEATURE,ENT_RATING>{

	};

	template <ENTITY_TYPE T1, ENTITY_TYPE T2>
	MapERContainer<T1,T2>& _cast_to_map_container(){
		return static_cast<MapERContainer<T1,T2>& >(m_aggregated_container);
	}

public:
	template <ENTITY_TYPE T1, ENTITY_TYPE T2>
	bool link_entity(entity_idx_type const& idx1, entity_idx_type const& idx2){
		MapERContainer<T1,T2>& containerRef = _cast_to_map_container<T1,T2>();
		return containerRef.link_entity(idx1,idx2);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2>
	void get_adj_list(entity_idx_type const& idx, typename MapERContainer<T1,T2>::adj_list_type& idList) const{
		MapERContainer<T1,T2>& containerRef = _cast_to_map_container<T1,T2>();
		containerRef.get_adj_list(idx,idList);
	}
protected:
	_ContainerAggregator m_aggregated_container;
};

}
}


#endif /* MEMORYCONTAINER_HPP_ */
