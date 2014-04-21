/*
 * main.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include <recsys/data/EntityInteraction.h>
#include <recsys/data/AmazonJSONDataLoader.h>
#include <recsys/data/AppConfig.h>
#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/lexical_cast.hpp>
using namespace boost;
using namespace recsys;

//#define __TEST_ENTITY__
//#define __TEST_AMAZON__
#define __HYBRID_MODEL__
int main(int argc, char** argv) {
#ifdef __TEST_AMAZON__
	AppConfig::ref().init(argc, argv);
	test_amazon_data_loader();
#endif
#ifdef __TEST_ENTITY__
	test_entity_interaction();
#endif
#ifdef __HYBRID_MODEL__
	assert(argc == 3);
	size_t latDim = lexical_cast<size_t>(string(argv[1]));
	bool useDiagGaussian = lexical_cast<bool>(string(argv[2]));
	HierarchicalHybridMF model(latDim,useDiagGaussian);
	model.train_model();
#endif
	return 0;
}
