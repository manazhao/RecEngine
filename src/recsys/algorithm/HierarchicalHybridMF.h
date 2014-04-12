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
#include "recsys/data/DatasetExt.h"
#include "vb/prob/Gaussian.h"
#include "vb/prob/InverseGamma.h"
#include "vb/prob/MVInverseGamma.h"

using namespace ::recsys::thrift;
using namespace prob;
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
	void _prepare_datasets();
	void _prepare_model_variables();
	void _init();
public:
	HierarchicalHybridMF();
	virtual ~HierarchicalHybridMF();
protected:
	/// model variables
	/// we use hierarchical Bayesian model and represent each variable
	/// as random variables defined in BN project

	/// user, item and feature latent variables
	vector<DiagMVGaussian> m_entity;
	/// user prior mean
	DiagMVGaussian m_user_prior_mean;
	/// item prior mean
	DiagMVGaussian m_item_prior_mean;
	/// feature prior mean
	DiagMVGaussian m_feature_prior_mean;
	/// user prior covariance matrix
	MVInverseGamma m_user_prior_cov;
	/// item prior covariance matrix
	MVInverseGamma m_item_prior_cov;
	///feature prior covariance matrix
	MVInverseGamma m_feature_prior_cov;
	/// rating variance
	InverseGamma m_rating_var;
	/// assume bias prior is diffuse
	Gaussian m_bias;

	size_t m_num_users;
	size_t m_num_items;
	size_t m_num_features;
	/// used by thrift client
	boost::shared_ptr<TTransport> m_socket;
	boost::shared_ptr<TTransport> m_transport;
	boost::shared_ptr<TProtocol> m_protocol;
	rt::HandleDataClient m_client;
	/// hold all data from the datahost
	DatasetExt m_dataset;
	/// training data set
	DatasetExt m_train_dataset;
	/// testing dataset
	DatasetExt m_test_dataset;
	/// coldstart testing dataset
	DatasetExt m_cs_dataset;
};

} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
