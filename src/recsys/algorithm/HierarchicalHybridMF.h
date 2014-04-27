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
#include "Model.h"

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
class HierarchicalHybridMF : public Model{
public:
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
	void _prepare_model_variables();
	void _init_entity_feature_moment_cache();
	void _init();
	void _update_user_prior_mean();
	void _update_user_prior_cov();
	void _update_item_prior_mean();
	void _update_item_prior_cov();
	void _update_feature_prior_mean();
	void _update_feature_prior_cov();
	void _update_user_prior();
	void _update_item_prior();
	void _update_feature_prior();
	void _update_rating_var();
	void _update_bias();
	float _get_mean_rating();
	void _update_entity_from_prior(int64_t const& entityId, int8_t entityType);
	void _update_feature_from_prior(int64_t const& entityId);
	void _update_entity_from_ratings(int64_t const& entityId, int8_t entityType, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_feature_from_entities(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_entity(int64_t const& entityId, int8_t entityType, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_feature(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_entity_feature_moments();
	vec _entity_feature_mean_sum(int64_t const& entityId);
	vec _entity_feature_cov_sum(int64_t const& entityId);
	void _get_entity_feature_cnt();
public:
	HierarchicalHybridMF(ModelParams const& modelParam, DatasetManager& datasetManager);
	float dataset_rmse(DatasetExt& dataset);
	float train_rmse();
	float test_rmse();
	float cs_rmse();
	virtual void train();
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
private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version ){
		ar & m_entity & m_user_prior_mean & m_user_prior_cov & m_item_prior_mean & m_item_prior_cov & m_feature_prior_mean & m_feature_prior_cov
		& m_rating_var & m_bias & m_feat_mean_sum & m_feat_cov_sum & m_feat_cnt_map & m_num_users & m_num_items & m_num_features & m_model_param;
	}
};

ostream& operator << (ostream& oss, HierarchicalHybridMF::ModelParams const& rhs);
ostream& operator << (ostream& oss, HierarchicalHybridMF::RunTimeLog const& rhs);


} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
