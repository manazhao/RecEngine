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
	/// determine whether it's a local or remote data loader
	bool localLoader = true;
	string datasetName;
	/// for remote loader
	string host;
	int port;
	//// for local loader
	string userFile, itemFile, ratingFile;
	/// parameters
	Model::ModelParams modelParams;
	po::options_description desc(
			"choose data loader based on the command line arguments");
	desc.add_options()
			("help,h", "help message on use this application")
			("local-data,l", po::value<bool>(&localLoader), "indicate data set is local or remote")
			("data-host,dh", po::value<string>(&host), "host of the data sharing service")
			("data-port,p", po::value<int>(&port), "port at which the data sharing service is listening at")
			("user-file,u", po::value<string>(&userFile),"user profile file")
			("item-file,i", po::value<string>(&itemFile), "item profile file")
			("rating-file,r", po::value<string>(&ratingFile),"rating file")
			("dataset-name,n", po::value<string>(&datasetName)->required(),"dataset name: should be one of [amazon,movielens]")
			("lat-dim", po::value<size_t>(&(modelParams.m_lat_dim)), "latent dimension")
			("diag-cov", po::value<bool>(&(modelParams.m_diag_cov)),"diagonal multivariate Gaussian")
			("max-iter", po::value<size_t>(&(modelParams.m_max_iter)), "maximum number of iterations")
			("use-feature", po::value<bool>(&(modelParams.m_use_feature)), "integrate content feature as prior");

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			cout << desc << "\n";
			exit(1);
		}
		/// check all required options are provided
		vm.notify();
	} catch (std::exception& e) {
		cerr << "Error:" << e.what() << endl;
		cerr << "\nUsage:\n" << desc << "\n\n";
		exit(1);
	}

	/// choose the data loader
	DataLoaderSwitcher& dlSwitcher = DataLoaderSwitcher::ref();
	DataLoader* dataLoaderPtr  = NULL;
	/// if it's a local loader, extract the required files
	if(localLoader){
		//// check whether the files are supplied
		dataLoaderPtr = &(dlSwitcher.get_local_loader(datasetName, userFile, itemFile, ratingFile));
	}else{
		dataLoaderPtr = &(dlSwitcher.get_remote_loader(host,port));
	}
	/// construct the model
	shared_ptr<Model> model_inst_ptr(new HierarchicalHybridMF(modelParams,dataLoaderPtr->get_dataset_manager()));
	model_inst_ptr->train();
}

ModelDriver::~ModelDriver() {
	// TODO Auto-generated destructor stub
}

}
