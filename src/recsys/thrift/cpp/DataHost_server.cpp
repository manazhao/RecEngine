// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "DataHost.h"
#include "recsys/data/DatasetExt.h"
#include <boost/shared_ptr.hpp>
#include <boost/timer.hpp>
#include <boost/program_options.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "recsys/data/AppConfig.h"
#include "recsys/data/Entity.h"
#include "recsys/data/EntityInteraction.h"
#include "recsys/data/DataLoaderSwitcher.h"
#include "boost/filesystem.hpp"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;
using namespace ::recsys::thrift;
using namespace recsys;
using namespace boost;
namespace po = boost::program_options;
namespace bf = boost::filesystem;

class DataHostHandler: virtual public DataHostIf {
protected:
	string m_dataset_name;
	DataLoader& m_data_loader;
	static map<string, int> m_dataset_port_map;
	static map<string, Entity::ENTITY_TYPE> m_entity_type_map;
public:
	static void register_dataset_port() {
		int currentPort = 9090;
		m_dataset_port_map["amazon"] = currentPort;
		m_dataset_port_map["movielens"] = ++currentPort;
	}
	static int get_dataset_port(string const& datasetName) {
		return m_dataset_port_map[datasetName];
	}

	static void register_entity_type() {
		m_entity_type_map["user"] = Entity::ENT_USER;
		m_entity_type_map["item"] = Entity::ENT_ITEM;
		m_entity_type_map["feature"] = Entity::ENT_FEATURE;
	}
	DataHostHandler(DataLoader& dataLoader) :
			m_data_loader(dataLoader) {
		// Your initialization goes here
	}

	void index_interaction(Response& _return, const std::string& fromId,
			const int8_t fromType, const std::string& toId, const int8_t toType,
			const int8_t type, const double val) {
		/// index from entity
		Entity fromEntity(fromId, fromType);
		Entity::entity_ptr fromEntityPtr = fromEntity.index_if_not_exist();

		/// if the toId is empty, it means only adding the from entity
		if (!toId.empty()) {
			Entity toEntity(toId, toType);
			Entity::entity_ptr toEntityPtr = toEntity.index_if_not_exist();

			/// attach the entity pointers to the interaction object
			EntityInteraction interaction(type, val);
			interaction.set_from_entity(fromEntityPtr);
			interaction.set_to_entity(toEntityPtr);
			EntityInteraction::entity_interact_ptr interactPtr =
					interaction.index_if_not_exist();
			cout << "add interaction from: [" << fromId << "-"
					<< fromEntityPtr->get_mapped_id() << "] to [" << toId << "-"
					<< toEntityPtr->get_mapped_id() << "]" << endl;
		}
		_return.status = StatusCode::SC_SUCCESS;
	}

	void query_entity_interacts(
			std::map<int8_t, std::vector<Interact> > & _return,
			const int64_t id) {
		// Your implementation goes here
		printf("query_entity_interacts\n");
		EntityInteraction::type_interact_map result =
				EntityInteraction::m_entity_type_interact_map[id];
		/// convert the result to _return
		for (EntityInteraction::type_interact_map::iterator iter =
				result.begin(); iter != result.end(); ++iter) {
			EntityInteraction::entity_interact_vec& vec = *(iter->second);
			for (EntityInteraction::entity_interact_vec::iterator iter1 =
					vec.begin(); iter1 < vec.end(); ++iter1) {
				EntityInteraction& tmpInteract = **iter1;
				rt::Interact tmpInteract1;
				tmpInteract1.ent_id = tmpInteract.m_to_entity->m_mapped_id;
				tmpInteract1.ent_val = tmpInteract.m_val;
				_return[iter->first].push_back(tmpInteract1);
			}
		}
	}

	void query_entity_name(std::string& _return, const int64_t id) {
		printf("query_entity_name\n");
		_return = "";
		if (Entity::m_id_name_map.find(id) != Entity::m_id_name_map.end()) {
			_return = Entity::m_id_name_map[id];
		}
	}

	void query_entity_names(std::vector<std::string> & _return,
			const std::vector<int64_t> & idList) {
		// Your implementation goes here
		printf("query_entity_names\n");
		for (std::vector<int64_t>::const_iterator iter = idList.begin();
				iter < idList.end(); ++iter) {
			string name = "";
			if (Entity::m_id_name_map.find(*iter)
					!= Entity::m_id_name_map.end()) {
				name = Entity::m_id_name_map[*iter];
			}
			_return.push_back(name);
		}
	}

	int64_t query_entity_id(const std::string& name, const int8_t type) {
		// Your implementation goes here
		/// get the composite key
		printf("query_entity_id\n");
		string cKey = Entity::create_composit_key(name, type);
		int64_t id = -1;
		if (Entity::m_name_id_map.find(cKey) != Entity::m_name_id_map.end()) {
			id = Entity::m_name_id_map[cKey];
		}
		return id;
	}

	void get_dataset(Dataset& _return, const DSType::type dsType) {
		// Your implementation goes here
		printf("get_dataset\n");
		_return = m_data_loader.dataset(dsType);
	}
	void get_cv_train(Dataset& _return, const int8_t foldIdx) {
		// Your implementation goes here
		printf("get_cv_train\n");
		_return = m_data_loader.get_dataset_manager()->cv_dataset(foldIdx).m_train;
	}

	void get_cv_test(Dataset& _return, const int8_t foldIdx) {
		// Your implementation goes here
		printf("get_cv_test\n");
		_return = m_data_loader.get_dataset_manager()->cv_dataset(foldIdx).m_test;
	}

};

//// static members
map<string, int> DataHostHandler::m_dataset_port_map;
map<string, Entity::ENTITY_TYPE> DataHostHandler::m_entity_type_map;

/// parse commandline arguments
void parse_app_args(int argc, char** argv, string& datasetName,
		string& userFile, string& itemFile, string& ratingFile) {
	po::options_description desc(
			"Load dataset into main memory and share with other applications through thrift interface");
	desc.add_options()("help", "help message on use this application")(
			"user-file,u", po::value<string>(&userFile), "user profile file")(
			"item-file,i", po::value<string>(&itemFile), "item profile file")(
			"rating-file,r", po::value<string>(&ratingFile)->required(),
			"rating file")("dataset-name,n",
			po::value<string>(&datasetName)->required(),
			"dataset name: should be one of [amazon,movielens]");

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
}

int main(int argc, char **argv) {
	string datasetName;
	string userFile, itemFile, ratingFile;
	/// parse commandline arguments
	parse_app_args(argc, argv, datasetName, userFile, itemFile, ratingFile);

	/// register listening port
	DataHostHandler::register_dataset_port();
	DataHostHandler::register_entity_type();

	/// get the data loader
	DataLoaderSwitcher& dlSwitcher = DataLoaderSwitcher::ref();
	shared_ptr<DataLoader> dataLoaderPtr = dlSwitcher.get_local_loader(
			datasetName, userFile, itemFile, ratingFile);

	/// attach data loader to the handler
	shared_ptr<DataHostHandler> handler(new DataHostHandler(*dataLoaderPtr));

	/// run Thrift Service
	/// get the port number assigned to the given dataset
	int port = DataHostHandler::get_dataset_port(datasetName);
	shared_ptr<TProcessor> processor(new DataHostProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(
			new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	TSimpleServer server(processor, serverTransport, transportFactory,
			protocolFactory);
	cout << "dataset is loaded, start the sharing service at: " << port << endl;
	server.serve();
	return 0;
}

