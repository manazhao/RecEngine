/*
 * HierarchicalHybridMF.h
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#ifndef HIERARCHICALHYBRIDMF_H_
#define HIERARCHICALHYBRIDMF_H_
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <armadillo>
#include "recsys/thrift/cpp/HandleData.h"
using namespace ::recsys::thrift;

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace arma;
namespace rt = recsys::thrift;

namespace recsys {

/**
 * hierarchical hybrid model which uses implicit feedback as prior of
 * user/item latent vector.
 * As the Bayesian network framework is not ready yet, we do the implementation
 * in a rushing style.
 */
class HierarchicalHybridMF {
protected:
	/// latent mean of all entities
	mat m_lat_mean;
	/// latent covariance
	mat m_lat_cov;
	float m_global_bias;
	/// rating variance
	float m_rating_var;
	size_t m_num_users;
	size_t m_num_items;
	size_t m_num_features;
	/// used by thrift client
	boost::shared_ptr<TTransport> m_socket;
	boost::shared_ptr<TTransport> m_transport;
	boost::shared_ptr<TProtocol> m_protocol;
	rt::HandleDataClient m_client;
	map<int8_t, vector<int64_t> > m_type_entity_id_map;
	//// interactions for a given entity, categorized by the interaction type
	/// use vector instead of map for performance purpose
	vector<map<int8_t,vector<rt::Interact> > > m_entity_type_interacts;
protected:
	void _load_entities();
	void _load_entity_interacts();
	void _init_from_data_host();
public:
	HierarchicalHybridMF();
	virtual ~HierarchicalHybridMF();
};

} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
