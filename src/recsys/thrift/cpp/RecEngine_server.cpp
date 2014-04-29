// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "RecEngine.h"
#include "DataHost.h"
#include "recsys/data/Entity.h"
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "recsys/algorithm/ModelDriver.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace po = boost::program_options;
namespace bf = boost::filesystem;

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace ::recsys::thrift;
using namespace recsys;

namespace rt = recsys::thrift;

class RecEngineHandler: virtual public RecEngineIf {
protected:
	class DataHostClientWrapper {
		friend class RecEngineHandler;
	protected:
		string m_host;
		int m_port;
		/// used by thrift client
		boost::shared_ptr<TTransport> m_socket;
		boost::shared_ptr<TTransport> m_transport;
		boost::shared_ptr<TProtocol> m_protocol;
		rt::DataHostClient m_client;
	public:
		DataHostClientWrapper(string const& host, int port) :
				m_host(host), m_port(port), m_socket(new TSocket(host, port)), m_transport(
						new TBufferedTransport(m_socket)), m_protocol(
						new TBinaryProtocol(m_transport)), m_client(m_protocol) {
		}
	};
protected:
	shared_ptr<DataHostClientWrapper> m_datahost_client_wrapper_ptr;
	string m_model_file;
	string m_model_name;
	string m_dataset_name;
protected:
	void _init_datahost_client(string const& host, int port) {
		cout << ">>> connect to datahost server@" << host << ":" << port
				<< endl;
		m_datahost_client_wrapper_ptr = shared_ptr<DataHostClientWrapper>(
				new DataHostClientWrapper(host, port));
	}

	void _load_model() {
		cout << ">>> load model from file: " << m_model_file << endl;
		ifstream ifs(m_model_file.c_str());
		boost::archive::binary_iarchive ia(ifs);
		ModelDriver& MODEL_DRIVER = ModelDriver::ref();
		ia >> MODEL_DRIVER;
		cout << "done!" << endl;
		/// dump model information
		string modelSummary = MODEL_DRIVER.get_model_ref().model_summary();
		cout << "--------------- model summary ---------------" << endl;
		cout << modelSummary << "\n";
		cout << "---------------------------------------------" << endl;
	}

	void _init_from_cmd(int argc, char** argv) {
		// TODO Auto-generated constructor stub
		/// for remote loader
		string host;
		int port;
		po::options_description desc(
				"choose data loader based on the command line arguments");
		desc.add_options()("help,h", "help message on use this application")(
				"model-file", po::value<string>(&m_model_file),
				"file storing the model training result")("model",
				po::value<string>(&m_model_name)->required(),
				"the name of the model: must be one of [HHMF]")("data-host",
				po::value<string>(&host), "host of the data sharing service")(
				"data-port", po::value<int>(&port),
				"port at which the data sharing service is listening at")(
				"dataset-name", po::value<string>(&m_dataset_name)->required(),
				"dataset name: should be one of [amazon,movielens]");

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
		ModelDriver& MODEL_DRIVER = ModelDriver::ref();
		if (!MODEL_DRIVER.is_model_supported(m_model_name)) {
			cerr << "unsupported model specified:" << m_model_name << endl;
			exit(1);
		}
		/// if model file is supplied and exists, it suggests load a trained model
		if (!m_model_file.empty() && bf::exists(m_model_file)) {
			_init_datahost_client(host, port);
			_load_model();
		}

	}
public:
	RecEngineHandler(int argc, char** argv) {
		// Your initialization goes here
		_init_from_cmd(argc, argv);
	}

