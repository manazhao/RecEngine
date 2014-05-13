/*
 * Graph_test.cpp
 *
 *  Created on: May 12, 2014
 *      Author: qzhao2
 */

#include "All.hpp"

using namespace recsys::graph;

void recsys::graph::test(){
	/// test graph library
	recsys::graph::memory_graph g;
	shared_ent_ptr userPtr = g.index_entity<NullValue>(Entity::ENT_USER,"Qi zhao");
	shared_ent_ptr itemPtr = g.index_entity<NullValue>(Entity::ENT_ITEM,"amazon item 1");
	/// introduce a rating between user and item
	DefaultComposeKey keyCompser;
	shared_ent_ptr ratingPtr = g.index_entity<char>(Entity::ENT_RATING,keyCompser(*userPtr,*itemPtr),1);
	/// now link the entities
	/// user <-> rating
	g.link_entity(userPtr,ratingPtr);
	g.link_entity(itemPtr,ratingPtr);
	/// add user feature
	shared_ent_ptr agePtr = g.index_entity<char>(Entity::ENT_FEATURE,"age_3",1);
	shared_ent_ptr genderPtr = g.index_entity<char>(Entity::ENT_FEATURE,"gender_male",1);
	shared_ent_ptr catPtr = g.index_entity<char>(Entity::ENT_FEATURE,"cat_book",1);
	g.link_entity(userPtr,agePtr);
	g.link_entity(userPtr,genderPtr);
	g.link_entity(itemPtr,catPtr);
	/// dump the graph
	cout << g << endl;
}

int main(int argc, char** argv){
	recsys::graph::test();
}
