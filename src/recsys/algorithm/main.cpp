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
	HierarchicalHybridMF::ModelParams modelParam(argc,argv);
//	modelParam.m_use_feature = true;
//	modelParam.m_lat_dim = 2;
	HierarchicalHybridMF model(modelParam);
	model.infer();
#endif
	return 0;
}
