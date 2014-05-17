/*
 * Graph_test.cpp
 *
 *  Created on: May 12, 2014
 *      Author: qzhao2
 */


#include "Graph.hpp"

using namespace recsys::graph;

void recsys::graph::test(){
	/// test graph library
	recsys::graph::memory_graph g;
	entity_idx_type userIdx = g.index_entity<ENT_USER>("Qi zhao");
	entity_idx_type itemIdx = g.index_entity<ENT_ITEM>("amazon item 1");
	/// introduce a rating between user and item
	DefaultComposeKey keyCompser;
	string ratingKey = keyCompser(userIdx,itemIdx);
	entity_idx_type ratingIdx = g.index_entity<ENT_RATING>(ratingKey,3);
	/// now link the entities
	/// user <-> rating
	g.link_entity<ENT_USER,ENT_RATING>(userIdx, ratingIdx);
	g.link_entity<ENT_ITEM,ENT_RATING>(itemIdx,ratingIdx);
	/// add user feature
	entity_idx_type ageIdx = g.index_entity<ENT_FEATURE>("age_3");
	entity_idx_type genderIdx = g.index_entity<ENT_FEATURE>("gender_male");
	entity_idx_type catIdx = g.index_entity<ENT_FEATURE>("cat_book");
	g.link_entity<ENT_USER,ENT_FEATURE>(userIdx,genderIdx);
	g.link_entity<ENT_USER,ENT_FEATURE>(userIdx,ageIdx);
	g.link_entity<ENT_ITEM,ENT_FEATURE>(itemIdx,catIdx);
	/// dump the graph
//	cout << g << endl;
}

int main(int argc, char** argv){
	recsys::graph::test();
}
