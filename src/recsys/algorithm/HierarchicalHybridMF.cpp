/*
 * HierarchicalHybridMF.cpp
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/timer.hpp>
#include <algorithm>
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>

namespace recsys {


void HierarchicalHybridMF::_generate_datasets() {
//	/// prepare the training, testing datasets as well as the coldstart testing samples
//	/// 80% percent for cross-validation and the rest 20% for coldstart testings
//	std::vector<int64_t> userIds = m_type_entity_id_map[Entity::ENT_USER];
//	std::random_shuffle(userIds.begin(), userIds.end());
//	const float csRatio = 0.2;
//	/// shuffle the array
//	size_t csUserEndId = (size_t) (userIds.size() * csRatio);
//	m_train_interacts = m_test_interacts = m_cs_interacts = vector<
//			map<int8_t, rt::Interact> >(m_entity_type_interacts.size(),
//			map<int8_t, rt::Interact>());
//	for (size_t i = 0; i < csUserEndId; i++) {
//		/// user -> item and user -> feature interactions
//		int64_t userIdx = userIds[i];
//		map<ushort,vector<rt::Interact> >& userInteracts = m_entity_type_interacts[userIdx];
//		for(map<ushort,vector<rt::Interact> >::iterator iter = userInteracts.begin(); iter != userInteracts.end(); ++iter){
//			ushort type = iter->first;
//			vector<rt::Interact>& tmpInteracts = iter->second;
//			for(vector<rt::Interact>::iterator iter1 = tmpInteracts.begin(); iter1 < tmpInteracts.end(); ++iter1){
//				m_train_interacts[userIdx][type].push_back(*iter1);
//				/// add the reverse
//				rt::Interact reverseInteract = *iter1;
//				reverseInteract.ent_id = userIdx;
//				m_train_interacts[iter1->ent_id][type].push_back(reverseInteract);
//			}
//		}
//
//	}
//	const float trainRatio = 0.8;
//	/// now prepare the training and testing dataset for the rest entries
//	for (size_t i = csUserEndId; i < m_num_users; i++) {
//		/// split based on the ratings
//		map<int8_t, vector<rt::Interact> >& interactMap =
//				m_entity_type_interacts[userIds[i]];
//		vector<rt::Interact>& ratingInteracts =
//				interactMap[EntityInteraction::RATE_ITEM];
//		vector<size_t> tmpIdx(ratingInteracts.size());
//		for (size_t j = 0; j < tmpIdx.size(); j++)
//			tmpIdx[j] = j;
//		random_shuffle(tmpIdx.begin(), tmpIdx.end());
//		size_t trainEndIdx = (size_t) (tmpIdx.size() * 0.8);
//		for (size_t j = 0; j < trainEndIdx; j++) {
//			size_t tmpIntIdx = tmpIdx[j];
//			rt::Interact& tmpInteract = ratingInteracts[tmpIntIdx];
//			/// add to the training set
//			m_train_interacts;
//		}
//	}
}

void HierarchicalHybridMF::_load_entities() {
	/// load the entities from server
	/// assume the transport is open
	boost::timer t;
	cout << "get the ids for each type of entity" << endl;
	/// get the entity index information
	m_client.get_entity_ids(m_dataset.m_type_ent_ids);
	/// get the size of each type of entity
	m_num_users = m_dataset.m_type_ent_ids[Entity::ENT_USER].size();
	m_num_items = m_dataset.m_type_ent_ids[Entity::ENT_ITEM].size();
	m_num_features = m_dataset.m_type_ent_ids[Entity::ENT_FEATURE].size();
	cout << "# of users:" << m_num_users << ", # of items:" << m_num_items
			<< ", # of features:" << m_num_features << endl;
	cout << "time elapsed:" << t.elapsed() << endl;
}

void HierarchicalHybridMF::_load_entity_interacts() {
	cout << "load entity interacts (edges) from data host" << endl;
	size_t numEntities = m_dataset.m_type_ent_ids[Entity::ENT_USER].size()
			+ m_dataset.m_type_ent_ids[Entity::ENT_ITEM].size()
			+ m_dataset.m_type_ent_ids[Entity::ENT_FEATURE].size();
	/// reserver space for the vector
	timer t;
	m_client.get_all_interacts(m_dataset.m_ent_type_interacts);
	/// build the entity interaction lookup map
	m_dataset.build_interaction_lookup();
	cout << "time elapsed:" << t.elapsed() << endl;
}

void HierarchicalHybridMF::_init_from_data_host() {
	try {
		m_transport->open();
		/// load entity ids for each type
		_load_entities();
		/// load entity interactions
		_load_entity_interacts();
		m_transport->close();
	} catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
	}
}

HierarchicalHybridMF::HierarchicalHybridMF() :
		m_num_users(0), m_num_items(0), m_num_features(0), m_socket(
				new TSocket("localhost", 9090)), m_transport(
				new TBufferedTransport(m_socket)), m_protocol(
				new TBinaryProtocol(m_transport)), m_client(m_protocol) {
	_init_from_data_host();
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
