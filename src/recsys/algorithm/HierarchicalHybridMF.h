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
#include "recsys/algorithm/Dataset.h"

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
	/// model variables
	/// global bias
	float m_rating_bias_mean;
	float m_rating_bias_var;
	/// mean and covariance for all entities
	mat m_entity_mean;
	mat m_entity_cov;
	colvec m_user_prior_mean_mean;
	float m_user_prior_mean_var;
	float m_user_prior_var_apha;
	float m_user_prior_var_beta;

	colvec m_item_prior_mean_mean;
	float m_item_prior_mean_var;
	float m_item_prior_var_apha;
	float m_item_prior_var_beta;

	/// Gaussian distribution for mean
	colvec m_feat_prior_mean_mean;
	float m_feat_prior_mean_var;
	/// inverse Gamma distribution for variance
	float m_feat_prior_var_alpha;
	float m_feat_prior_var_beta;
	/// rating variance, inverse Gamma distribution
	float m_rating_var_alpha;
	float m_rating_var_beta;

	size_t m_num_users;
	size_t m_num_items;
	size_t m_num_features;
	/// used by thrift client
	boost::shared_ptr<TTransport> m_socket;
	boost::shared_ptr<TTransport> m_transport;
	boost::shared_ptr<TProtocol> m_protocol;
	rt::HandleDataClient m_client;
	/// hold all data from the datahost
	Dataset m_dataset;
	/// training data set
	Dataset m_train_dataset;
	/// testing dataset
	Dataset m_test_dataset;
	/// coldstart testing dataset
	Dataset m_cs_dataset;
protected:
	void _generate_datasets();
	void _load_entities();
	void _load_entity_interacts();
	void _init_from_data_host();
public:
	HierarchicalHybridMF();
	virtual ~HierarchicalHybridMF();
};

} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
