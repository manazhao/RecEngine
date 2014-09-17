/*
 * main.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include <recsys/data/EntityInteraction.h>
#include <recsys/data/AppConfig.h>
#include <recsys/data/ThriftDataLoader.h>
#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/lexical_cast.hpp>
#include <recsys/algorithm/ModelDriver.h>
using namespace boost;
using namespace recsys;

int main(int argc, char** argv) {
	ModelDriver& MODEL_DRIVER = ModelDriver::ref();
	MODEL_DRIVER.run_from_cmd(argc, argv);
	return 0;
}
