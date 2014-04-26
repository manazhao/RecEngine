/*
 * DataLoaderSwitcher.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#include "DataLoaderSwitcher.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace bf = boost::filesystem ;

using namespace boost;

namespace recsys {

DataLoader& DataLoaderSwitcher::get_loader(int argc, char** argv){
	/// determine whether it's a local or remote data loader
	bool localLoader = true;

	po::options_description desc(
			"choose data loader based on the command line arguments");
	desc.add_options()("help,h", "help message on use this application")(
			"local-data,l", po::value<bool>(&localLoader), "load data from local file system");
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
	/// if it's a local loader, extract the required files
	if(localLoader){
		return get_local_loader(argc,argv);
	}else{
		return get_remote_loader(argc,argv);
	}
}

DataLoader& DataLoaderSwitcher::get_remote_loader(int argc, char** argv){
	string userFile, itemFile, ratingFile;
	string datasetName;
	_parse_local_loader(argc, argv, datasetName, userFile, itemFile, ratingFile);
	/// create the dataloader shared pointer
	m_data_loader_ptr = shared_ptr<DataLoader>(new JSONDataLoader());
	JSONDataLoader& loaderRef = dynamic_cast<JSONDataLoader&>(*m_data_loader_ptr);
	/// now choose the data loader based on dataset name
	// Your initialization goes here
	cout
			<< "############### loading " << datasetName <<" ###############"
			<< endl;
	if(!userFile.empty() && m_dataset_parser_map[datasetName]["user"]){
		loaderRef.load_user_profile(userFile,*(m_dataset_parser_map[datasetName]["user"]));
	}
	if(!itemFile.empty() && m_dataset_parser_map[datasetName]["item"]){
		loaderRef.load_item_profile(itemFile,*(m_dataset_parser_map[datasetName]["item"]));
	}
	loaderRef.load_user_item_rating(ratingFile);
	cout << "generate training, testing and coldstart dataset " << endl;
	loaderRef.prepare_datasets();
	cout
			<< ">>>>>>>>>>>>>>> All done! data is ready for use >>>>>>>>>>>>>>>"
			<< endl;
	return loaderRef;
}

DataLoader& DataLoaderSwitcher::get_local_loader(int argc, char** argv){
	/// extract host and port for the data sharing service
	string host;
	int port;
	_parse_remote_loader(argc, argv, host, port);
	/// the data sets will be loaded from network
	m_data_loader_ptr = shared_ptr<DataLoader>(new ThriftDataLoader(host,port));
	return *m_data_loader_ptr;
}



bool DataLoaderSwitcher::_is_dataset_supported(string const& datasetName){
		return m_dataset_parser_map.find(datasetName) != m_dataset_parser_map.end();
}

DataLoaderSwitcher::DataLoaderSwitcher() {
	_register_entity_parsers();
}

void DataLoaderSwitcher::_register_entity_parsers(){
	// TODO Auto-generated constructor stub
	m_dataset_parser_map["amazon"]["user"] = shared_ptr<EntityParser>(new recsys::amazon::UserEntityParser());
	m_dataset_parser_map["amazon"]["item"] = shared_ptr<EntityParser>(new recsys::amazon::UserEntityParser());
}

void DataLoaderSwitcher::_parse_remote_loader(int argc, char** argv, string& host, int& port){
	po::options_description desc(
			"parse arguments for remote data sharing service");
	desc.add_options()("help,h", "help message on use this application")(
			"data-host,h", po::value<string>(&host),
			"host of the data sharing service")("data-port,p", po::value<int>(
			&port), "port at which the data sharing service is listening at");
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
}

void DataLoaderSwitcher::_parse_local_loader(int argc, char** argv,string& datasetName, string & userFile,
		string & itemFile, string & ratingFile){
	po::options_description desc(
			"Load dataset into main memory and share with other applications through thrift interface");
	desc.add_options()("help", "help message on use this application")(
			"user-file,u", po::value<string>(&userFile),
			"user profile file")("item-file,i",
			po::value<string>(&itemFile), "item profile file")(
			"rating-file,r", po::value<string>(&ratingFile)->required(),
			"rating file")(
					"dataset-name,n", po::value<string>(&datasetName)->required(),
					"dataset name: should be one of [amazon,movielense]");
	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			cout << desc << "\n";
			exit(1);
		}
		/// check all required options are provided
		vm.notify();
	} catch (std::exception& e) {
		cerr << "Error:" << e.what() << "\n\n";
		cout << "Usage:" << "\n";
		cout << desc << "\n";
		exit(1);
	}

	/// check the file existence
	if(!userFile.empty() && !bf::exists(userFile)){
		cerr << "user profile file: " << userFile << " does not exist" << endl;
		exit(1);
	}

	if(!itemFile.empty() && !bf::exists(itemFile)){
		cerr << "item profile file: " << itemFile << " does not exist" << endl;
		exit(1);
	}

	if(!bf::exists(ratingFile)){
		cerr << "rating file: " << ratingFile << " does not exist" << endl;
		exit(1);
	}

	if(!_is_dataset_supported(datasetName)){
		cerr << "dataset: " << datasetName << " is not supported" << endl;
		exit(1);
	}
}



DataLoaderSwitcher::~DataLoaderSwitcher() {
	// TODO Auto-generated destructor stub
}

}
