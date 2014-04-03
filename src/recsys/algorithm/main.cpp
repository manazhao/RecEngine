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

int main(int argc, char** argv) {
	AppConfig::ref().init(argc, argv);
	test_amazon_data_loader();
	return 0;
}
