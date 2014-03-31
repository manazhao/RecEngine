/*
 * EntityInteraction_test.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include "EntityInteraction.h"

namespace recsys{

void test_entity_interaction(){
	EntityInteraction ei(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",30));
	ei.add_from_entity("zq",Entity::ENT_USER);
	ei.add_to_entity("age",Entity::ENT_FEATURE,JSObjectWrapper().add(string("t"),string("r")));
	ei.index_if_not_exist();
}
}
