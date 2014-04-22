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
#include "vb/prob/MVGaussian.h"
#include "vb/prob/DiagMVGaussian.h"
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
public:
	struct ModelParams{
		/// the dimensionality of the latent vector
		size_t m_lat_dim;
		/// maximum number of iterations
		size_t m_max_iter;
		/// whether use diagonal multivariate Gaussian
		bool m_diag_cov;
		// whether use feature
		bool m_use_feature;
		ModelParams(size_t const& latDim = 10, size_t const& maxIter = 10, bool diagCov = true, bool useFeature = true);
		ModelParams(int argc, char** argv);
	};
	struct RunTimeLog{
		/// iteration index
		size_t m_iter;
		/// training rmse
		float m_train_rmse;
		/// testing rmse
		float m_test_rmse;
		/// coldstart dataset rmse
		float m_cs_rmse;
		/// iteration time
		float m_iter_time;
	};
protected:
	void _lat_ip_moments(DiagMVGaussian & lat1, DiagMVGaussian & lat2, float & firstMoment, float & secondMoment);
	void _rating_bias_moments(float rating, float & firstMoment, float& secondMoment);
	void _prepare_datasets();
	void _prepare_model_variables();
	void _init();
	void _update_user_prior_mean();
	void _update_user_prior_cov();
	void _update_item_prior_mean();
	void _update_item_prior_cov();
	void _update_user_prior();
	void _update_item_prior();
	void _update_feature_prior();
	void _update_rating_var();
	void _update_bias();
	float _get_mean_rating();
	void _update_user_or_item(int64_t const& entityId, int8_t entityType, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_item(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_feature(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_entity_feature_moments();
	void _dump_interact_array(vector<Interact> const& vec);
public:
	HierarchicalHybridMF(ModelParams const& modelParam);
	float dataset_rmse(DatasetExt& dataset);
	float train_rmse();
	float test_rmse();
	float cs_rmse();
	void infer();
	virtual ~HierarchicalHybridMF();
protected:
	/// model variables
	/// we use hierarchical Bayesian model and represent each variable
	/// as random variables defined in BN project

	/// user, item and feature latent variables
	vector<DiagMVGaussian> m_entity;
	/// user prior
	DiagMVGaussian m_user_prior_mean;
	MVInverseGamma m_user_prior_cov;
	/// item prior
	MVInverseGamma m_item_prior_cov;
	DiagMVGaussian m_item_prior_mean;
	/// feature prior
	DiagMVGaussian m_feature_prior_mean;
	MVInverseGamma m_feature_prior_cov;
	/// rating variance
	InverseGamma m_rating_var;
	/// assume bias prior is diffuse
	Gaussian m_bias;
	///
	map<int64_t,vec> m_feat_mean_sum;
	map<int64_t,vec> m_feat_cov_sum;
	map<int64_t,size_t> m_feat_cnt_map;
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
	ModelParams m_model_param;
};

ostream& operator << (ostream& oss, HierarchicalHybridMF::ModelParams const& rhs);
ostream& operator << (ostream& oss, HierarchicalHybridMF::RunTimeLog const& rhs);


} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
