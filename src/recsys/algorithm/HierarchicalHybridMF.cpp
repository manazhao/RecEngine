/*
 * HierarchicalHybridMF.cpp
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/timer.hpp>
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>

namespace recsys {

void HierarchicalHybridMF::_load_entities() {
	/// load the entities from server
	/// assume the transport is open
	boost::timer t;
	cout << "get the ids for each type of entity" << endl;
	/// get the entity index information
	m_client.get_entity_ids(m_type_entity_id_map);
	/// get the size of each type of entity
	m_num_users = m_type_entity_id_map[Entity::ENT_USER].size();
	m_num_items = m_type_entity_id_map[Entity::ENT_ITEM].size();
	m_num_features = m_type_entity_id_map[Entity::ENT_FEATURE].size();
	cout << "# of users:" << m_num_users << ", # of items:" << m_num_items
			<< ", # of features:" << m_num_features << endl;
	cout << "time elapsed:" << t.elapsed() << endl;
}

void HierarchicalHybridMF::_load_entity_interacts() {
	cout << "load entity interacts (edges) from data host" << endl;
	size_t numEntities = m_type_entity_id_map[Entity::ENT_USER].size()
			+ m_type_entity_id_map[Entity::ENT_ITEM].size()
			+ m_type_entity_id_map[Entity::ENT_FEATURE].size();
	/// reserver space for the vector
	timer t;
	m_client.get_all_interacts(m_entity_type_interacts);
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
