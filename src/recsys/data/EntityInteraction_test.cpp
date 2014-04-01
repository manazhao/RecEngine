/*
 * EntityInteraction_test.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include "EntityInteraction.h"

namespace recsys{

void test_entity_interaction(){
	bool useMemory = true;
	EntityInteraction ei(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",30),useMemory);
	ei.add_from_entity("zq",Entity::ENT_USER);
	ei.add_to_entity("age",Entity::ENT_FEATURE,JSObjectWrapper().add("t","r"));
	ei.index_if_not_exist();
	ei = EntityInteraction(EntityInteraction::ADD_FEATURE, JSObjectWrapper().add("v",1),useMemory);
	ei.add_from_entity("zq",Entity::ENT_USER);
	ei.add_to_entity("gender_m",Entity::ENT_FEATURE,JSObjectWrapper().add("t","c"));
	ei.index_if_not_exist();

	ei = EntityInteraction(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",14.99),useMemory);
	ei.add_from_entity("amz_0x3434",Entity::ENT_ITEM);
	ei.add_to_entity("price",Entity::ENT_FEATURE,JSObjectWrapper().add("t","r"));
	ei.index_if_not_exist();

	/// add rating between user and item
	ei = EntityInteraction(EntityInteraction::RATE_ITEM,JSObjectWrapper().add("v",4),useMemory);
	ei.add_from_entity("zq",Entity::ENT_USER);
	ei.add_to_entity("amz_0x3434",Entity::ENT_ITEM);
	ei.index_if_not_exist();

	/// now dump the graph
	EntityInteraction::entity_interact_vec_ptr entInteracts = EntityInteraction::query("zq",Entity::ENT_USER);
	for(size_t i = 0; i < entInteracts->size(); i++){
		EntityInteraction::entity_interact_ptr& tmpPtr = (*entInteracts)[i];
		/// print the interactions related to current entity
		cout << *tmpPtr << endl;
	}
}
}
