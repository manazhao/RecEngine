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
public:
	string m_feature_query_file;
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
				"dataset name: should be one of [amazon,movielens]")
				("query-file", po::value<string>(&m_feature_query_file)->required(),
								"user profile file")				;
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

	void test_index_user(string const& userName, map<string, int>& profile) {
		rt::DataHostClient dataHostClient =
				m_datahost_client_wrapper_ptr->m_client;
		try {
			m_datahost_client_wrapper_ptr->m_transport->open();
			rt::Response response;
			/// first index the user entity
			dataHostClient.index_interaction(response, userName,
					Entity::ENT_USER, "", Entity::ENT_DEFAULT,
					EntityInteraction::ADD_FEATURE, 1);
			/// add features to the user entity
			for (map<string, int>::iterator iter = profile.begin();
					iter != profile.end(); ++iter) {
				string featureName = iter->first;
				dataHostClient.index_interaction(response, userName,
						Entity::ENT_USER, featureName, Entity::ENT_FEATURE,
						EntityInteraction::ADD_FEATURE, 1.0);
			}
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

	/// dump feature id <-> feature name
	void dump_feature_dict() {
		ModelDriver& MODEL_DRIVER = ModelDriver::ref();
		RecModel& MODEL = MODEL_DRIVER.get_model_ref();
		/// get active dataset
		DatasetExt& activeDataset = MODEL.get_active_ds();
		/// query user profile
		set<int64_t>& featIds = activeDataset.type_ent_ids[Entity::ENT_FEATURE];
		try {
			m_datahost_client_wrapper_ptr->m_transport->open();
			vector<int64_t> featIdVec;
			featIdVec.insert(featIdVec.end(), featIds.begin(), featIds.end());
			vector<string> featNameVec;
			rt::DataHostClient dataHostClient =
					m_datahost_client_wrapper_ptr->m_client;
			dataHostClient.query_entity_names(featNameVec, featIdVec);
			/// now dump the result to file
			string fileName = MODEL_DRIVER.get_dataset_name() + "_feat.csv";
			ofstream ofs;
			ofs.open(fileName.c_str(), std::ofstream::out);
			assert(ofs.good());
			for (size_t i = 0; i < featNameVec.size(); i++) {
				ofs << featNameVec[i] << "," << featIdVec[i] << endl;
			}
			ofs.close();
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

	void save_recommendation_json(string const& fileName, int64_t const& userId, vector<string> const& featNameVec,
			vector<int64_t> const& featIdVec, vector<string> const& recNameVec, vector<int64_t> const& recIdVec){
		ofstream ofs;
		ofs.open(fileName.c_str(), std::ofstream::out|std::ofstream::app);
		assert(ofs.good());
		js::Object obj;
		obj["userId"] = js::String(lexical_cast<string>(userId));
		js::Array features;
		js::Array featureNames;
		for(size_t i = 0; i < featIdVec.size(); i++){
			features.Insert(js::String(lexical_cast<string>(featIdVec[i])));
			featureNames.Insert(js::String(featNameVec[i]));
		}
		obj["features"] = features;
		obj["featureNames"] = featureNames;

		/// insert the recommendation list
		js::Array recNameArr;
		js::Array recIdArr;
		for(size_t i = 0; i < recNameVec.size(); i++){
			recIdArr.Insert(js::String(lexical_cast<string>(recIdVec[i])));
			recNameArr.Insert(js::String(recNameVec[i]));
		}
		obj["recIdList"] = recIdArr;
		obj["recNameList"] = recNameArr;
		/// convert to string
		stringstream ss;
		js::Writer::Write(obj,ss);
		string jsonStr = ss.str();
		jsonStr.erase(
				std::remove_if(jsonStr.begin(), jsonStr.end(), ::isspace),
				jsonStr.end());
		ofs << jsonStr << "\n";
		ofs.close();
	}

//	void save_recommendation(string const& file,
//			vector<Recommendation> const& recList) {
//		ofstream ofs;
//		ofs.open(file.c_str());
//		assert(ofs.good());
//		/// write the result to the file
//		for (vector<Recommendation>::const_iterator iter = recList.begin();
//				iter < recList.end(); ++iter) {
//			Recommendation const& rec = *iter;
//			vector<string> splits;
//			boost::split(splits, rec.id, boost::is_any_of("_"));
//			string url = "http://amazon.com/dp/" + splits[1];
//			ofs << "id:" << url << ", score:" << rec.score << endl;
//		}
//		ofs.close();
//	}

	void get_recommendation(string const& userFile) {
		/// read user profile from a given file
		ifstream ifs;
		ifs.open(userFile.c_str());
		assert(ifs.good());
		string line;
		js::Object userObj;
		while (std::getline(ifs, line)) {
			stringstream ss;
			ss << line;
			js::Object userObj;
			js::Reader::Read(userObj, ss);
			js::String userId = userObj["id"];
			/// put all features in a map
			js::Array features = userObj["features"];
			map<string, int> featureMap;
			for (js::Array::const_iterator iter = features.Begin();
					iter != features.End(); ++iter) {
				js::String feature = *iter;
				featureMap[feature.Value()] = 1;
			}
			this->test_index_user(userId.Value(), featureMap);
			//// now get the recommendation
			vector<rt::Recommendation> recList;
			get_recommendation(recList, userId.Value());
		}
		ifs.close();
	}

	void get_recommendation(std::vector<Recommendation> & _return,
			const std::string& userId) {
		// Your implementation goes here
		printf("get_recommendation\n");
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

		/// make recommendations
		ModelDriver& MODEL_DRIVER = ModelDriver::ref();
		RecModel& recModel = MODEL_DRIVER.get_model_ref();
		_return = recModel.recommend(mappedUserId, entityInteracts);

		/// convert mapped id to original id
		vector<int64_t> idVec;
		/// original item id
		vector<string> origIdVec;
		/// also the feature names
		vector<int64_t> featIdVec;
		vector<string> featNameVec;
		for (vector<Recommendation>::iterator iter = _return.begin();
				iter < _return.end(); ++iter) {
			int64_t itemId = lexical_cast<int64_t>(iter->id);
			idVec.push_back(itemId);
		}
		for(vector<Interact>::iterator iter = entityInteracts[EntityInteraction::ADD_FEATURE].begin(); iter < entityInteracts[EntityInteraction::ADD_FEATURE].end(); ++iter){
			featIdVec.push_back(iter->ent_id);
		}
		try{
			m_datahost_client_wrapper_ptr->m_transport->open();
			/// get original item id
			m_datahost_client_wrapper_ptr->m_client.query_entity_names(origIdVec,
					idVec);
			/// get feature names given feature ids
			m_datahost_client_wrapper_ptr->m_client.query_entity_names(featNameVec,
								featIdVec);
			m_datahost_client_wrapper_ptr->m_transport->close();
		}catch (TException &tx) {
			printf("ERROR: %s\n", tx.what());
		}

		/// put it back
		for (size_t i = 0; i < origIdVec.size(); i++) {
			_return[i].id = origIdVec[i];
		}

		//// save the result
		/// also save user's latent profile
		HierarchicalHybridMF& hhmf =
				dynamic_cast<HierarchicalHybridMF&>(MODEL_DRIVER.get_model_ref());
		RecModel::ModelParam const& mp = hhmf.get_model_param();
		/// dump the recommended item profile
		string fileName = MODEL_DRIVER.get_model_name() + "-" + (string) mp
				+ "-entity-" + userId + ".json";

		save_recommendation_json(fileName, mappedUserId, featNameVec, featIdVec, origIdVec, idVec);
	}
};

int main(int argc, char **argv) {
//	int port = 9090;
	/// create the handler by passing the command line arguments
	shared_ptr<RecEngineHandler> handler(new RecEngineHandler(argc, argv));
//		handler->test_datahost_client();

	//// dump feature dictionary
	handler->dump_feature_dict();
	ModelDriver& MODEL_DRIVER = ModelDriver::ref();
	//// cast to HierarchicalHybridMF reference
	HierarchicalHybridMF& hhmf =
			dynamic_cast<HierarchicalHybridMF&>(MODEL_DRIVER.get_model_ref());

	/// dump the prior information to text file
	RecModel::ModelParam const& modelParam = hhmf.get_model_param();
	string priorFile = MODEL_DRIVER.get_model_name() + "-" + (string) modelParam
			+ ".prior.txt";
	hhmf.dump_prior_information(priorFile);

	/// make recommendation queries for the given profile
	handler->get_recommendation(handler->m_feature_query_file);

	/// save the entity profiles
	string profileFile = MODEL_DRIVER.get_model_name() + "-"
			+ (string) modelParam + ".latent.txt";
	hhmf.dump_model_profile(profileFile);

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

