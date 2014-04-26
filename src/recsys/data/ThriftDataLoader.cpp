/*
 * ThriftDataLoader.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#include <recsys/data/ThriftDataLoader.h>

namespace recsys {

ThriftDataLoader::ThriftDataLoader(string const& host, int port) :
		m_host(host),m_port(port),
		m_socket(new TSocket(m_host, m_port)), m_transport(
				new TBufferedTransport(m_socket)), m_protocol(
				new TBinaryProtocol(m_transport)), m_client(m_protocol) {
	/// load the data from the datahost
	try {
		m_transport->open();
		cout
				<< "############## retrieve datasets from data host  ##############"
				<< endl;
		timer t;
		m_client.get_dataset(m_dataset_manager.m_datasets[rt::DSType::DS_ALL], rt::DSType::DS_ALL);
		m_client.get_dataset(m_dataset_manager.m_datasets[rt::DSType::DS_TRAIN], rt::DSType::DS_TRAIN);
		m_client.get_dataset(m_dataset_manager.m_datasets[rt::DSType::DS_TEST], rt::DSType::DS_TEST);
		m_client.get_dataset(m_dataset_manager.m_datasets[rt::DSType::DS_CS], rt::DSType::DS_CS);
		m_dataset.prepare_id_type_map();
		m_train_dataset.prepare_id_type_map();
		m_test_dataset.prepare_id_type_map();
		m_cs_dataset.prepare_id_type_map();
		cout << "---------- dataset ----------" << endl;
		cout << m_dataset_manager.m_datasets[rt::DSType::DS_ALL] << endl;
		cout << "---------- train dataset ----------" << endl;
		cout << m_dataset_manager.m_datasets[rt::DSType::DS_TRAIN] << endl;
		cout << "---------- test dataset ----------" << endl;
		cout << m_dataset_manager.m_datasets[rt::DSType::DS_TEST] << endl;
		cout << "---------- coldstart dataset ----------" << endl;
		cout << m_dataset_manager.m_datasets[rt::DSType::DS_CS] << endl;
		cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed()
				<< " >>>>>>>>>>>>>>" << endl;
		m_transport->close();
	} catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
	}

}

ThriftDataLoader::~ThriftDataLoader() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */