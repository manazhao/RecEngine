/*
 * MemoryContainerImpl.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#include "MemoryContainer.hpp"

namespace recsys{
namespace graph{

//// initialization of the static NullValue member
NullValue MapEntityContainer<NullValue>::NULL_VALUE = NullValue();

ostream& operator<<(ostream& oss, MapERContainer<ENT_RATING,ENT_USER> const& erc){
	for(map<entity_idx_type,entity_idx_type>::const_iterator iter = erc.m_rating_adj_map.begin(); iter != erc.m_rating_adj_map.end(); ++iter){
		oss << iter->first << ":" << iter->second << "\n";
	}
	return oss;

}

ostream& operator<<(ostream& oss, MapERContainer<ENT_RATING,ENT_ITEM> const& erc){
	for(map<entity_idx_type,entity_idx_type>::const_iterator iter = erc.m_rating_adj_map.begin(); iter != erc.m_rating_adj_map.end(); ++iter){
		oss << iter->first << ":" << iter->second << "\n";
	}
	return oss;
}

ostream& operator<<(ostream& oss, MemoryERContainer const& erc){
	oss << ">>> user feature\n";
	oss << erc._cast_to_map_container<ENT_USER,ENT_FEATURE>();
	oss << ">>> item feature\n";
	oss << erc._cast_to_map_container<ENT_ITEM,ENT_FEATURE>() ;
	oss << ">>> rating\n";
	oss << erc._cast_to_map_container<ENT_RATING,ENT_USER>();
	oss << erc._cast_to_map_container<ENT_RATING,ENT_ITEM>();
	oss << ">>> rating feature\n";
	oss << erc._cast_to_map_container<ENT_RATING,ENT_FEATURE>();
	return oss;
}

}
}
