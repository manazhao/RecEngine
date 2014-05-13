/*
 * Graph_test.cpp
 *
 *  Created on: May 12, 2014
 *      Author: qzhao2
 */

#include "Interface.hpp"

using namespace recsys::graph;

void recsys::graph::test(){
	/// test graph library
	memory_graph g;
	shared_ent_ptr userPtr = g.index_entity<NullValue>(Entity::ENT_USER,"Qi zhao");
	shared_ent_ptr itemPtr = g.index_entity<NullValue>(Entity::ENT_ITEM,"amazon item 1");
	/// introduce a rating between user and item
	string ratingName = lexical_cast<string>(userPtr->get_idx()) + "_" + lexical_cast<string>(itemPtr->get_idx());
	shared_ent_ptr ratingPtr = g.index_entity<char>(Entity::ENT_RATING,ratingName,1);
	/// now link the entities
	/// user <-> rating
	g.link_entity(userPtr,ratingPtr);
	g.link_entity(itemPtr,ratingPtr);
	/// dump the graph
	cout << g << endl;
}

int main(int argc, char** argv){
	recsys::graph::test();
}
