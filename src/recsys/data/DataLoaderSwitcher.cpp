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

shared_ptr<DataLoader> DataLoaderSwitcher::get_loader(int argc, char** argv){
	/// determine whether it's a local or remote data loader
	bool localLoader = true;
	string datasetName;
	/// for remote loader
	string host;
	int port;
	//// for local loader
	string userFile, itemFile, ratingFile;
	po::options_description desc(
			"choose data loader based on the command line arguments");
	desc.add_options()
			("help,h", "help message on use this application")
			("local-data,l", po::value<bool>(&localLoader), "indicate data set is local or remote")
			("data-host,h", po::value<string>(&host), "host of the data sharing service")
			("data-port,p", po::value<int>(&port), "port at which the data sharing service is listening at")
			("user-file,u", po::value<string>(&userFile),"user profile file")
			("item-file,i", po::value<string>(&itemFile), "item profile file")
			("rating-file,r", po::value<string>(&ratingFile)->required(),"rating file")
			("dataset-name,n", po::value<string>(&datasetName)->required(),"dataset name: should be one of [amazon,movielense]");
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
		//// check whether the files are supplied
		return _create_local_loader(datasetName, userFile, itemFile, ratingFile);
	}else{
		return _create_remote_loader(host,port);
	}
}

shared_ptr<DataLoader> DataLoaderSwitcher::get_remote_loader(string const& host, int port){
	return _create_remote_loader(host,port);
}

shared_ptr<DataLoader> DataLoaderSwitcher::get_local_loader(string const& datasetName, string const& userFile, string const& itemFile, string const& ratingFile){
	return _create_local_loader(datasetName,userFile,itemFile,ratingFile);
}

shared_ptr<DataLoader> DataLoaderSwitcher::get_remote_loader(int argc, char** argv){
	/// extract host and port for the data sharing service
	string host;
	int port;
	po::options_description desc(
			"load data from remote server");
	desc.add_options()
			("help,h", "help message on use this application")
			("data-host,h", po::value<string>(&host), "host of the data sharing service")
			("data-port,p", po::value<int>(&port), "port at which the data sharing service is listening at");
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

	return _create_remote_loader(host,port);
}

shared_ptr<DataLoader> DataLoaderSwitcher::get_local_loader(int argc, char** argv){
	string userFile, itemFile, ratingFile;
	string datasetName;

	po::options_description desc(
			"Load data from local file system");
	desc.add_options()
			("help,h", "help message on use this application")
			("user-file,u", po::value<string>(&userFile),"user profile file")
			("item-file,i", po::value<string>(&itemFile), "item profile file")
			("rating-file,r", po::value<string>(&ratingFile)->required(),"rating file")
			("dataset-name,n", po::value<string>(&datasetName)->required(),"dataset name: should be one of [amazon,movielense]");
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
	return _create_local_loader(datasetName, userFile, itemFile, ratingFile);
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
	m_dataset_parser_map["amazon"]["item"] = shared_ptr<EntityParser>(new recsys::amazon::ItemEntityParser());
	m_dataset_parser_map["movielens"]["user"] = shared_ptr<EntityParser>();
	m_dataset_parser_map["movielens"]["item"] = shared_ptr<EntityParser>();


}

shared_ptr<DataLoader> DataLoaderSwitcher::_create_remote_loader(string const& host, int& port){
	if(host.empty() || !port){
		cerr << "host name and port must be supplied" << endl;
		exit(1);
	}
	/// the data sets will be loaded from network
	m_data_loader_ptr = shared_ptr<DataLoader>(new ThriftDataLoader(host,port));
	return m_data_loader_ptr;
}

shared_ptr<DataLoader> DataLoaderSwitcher::_create_local_loader(string const& datasetName, string const& userFile,
		string const& itemFile, string const& ratingFile){
	/// check the file existence
	if(!_is_dataset_supported(datasetName)){
		cerr << "dataset: " << datasetName << " is not supported" << endl;
		exit(1);
	}
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
	return m_data_loader_ptr;
}

DataLoaderSwitcher::~DataLoaderSwitcher() {
	// TODO Auto-generated destructor stub
}

}
