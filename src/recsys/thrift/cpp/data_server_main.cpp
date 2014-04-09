// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "HandleData.h"
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
#include "recsys/data/AmazonJSONDataLoader.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;
using namespace ::recsys::thrift;
using namespace recsys;
using namespace boost;
namespace po = boost::program_options;

class HandleDataHandler: virtual public HandleDataIf {
protected:
	AmazonJSONDataLoader m_amazon_data_loader;
public:
	HandleDataHandler(string const& authorFile, string const& itemFile,
			string const& ratingFile) :
			m_amazon_data_loader(authorFile, itemFile, ratingFile) {
		// Your initialization goes here
		cout
				<< "############### loading Amazon book rating data ###############"
				<< endl;
#ifndef __DEBUG_LOADING__
		m_amazon_data_loader.load_author_profile();
		m_amazon_data_loader.load_item_profile();
#endif
		m_amazon_data_loader.load_rating_file();
		m_amazon_data_loader.prepare_datasets();
		cout
				<< ">>>>>>>>>>>>>>> All done, ready to take request >>>>>>>>>>>>>>>"
				<< endl;
	}

	void add_entity(std::string& _return, const std::string& entityJson) {
		// Your implementation goes here
		printf("add_entity\n");
	}

	void add_interaction(std::string& _return,
			const std::string& interactionJson) {
		// Your implementation goes here
		printf("add_interaction\n");
	}

	void get_recommend_list(std::string& _return, const std::string& userId) {
		// Your implementation goes here
		printf("get_recommend_list\n");
	}

	void get_dataset(Dataset& _return, const DSType::type dsType) {
		// Your implementation goes here
		printf("get_dataset\n");
		_return = m_amazon_data_loader.get_data_set(dsType);
	}

};

void parse_input_files(int argc, char** argv, string & authorFile,
		string & itemFile, string & ratingFile) {
	try {
		po::options_description desc(
				"Load Amazon data into main memory and share with other applications through thrift interface");
		desc.add_options()("help", "help message on use this application")(
				"author-file,a", po::value<string>(&authorFile)->required(),
				"amazon author file")("item-file,i",
				po::value<string>(&itemFile)->required(), "amazon item file")(
				"rating-file,r", po::value<string>(&ratingFile)->required(),
				"amazon rating file");
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			cout << desc << "\n";
			exit(1);
		}
		/// check all required options are provided
		vm.notify();
	} catch (std::exception& e) {
		cerr << "Error:" << e.what() << endl;
		exit(1);
	}

}

int main(int argc, char **argv) {
	string amazonAuthorFile, amazonItemFile, amazonRatingFile;
	parse_input_files(argc, argv, amazonAuthorFile, amazonItemFile,
			amazonRatingFile);
	int port = 9090;
	shared_ptr<HandleDataHandler> handler(
			new HandleDataHandler(amazonAuthorFile, amazonItemFile,
					amazonRatingFile));
	shared_ptr<TProcessor> processor(new HandleDataProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(
			new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

	TSimpleServer server(processor, serverTransport, transportFactory,
			protocolFactory);
	server.serve();
	return 0;
}

