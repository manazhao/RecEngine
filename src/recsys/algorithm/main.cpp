/*
 * main.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include <recsys/data/EntityInteraction.h>
#include <recsys/data/AmazonJSONDataLoader.h>
#include <recsys/data/AppConfig.h>
using namespace recsys;

#define __TEST_ENTITY__
int main(int argc, char** argv) {
#ifdef __TEST_AMAZON__
	AppConfig::ref().init(argc, argv);
	test_amazon_data_loader();
#endif
#ifdef _TEST_ENTITY__
	test_entity_interaction();
#endif
	return 0;
}
