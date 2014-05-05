/*
 * ModelDriver.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#include "ModelDriver.h"
#include "recsys/data/DataLoaderSwitcher.h"
#include "recsys/data/DataLoader.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace po = boost::program_options;
namespace bf = boost::filesystem ;

using namespace boost;
namespace recsys {

void ModelDriver::_save_model(){
	/// save the model to the file
	if(m_model_file.empty()){
		/// generate a file name
		stringstream ss;
		ss << m_model_name << "-" << (string)(get_model_ref().m_model_param) << "-model" << (get_model_ref().m_model_selection ? "[SEL]": "") << ".bin";
		string fileName = ss.str();
		/// get working directory
		boost::filesystem::path cwd(boost::filesystem::current_path());
		m_model_file = string(cwd.c_str()) + "/" + fileName;
	}
	//// create an archive
	cout << ">>> start to write the model to file: " << m_model_file << endl;
	std::ofstream ofs(m_model_file.c_str());
	boost::archive::binary_oarchive oa(ofs);
	/// serialize the model
	oa << *this;
	cout << "done!" << endl;
}

void ModelDriver::_load_model(){
	cout << ">>> load model from file: " << m_model_file << endl;
	ifstream ifs(m_model_file.c_str());
	boost::archive::binary_iarchive ia(ifs);
	ia >> *this;
	cout << "done!" << endl;
}

void ModelDriver::run_from_cmd(int argc, char** argv) {
	// TODO Auto-generated constructor stub
	/// determine whether it's a local or remote data loader
	bool localLoader = true;
	bool modelSelection = false;
	string datasetName;
	/// for remote loader
	string host;
	int port;
	//// for local loader
	string userFile, itemFile, ratingFile;
	/// parameters
	RecModel::ModelParam modelParams;
	po::options_description desc(
			"choose data loader based on the command line arguments");
	desc.add_options()
			("help,h", "help message on use this application")
			("data-host", po::value<string>(&host), "host of the data sharing service")
			("data-port", po::value<int>(&port), "port at which the data sharing service is listening at")
			("user-file", po::value<string>(&userFile),"user profile file")
			("item-file", po::value<string>(&itemFile), "item profile file")
			("rating-file", po::value<string>(&ratingFile),"rating file")
			("dataset-name", po::value<string>(&datasetName)->required(),"dataset name: should be one of [amazon,movielens]")
			("lat-dim", po::value<size_t>(&(modelParams.m_lat_dim)), "latent dimension")
			("diag-cov", po::value<bool>(&(modelParams.m_diag_cov)),"diagonal multivariate Gaussian")
			("max-iter", po::value<size_t>(&(modelParams.m_max_iter)), "maximum number of iterations")
			("use-feature", po::value<bool>(&(modelParams.m_use_feature)), "integrate content feature as prior")
			("model-file", po::value<string>(&m_model_file), "file storing the model training result, must be specified for model training")
			("model", po::value<string>(&m_model_name)->required(), "the name of the model: must be one of [HHMF,AVG,POP]")
			("cross-validation", "run cross validation on the dataset");

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
	if(!is_model_supported(m_model_name)){
		cerr << "unsupported model specified:" <<  m_model_name << endl;
		exit(1);
	}
	/// if model file is supplied and exists, it suggests load a trained model
	if(!m_model_file.empty() && bf::exists(m_model_file)){
		_load_model();
		/// dump model information
		string modelSummary = get_model_ref().model_summary();
		cout << "--------------- model summary ---------------" << endl;
		cout << modelSummary << "\n";
		cout << "---------------------------------------------" << endl;
	}
	else{
		if(vm.count("cross-validation")){
			modelSelection = true;
		}
		/// The presence of rating file implies local data loader
		localLoader = !ratingFile.empty();
		/// choose the data loader
		DataLoaderSwitcher& dlSwitcher = DataLoaderSwitcher::ref();
		shared_ptr<DataLoader> dataLoaderPtr;
		/// if it's a local loader, extract the required files
		if(localLoader){
			//// check whether the files are supplied
			dataLoaderPtr = dlSwitcher.get_local_loader(datasetName, userFile, itemFile, ratingFile);
		}else{
			dataLoaderPtr = dlSwitcher.get_remote_loader(host,port);
		}
		/// construct the model
		RecModel& modelRef = get_model_ref();
		modelRef.setup_train(modelParams,dataLoaderPtr->get_dataset_manager());
		if(modelSelection){
			/// running model selection
//			modelRef.train(modelRef.get_train_ds(),modelRef.get_test_ds(),modelRef.get_cs_ds());
			cout << ">>> evaluate model by cross validation\n" << endl;
			for(size_t i = 0; i < 5; i++){
				cout << ">>> cv fold: " << i << endl;
				DatasetExt& cvTrain = dataLoaderPtr->get_dataset_manager()->cv_dataset(i).m_train;
				DatasetExt& cvTest = dataLoaderPtr->get_dataset_manager()->cv_dataset(i).m_test;
				DatasetExt dummyDataset;
				modelRef.train(cvTrain,cvTest,dummyDataset);
			}
		}else{
			/// train model using entire dataset
			DatasetExt dummyDataset;
			modelRef.train(modelRef.get_ds(), dummyDataset, dummyDataset);
		}
		_save_model();
	}

}

ModelDriver::~ModelDriver() {
	// TODO Auto-generated destructor stub
}

}
