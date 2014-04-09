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

void HierarchicalHybridMF::_init() {
	/// first prepare the datasets
	_prepare_datasets();
	/// allocate space for model variables
	_prepare_model_variables();
	/// we are all set, embark the fun journey!
}

void HierarchicalHybridMF::_prepare_model_variables() {
	cout << "############## initialize model variables ##############" << endl;
	timer t;

	cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed() << " >>>>>>>>>>>>>>"
			<< endl;

}
void HierarchicalHybridMF::_prepare_datasets() {
	try {
		m_transport->open();
		cout << "############## retrieve datasets from data host  ##############" << endl;
		timer t;
		m_client.get_dataset(m_dataset,rt::DSType::DS_ALL);
		m_client.get_dataset(m_train_dataset,rt::DSType::DS_TRAIN);
		m_client.get_dataset(m_test_dataset,rt::DSType::DS_TEST);
		m_client.get_dataset(m_cs_dataset,rt::DSType::DS_CS);
		cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed() << " >>>>>>>>>>>>>>" << endl;
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
	_init();
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
