/*
 * ModelDriver.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#include "ModelDriver.h"
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include "recsys/data/DataLoaderSwitcher.h"
#include "recsys/data/DataLoader.h"

namespace po = boost::program_options;
using namespace boost;
namespace recsys {

ModelDriver::ModelDriver(int argc, char** argv) {
	// TODO Auto-generated constructor stub

	cout << "Running Recommendation Model for a given dataset" << endl;
	/// extract model parameters from the command line
	Model::ModelParams modelParams(argc,argv);
	DataLoaderSwitcher& dlSwitcher = DataLoaderSwitcher::ref();
	DataLoader& dataLoader = dlSwitcher.get_loader(argc,argv);
	/// construct the model
	shared_ptr<Model> model_inst_ptr(new HierarchicalHybridMF(modelParams,dataLoader.get_dataset_manager()));
	model_inst_ptr->train();
}

ModelDriver::~ModelDriver() {
	// TODO Auto-generated destructor stub
}

}
