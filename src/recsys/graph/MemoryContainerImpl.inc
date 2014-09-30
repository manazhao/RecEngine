namespace recsys{
namespace graph{

template<ENTITY_TYPE t1, ENTITY_TYPE t2>
ostream& operator<< (ostream& oss, MapERContainer<t1,t2> const& erc){
	for(map<entity_idx_type,entity_set>::const_iterator iter = erc.m_adj_set_map.begin(); iter != erc.m_adj_set_map.end(); ++iter){
		oss << iter->first;
		entity_set const& adjList = iter->second;
		for(entity_set::const_iterator iter1 = adjList.begin(); iter1 != adjList.end(); ++iter1){
			oss << (iter1 == adjList.begin() ? ":" : ",") << *iter1;
		}
		oss << "\n";
	}
	return oss;
}

}
}

