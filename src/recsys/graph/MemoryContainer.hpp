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

class MemoryNameIdContainer : public NameIdContainer{
protected:
	entity_idx_type _next_entity_idx() const{
		return m_name_id_map.size();
	}
public:
	template <ENTITY_TYPE type,typename KeyComposer>
	entity_idx_type add(string const& name, KeyComposer const& keyComposer){
		/// generate the composite key
		string compKey = keyComposer(type,name);
		/// check existence
		entity_idx_type entityIdx = 0;
		map<string,entity_idx_type>::const_iterator resultIter = m_name_id_map.find(compKey);
		if(resultIter != m_name_id_map.end()){
			entity_idx_type nextIdx = _next_entity_idx();
			m_name_id_map[compKey] = nextIdx;
			m_id_name_map[nextIdx] = compKey;
			m_id_type_map[nextIdx] = type;
			m_type_idSet_map[type].insert(nextIdx);
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
		map<entity_idx_type, string>::const_iterator resultIter = m_id_type_map.find(idx);
		return resultIter->second;
	}

	bool exist(string const& compName) const{
		return m_name_id_map.find(compName) != m_name_id_map.end();
	}

	bool exist(entity_idx_type const& idx) const{
		return m_id_name_map.find(idx) != m_id_name_map.end();
	}

	template<ENTITY_TYPE type>
	void retrieve(entity_id_vec& entityVec) const{
		typename map<ENTITY_TYPE,entity_set_type>::const_iterator resultIter = m_type_idSet_map.find(type);
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
class MapContainer{
public:
	typedef EntityValueType entity_value_type;
public:
	entity_value_type const& add(entity_idx_type const& idx, entity_value_type const& value = entity_value_type()){
		m_entity_map[idx] = value;
		return m_entity_map[idx];
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
class MapContainer<NullValue>{
public:
	typedef NullValue entity_value_type;
public:
	entity_value_type const& add(entity_idx_type const& idx, entity_value_type const& value = entity_value_type()){
		m_entity_set.insert(idx);
		return NULL_VALUE;
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
	class _ContainerAggregator : public MapContainer<unsigned char>, MapContainer<NullValue>, MapContainer<float>{

	};
	template<typename EntityValueType>
	MapContainer<EntityValueType>& _cast_map_container(){
		return static_cast<MapContainer<EntityValueType>& >(m_aggregated_container);
	}

public:
	template<ENTITY_TYPE type>
	Entity<typename EntityValueTrait<type>::value_type > const& add(entity_idx_type const& idx, typename EntityValueTrait<type>::value_type const& value = EntityValueTrait<type>::value_type() ){
		/// use type information to cast the aggregated container to the desired type
		typedef typename EntityValueTrait<type>::value_type entity_value_type;
		/// cast to the appropriate base MapContainer
		MapContainer<entity_value_type>& mapContainer = _cast_map_container<entity_value_type>();
		return mapContainer.add(idx,value);
	}

	template<ENTITY_TYPE type>
	typename EntityValueTrait<type>::value_type const& retrieve(entity_idx_type const& idx) const{
		typedef typename EntityValueTrait<type>::value_type entity_value_type;
		/// cast to the appropriate base MapContainer
		MapContainer<entity_value_type>& mapContainer = _cast_map_container<entity_value_type>();
		return mapContainer.retrieve(idx);
	}

	template<ENTITY_TYPE type>
	bool exist(entity_idx_type const& idx) const{
		typedef typename EntityValueTrait<type>::value_type entity_value_type;
		/// cast to the appropriate base MapContainer
		MapContainer<entity_value_type>& mapContainer = _cast_map_container<entity_value_type>();
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

template<ENTITY_TYPE T1, ENTITY_TYPE T2>
class MapERContainer{
public:
	bool link_entity(entity_idx_type const& fromId, entity_idx_type const& toId){
		m_adj_set_map[fromId].insert(toId);
		m_adj_set_map[toId].insert(fromId);
		return true;
	}

	template<typename AdjListType>
	void get_adj_list(entity_idx_type const& idx, AdjListType& idList) const{
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
struct AdjListIdx{
};

template<>
struct AdjListIdx<ENT_USER>{
	enum { val = 0};
};

template<>
struct AdjListIdx<ENT_ITEM>{
	enum { val = 1 };
};

/// array containing user and item id
typedef entity_idx_type UserItemPair[2];

/**
 * specialization with rating entity,
 * T2 could be ENT_USER or ENT_ITEM
 */
template<ENTITY_TYPE T2>
class MapERContainer<ENT_RATING, T2>{
public:
	bool link_entity(entity_idx_type const& fromId, entity_idx_type const& toId){
		m_adj_map[fromId].insert(toId);
		m_adj_map[toId].insert(fromId);
		return true;
	}

	void get_adj_list(entity_idx_type const& idx, UserItemPair& idList) const{
		map<entity_idx_type,UserItemPair>::const_iterator resultIter = m_adj_map.find(idx);
		if(resultIter != m_adj_map.end()){
			//// simply copy the result
			idList = resultIter->second;
		}
	}
protected:
	map<entity_idx_type,UserItemPair> m_adj_map;
};


class MemoryERContainer : public ERContainer<MemoryERContainer>{
public:
	class _ContainerAggregator : public MapERContainer<ENT_USER,ENT_FEATURE>, MapERContainer<ENT_FEATURE,ENT_USER>,
	MapERContainer<ENT_ITEM,ENT_FEATURE>,MapERContainer<ENT_FEATURE,ENT_ITEM>,MapERContainer<ENT_USER,ENT_RATING>
	,MapERContainer<ENT_RATING,ENT_USER>,MapERContainer<ENT_ITEM,ENT_RATING>,MapERContainer<ENT_RATING,ENT_ITEM>,MapERContainer<ENT_RATING,ENT_FEATURE>,
	MapERContainer<ENT_FEATURE,ENT_RATING>{
	public:
		template <ENTITY_TYPE T1, ENTITY_TYPE T2>
		MapERContainer<T1,T2>& cast_to_map_container(){
			return static_cast<MapERContainer<T1,T2> >(*this);
		}
	};
public:
	template <ENTITY_TYPE T1, ENTITY_TYPE T2>
	bool link_entity(entity_idx_type const& idx1, entity_idx_type& idx2){
		MapERContainer<T1,T2>& containerRef = m_aggregated_container.cast_to_map_container<T1,T2>();
		return containerRef.link_entity(idx1,idx2);
	}

	template<ENTITY_TYPE T1, ENTITY_TYPE T2, typename AdjListType>
	void get_adj_list(entity_idx_type const& idx, AdjListType& idList) const{
		MapERContainer<T1,T2>& containerRef = m_aggregated_container.cast_to_map_container<T1,T2>();
		containerRef.get_adj_list(idx,idList);
	}
protected:
	_ContainerAggregator m_aggregated_container;
};

}
}

#include "./MemoryContainerImpl.inc"

#endif /* MEMORYCONTAINER_HPP_ */
