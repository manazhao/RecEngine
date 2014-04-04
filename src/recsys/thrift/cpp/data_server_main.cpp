// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "HandleData.h"
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

class HandleDataHandler: virtual public HandleDataIf {
protected:
	void _load_amazon_data() {
//		string authorFile =
//				"/home/qzhao2/data/amazon-yms/ratings/processed/author_profile.json";
//		string itemFile =
//				"/home/qzhao2/data/amazon-yms/ratings/processed/item_profile.json";
//		string ratingFile =
//				"/home/qzhao2/data/amazon-yms/ratings/processed/book_rating_filter.json";
		string authorFile =
				"/home/manazhao/rating/amazon_book_rating/author_profile.json";
		string itemFile =
				"/home/manazhao/rating/amazon_book_rating/item_profile.json";
		string ratingFile =
				"/home/manazhao/rating/amazon_book_rating/book_rating_filter.json";

		AmazonJSONDataLoader amazonDataLoader;
//		amazonDataLoader.load_author_profile(authorFile);
//		amazonDataLoader.load_item_profile(itemFile);
		amazonDataLoader.load_rating_file(ratingFile);
	}
public:
	HandleDataHandler() {
		// Your initialization goes here
		cout << "loading Amazon book rating data..." << endl;
		_load_amazon_data();
		cout << "loading finished, ready to take request..." << endl;
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

	void get_entity_ids(std::map<int8_t, std::vector<int64_t> > & _return) {
		// Your implementation goes here
		printf("get_entity_ids\n");
		for(Entity::entity_ptr_map::iterator iter = Entity::m_entity_map.begin(); iter != Entity::m_entity_map.end(); ++iter){
			///  get the mapped id and type
			Entity::mapped_id_type id = iter->first;
			int8_t type = iter->second->m_type;
			cout << "mapped id:" << id << ":" << *(iter->second);
			_return[type].push_back(id);
		}
	}

	void get_entity_interacts(
			std::map<int8_t, std::vector<Interact> > & _return,
			const int64_t entId) {
		// Your implementation goes here
		printf("get_entity_interacts\n");
		EntityInteraction::type_interact_map& entIntMap = EntityInteraction::m_entity_type_interact_map[entId];
		for(EntityInteraction::type_interact_map::iterator iter = entIntMap.begin(); iter != entIntMap.end();++iter){
			/// extract the interaction value
			int8_t intType = iter->first;
			EntityInteraction::entity_interact_vec_ptr& interactVecPtr = iter->second;
			if(interactVecPtr){
				for(EntityInteraction::entity_interact_vec::iterator iter1 = interactVecPtr->begin(); iter1 < interactVecPtr->end(); ++iter1){
					EntityInteraction& tmpInteract = **iter1;
					Interact resultInteract;
					Entity::mapped_id_type fromId = tmpInteract.m_from_entity->m_mapped_id;
					Entity::mapped_id_type toId = tmpInteract.m_to_entity->m_mapped_id;
					resultInteract.ent_id = (fromId == entId ? toId : fromId);
					resultInteract.ent_val = tmpInteract.m_val;
					_return[intType].push_back(resultInteract);
				}
			}
		}
	}
};

int main(int argc, char **argv) {
	int port = 9090;
	shared_ptr<HandleDataHandler> handler(new HandleDataHandler());
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