	void test_add_entity_interaction(string const& userName, int const& age, string const& gender) {
		rt::DataHostClient dataHostClient =
				m_datahost_client_wrapper_ptr->m_client;
		int ageI = age / 5;
		string ageFeat = "ag_" + lexical_cast<string, int>(ageI);
		string genderFeat = "gd_" + gender;
		try {
			m_datahost_client_wrapper_ptr->m_transport->open();
			rt::Response response;
			dataHostClient.index_interaction(response, userName,
					Entity::ENT_USER, ageFeat, Entity::ENT_FEATURE,
					EntityInteraction::ADD_FEATURE, 1.0);
			dataHostClient.index_interaction(response, userName,
					Entity::ENT_USER, genderFeat, Entity::ENT_FEATURE,
					EntityInteraction::ADD_FEATURE, 1.0);
			int64_t userId = dataHostClient.query_entity_id(userName,
					Entity::ENT_USER);
			/// now retrieve user entity interactions
			map<int8_t, vector<rt::Interact> > userInteracts;
			cout << "features for user " << userId << "-" << userName << ": ";
			dataHostClient.query_entity_interacts(userInteracts, userId);
			/// now dump the interactions
			vector<rt::Interact>& featureInteracts =
					userInteracts[EntityInteraction::ADD_FEATURE];
			for (vector<rt::Interact>::iterator iter1 =
					featureInteracts.begin(); iter1 < featureInteracts.end();
					++iter1) {
				int64_t featId = iter1->ent_id;
				/// query about feature's name
				string featName;
				dataHostClient.query_entity_name(featName, featId);
				cout << featId << "-" << featName << ",";
			}
			cout << endl;
			m_datahost_client_wrapper_ptr->m_transport->close();
		} catch (TException &tx) {
			printf("ERROR: %s\n", tx.what());
		}

	}
	void test_datahost_client() {
		/// make some queries to the datahost server to see whether the response looks normal
		ModelDriver& MODEL_DRIVER = ModelDriver::ref();
		RecModel& MODEL = MODEL_DRIVER.get_model_ref();
		/// get active dataset
		DatasetExt& activeDataset = MODEL.get_active_ds();
		/// query user profile
		set<int64_t>& userIds = activeDataset.type_ent_ids[Entity::ENT_USER];
		size_t userCnt = 0;
		rt::DataHostClient dataHostClient =
				m_datahost_client_wrapper_ptr->m_client;
		set<int64_t> itemIds;
		try {
			/// open the connection
			m_datahost_client_wrapper_ptr->m_transport->open();
			for (set<int64_t>::iterator iter = userIds.begin();
					iter != userIds.end() && userCnt < 10; ++iter, userCnt++) {
				/// get user interactions
				map<int8_t, vector<rt::Interact> > userInteracts;
				int64_t userId = *iter;
				string userName;
				dataHostClient.query_entity_name(userName, userId);
				cout << "features for user " << userId << "-" << userName
						<< ": ";
				dataHostClient.query_entity_interacts(userInteracts, userId);
				/// now dump the interactions
				vector<rt::Interact>& featureInteracts =
						userInteracts[EntityInteraction::ADD_FEATURE];
				for (vector<rt::Interact>::iterator iter1 =
						featureInteracts.begin();
						iter1 < featureInteracts.end(); ++iter1) {
					int64_t featId = iter1->ent_id;
					/// query about feature's name
					string featName;
					dataHostClient.query_entity_name(featName, featId);
					cout << featId << "-" << featName << ",";
				}
				cout << endl;
				vector<rt::Interact>& ratingInteracts =
						userInteracts[EntityInteraction::RATE_ITEM];
				for (vector<rt::Interact>::iterator iter1 =
						ratingInteracts.begin(); iter1 < ratingInteracts.end();
						++iter1) {
					int64_t itemId = iter1->ent_id;
					itemIds.insert(itemId);
				}
			}
			size_t itemCnt = 0;
			for (set<int64_t>::iterator iter = itemIds.begin();
					iter != itemIds.end() && itemCnt < 10; ++iter, itemCnt++) {
				/// get user interactions
				map<int8_t, vector<rt::Interact> > itemInteracts;
				int64_t itemId = *iter;
				string itemName;
				dataHostClient.query_entity_name(itemName, itemId);
				cout << "features for item " << itemId << "-" << itemName
						<< ": ";
				dataHostClient.query_entity_interacts(itemInteracts, itemId);
				/// now dump the interactions
				vector<rt::Interact>& featureInteracts =
						itemInteracts[EntityInteraction::ADD_FEATURE];
				for (vector<rt::Interact>::iterator iter1 =
						featureInteracts.begin();
						iter1 < featureInteracts.end(); ++iter1) {
					int64_t featId = iter1->ent_id;
					/// query about feature's name
					string featName;
					dataHostClient.query_entity_name(featName, featId);
					cout << featId << "-" << featName << ",";
				}
				cout << endl;
			}
			/// shutdown the connection
			m_datahost_client_wrapper_ptr->m_transport->close();
		} catch (TException &tx) {
			printf("ERROR: %s\n", tx.what());
		}
	}

	void get_recommendation(std::vector<Recommendation> & _return,
			const std::string& userId) {
		// Your implementation goes here
		/// first get all the interactions about the user
		map<int8_t, vector<Interact> > entityInteracts;
		int64_t mappedUserId;
		try {
			m_datahost_client_wrapper_ptr->m_transport->open();
			/// first retrieve the integer user id
			mappedUserId =
					m_datahost_client_wrapper_ptr->m_client.query_entity_id(
							userId, Entity::ENT_USER);
			m_datahost_client_wrapper_ptr->m_client.query_entity_interacts(
					entityInteracts, mappedUserId);
			m_datahost_client_wrapper_ptr->m_transport->close();
		} catch (TException &tx) {
			printf("ERROR: %s\n", tx.what());
		}
		printf("get_recommendation\n");
		/// make recommendations
		ModelDriver& MODEL_DRIVER = ModelDriver::ref();
		RecModel& recModel = MODEL_DRIVER.get_model_ref();
		_return = recModel.recommend(mappedUserId, entityInteracts);
		/// convert mapped id to original id
		/// this looks really ugly
		vector<int64_t> idVec;
		for (vector<Recommendation>::iterator iter = _return.begin();
				iter < _return.end(); ++iter) {
			int64_t itemId = lexical_cast<int64_t>(iter->id);
			idVec.push_back(itemId);
		}
		vector<string> origIdVec;
		m_datahost_client_wrapper_ptr->m_transport->open();
		m_datahost_client_wrapper_ptr->m_client.query_entity_names(origIdVec,
				idVec);
		m_datahost_client_wrapper_ptr->m_transport->close();
		/// put it back
		for (size_t i = 0; i < origIdVec.size(); i++) {
			_return[i].id = origIdVec[i];
		}
	}
};

int main(int argc, char **argv) {
	int port = 9090;
	/// create the handler by passing the command line arguments
	shared_ptr<RecEngineHandler> handler(new RecEngineHandler(argc, argv));
	//	handler->test_datahost_client();
	handler->test_add_entity_interaction("gbl-female", 20, "Female");
	vector<rt::Recommendation> recList;
	handler->get_recommendation(recList, "gbl-female");
	/// dump the list
	for(size_t i = 0; i < recList.size(); i++){
		Recommendation& rec = recList[i];
		cout << "id:" << rec.id << ", score:" << rec.score << endl;
	}
//	shared_ptr<TProcessor> processor(new RecEngineProcessor(handler));
//	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
//	shared_ptr<TTransportFactory> transportFactory(
//			new TBufferedTransportFactory());
//	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
//
//	TSimpleServer server(processor, serverTransport, transportFactory,
//			protocolFactory);
//	server.serve();
	return 0;
}

