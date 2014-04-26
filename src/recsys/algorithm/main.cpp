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
using namespace boost;
using namespace recsys;

int main(int argc, char** argv) {
	/// load data from the datahost
	ThriftDataLoader dataLoader("localhost",9090);
	Model::ModelParams modelParam(argc,argv);
	/// initialize model using model parameters and the datasets
	HierarchicalHybridMF model(modelParam,dataLoader.get_dataset_manager());
	model.train();
	return 0;
}
