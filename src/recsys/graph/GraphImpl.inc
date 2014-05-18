namespace recsys {
namespace graph {

ostream& operator<<(ostream& oss, entity_list const& entityList) {
	for (entity_list::const_iterator iter = entityList.begin();
			iter != entityList.end(); ++iter) {
		oss << (iter == entityList.begin() ? "" : ",") << *iter;
	}
	return oss;
}

template<typename _T1, typename _T2, typename _T3>
ostream& operator<<(ostream& oss, Graph<_T1, _T2, _T3> const& graph) {
	/// dump the nodes
	oss << "--------------- graph information --------------- \n";
	oss << "--------------- nodes ---------------\n";
	entity_list idList;
	graph.template retrieve_entity<ENT_USER> (idList);
	oss << ">>> user nodes:" << idList << "\n";

	idList.clear();
	graph.template retrieve_entity < ENT_ITEM > (idList);
	oss << ">>> item nodes:" << idList << "\n";

	idList.clear();
	graph.template retrieve_entity < ENT_FEATURE > (idList);
	oss << ">>> feature nodes:" << idList << "\n";

	idList.clear();
	graph.template retrieve_entity < ENT_RATING > (idList);
	oss << ">>> rating nodes:" << idList << "\n";

	idList.clear();
	graph.template retrieve_entity < ENT_RATING_FEATURE > (idList);
	oss << ">>> rating feature nodes:" << idList << "\n";
	oss << "--------------- node connection ---------------\n";
	
	oss << graph.m_entity_relation_container << "\n";
	
	return oss;
}

}
}
