/*
 * ThriftDataLoader.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#include <recsys/data/ThriftDataLoader.h>
#include <boost/timer.hpp>

namespace recsys {

ThriftDataLoader::ThriftDataLoader(string const& host, int port) :
		m_host(host), m_port(port), m_socket(new TSocket(m_host, m_port)), m_transport(
				new TBufferedTransport(m_socket)), m_protocol(
				new TBinaryProtocol(m_transport)), m_client(m_protocol) {
	/// load the data from the datahost
	try {
		m_transport->open();
		cout
				<< "############## retrieve datasets from data host  ##############"
				<< endl;
		timer t;
		m_client.get_dataset(dataset(rt::DSType::DS_ALL), rt::DSType::DS_ALL);
		m_client.get_dataset(dataset(rt::DSType::DS_TRAIN),
				rt::DSType::DS_TRAIN);
		m_client.get_dataset(dataset(rt::DSType::DS_TEST), rt::DSType::DS_TEST);
		m_client.get_dataset(dataset(rt::DSType::DS_CS), rt::DSType::DS_CS);
		dataset(rt::DSType::DS_ALL).prepare_id_type_map();
		dataset(rt::DSType::DS_TRAIN).prepare_id_type_map();
		dataset(rt::DSType::DS_TEST).prepare_id_type_map();
		dataset(rt::DSType::DS_CS).prepare_id_type_map();

		/// get the cross validation dataset
		for (size_t i = 0; i < m_dataset_manager->get_cv_folds(); i++) {
			m_client.get_cv_train(m_dataset_manager->cv_dataset(i).m_train, i);
			m_client.get_cv_test(m_dataset_manager->cv_dataset(i).m_test, i);
			m_dataset_manager->cv_dataset(i).m_train.prepare_id_type_map();
			m_dataset_manager->cv_dataset(i).m_test.prepare_id_type_map();
//			m_dataset_manager->cv_dataset(i).m_train.dump_rating_interact();
//			m_dataset_manager->cv_dataset(i).m_test.dump_rating_interact();
		}

		cout << "---------- dataset ----------" << endl;
		cout << dataset(rt::DSType::DS_ALL) << endl;
		cout << "---------- train dataset ----------" << endl;
		cout << dataset(rt::DSType::DS_TRAIN) << endl;
		cout << "---------- test dataset ----------" << endl;
		cout << dataset(rt::DSType::DS_TEST) << endl;
		cout << "---------- coldstart dataset ----------" << endl;
		cout << dataset(rt::DSType::DS_CS) << endl;

		cout << "--------- cross validation datasets --------------" << endl;
		for (size_t i = 0; i < m_dataset_manager->get_cv_folds(); i++) {
			cout << ">>> fold: " << i << endl;
			cout << ">>> train" << endl;
			cout << m_dataset_manager->cv_dataset(i).m_train << endl;
			cout << ">>> test" << endl;
			cout << m_dataset_manager->cv_dataset(i).m_test << endl;

		}
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
