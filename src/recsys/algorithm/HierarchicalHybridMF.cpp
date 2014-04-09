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

void HierarchicalHybridMF::_init(){
	/// first prepare the datasets
	_prepare_datasets();
	/// allocate space for model variables
	_prepare_model_variables();
	/// we are all set, embark the fun journey!
}

void HierarchicalHybridMF::_prepare_model_variables(){

}
void HierarchicalHybridMF::_prepare_datasets() {
//	/// prepare the training, testing datasets as well as the coldstart testing samples
//	/// 80% percent for cross-validation and the rest 20% for coldstart testings
	std::set<int64_t> userIdSet = m_dataset.m_type_ent_ids[Entity::ENT_USER];
	vector<int64_t> userIds;
	userIds.assign(userIdSet.begin(),userIdSet.end());
	std::random_shuffle(userIds.begin(), userIds.end());
	const float csRatio = 0.2;
	const float trainRatio = 0.8;
	/// shuffle the array
	size_t csUserEndId = (size_t) (userIds.size() * csRatio);
	/// reserve space for training dataset
	m_train_dataset = Dataset(m_dataset.m_ent_type_interacts.size());
	m_test_dataset = Dataset(m_dataset.m_ent_type_interacts.size());
	m_cs_dataset = Dataset(m_dataset.m_ent_type_interacts.size());
	for (size_t userIdx = 0; userIdx < userIds.size(); userIdx++) {
		/// user -> item and user -> feature interactions
		int64_t userId = userIds[userIdx];
		map<int8_t,vector<rt::Interact> >& userInteracts = m_dataset.m_ent_type_interacts[userId];
		if(userIdx < csUserEndId ){
			m_cs_dataset.add_entity(Entity::ENT_USER,userId);
		}else{
			m_train_dataset.add_entity(Entity::ENT_USER,userId);
			m_test_dataset.add_entity(Entity::ENT_USER,userId);
		}
		for(map<int8_t,vector<rt::Interact> >::iterator iter = userInteracts.begin(); iter != userInteracts.end(); ++iter){
			int8_t type = iter->first;
			vector<rt::Interact>& tmpInteracts = iter->second;
			vector<size_t> tmpIdxVec(tmpInteracts.size());
			for(size_t j = 0; j < tmpInteracts.size(); j++){
				tmpIdxVec[j] = j;
			}
			if(type == EntityInteraction::RATE_ITEM && userIdx >= csUserEndId){
				random_shuffle(tmpIdxVec.begin(),tmpIdxVec.end());
			}
			size_t endTrainIdx = 0;
			if(type == EntityInteraction::RATE_ITEM){
				endTrainIdx = (size_t)(tmpInteracts.size() * trainRatio);
			}
			for(size_t intIdx = 0; 0 < tmpIdxVec.size(); intIdx++){
				// add the destination entity id
				Interact& tmpInteract = tmpInteracts[tmpIdxVec[intIdx]];
				Entity::ENTITY_TYPE endEntType = Entity::ENT_DEFAULT;
				switch(type){
				case EntityInteraction::RATE_ITEM:
					endEntType = Entity::ENT_ITEM;
					break;
				case EntityInteraction::ADD_FEATURE:
					endEntType = Entity::ENT_FEATURE;
					break;
				}
				/// for coldstart dataset, add directly
				if(userIdx < csUserEndId){
					m_cs_dataset.add_entity(endEntType,tmpInteract.ent_id);
				}else{
					/// add user id and feature id to both training and testing dataset
					m_train_dataset.add_entity(Entity::ENT_USER,userId);
					m_test_dataset.add_entity(Entity::ENT_USER,userId);
					if(type == EntityInteraction::ADD_FEATURE){
						m_test_dataset.add_entity(endEntType,tmpInteract.ent_id);
						m_train_dataset.add_entity(endEntType,tmpInteract.ent_id);
					}
					if(type == EntityInteraction::RATE_ITEM){
						if(intIdx < endTrainIdx)
							m_train_dataset.add_entity(endEntType,tmpInteract.ent_id);
						else
							m_test_dataset.add_entity(endEntType,tmpInteract.ent_id);
					}
				}
			}
		}
	}
	m_cs_dataset.filter_interaction(m_dataset.m_ent_type_interacts);
	m_train_dataset.filter_interaction(m_dataset.m_ent_type_interacts);
	m_test_dataset.filter_interaction(m_dataset.m_ent_type_interacts);
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
	timer t;
	m_client.get_all_interacts(m_dataset.m_ent_type_interacts);
	cout << "time elapsed:" << t.elapsed() << endl;
}

void HierarchicalHybridMF::_load_data_thrift() {
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
	_load_data_thrift();
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
