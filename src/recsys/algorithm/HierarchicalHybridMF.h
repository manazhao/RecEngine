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
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include "recsys/thrift/cpp/DataHost.h"
#include "recsys/data/DatasetExt.h"
#include "vb/prob/Gaussian.h"
#include "vb/prob/MVGaussian.h"
#include "vb/prob/DiagMVGaussian.h"
#include "vb/prob/InverseGamma.h"
#include "vb/prob/MVInverseGamma.h"
#include "RecModel.h"

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
class HierarchicalHybridMF : public RecModel{
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
	virtual void _init_training();
	virtual TrainIterLog _train_update();
	virtual float _pred_error(int64_t const& userId, DatasetExt& dataset);
//	virtual float _pred_error(int64_t const& entityId, map<int8_t, vector<Interact> >& entityInteractMap);
	void _lat_ip_moments(DiagMVGaussian & lat1, DiagMVGaussian & lat2, float & firstMoment, float & secondMoment);
	void _rating_bias_moments(float rating, float & firstMoment, float& secondMoment);
	void _update_user_prior_mean();
	void _update_user_prior_cov();
	void _update_item_prior_mean();
	void _update_item_prior_cov();
	void _update_feature_prior_mean();
	void _update_feature_prior_cov();
	void _update_user_prior();
	void _update_item_prior();
	void _update_feature_prior();
	void _update_entity_from_prior_helper(int64_t const& entityId, int8_t entityType, vector<Interact>& featureInteracts);
//	void _update_entity_from_prior(int64_t const& entityId, int8_t entityType);
	virtual void _init_global_bias();
	virtual void _update_rating_var();
	virtual void _update_global_bias();
	void _update_feature_from_prior(int64_t const& entityId);
	virtual void _update_entity_from_ratings(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_feature_from_entities(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	virtual void _update_entity(int64_t const& entityId, int8_t entityType, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_feature(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	vec _entity_feature_mean_sum(vector<Interact> const& featureInteracts);
	vec _entity_feature_mean_sum(int64_t const& entityId);
	vec _entity_feature_cov_sum(int64_t const& entityId);
	void _get_entity_feature_cnt();
	virtual void _add_new_entity(int64_t const& entityId, int8_t const& entityType);
	void _dump_prior_information(string const& fileName);
	void _dump_vec_csv(ostream& oss, arma::vec const& v);
	void _dump_latent_information(string const& fileName);

public:
	HierarchicalHybridMF(ModelParam const& modelParam, shared_ptr<DatasetManager> datasetManager);
	HierarchicalHybridMF();
	virtual string model_summary();
	virtual void dump_model_text(string const& filePrefix = "");
	void dump_recommendation_json(string const& fileName, int64_t const& userId, vector<Interact>& featureInteracts, vector<int64_t>& recList);
	virtual vector<rt::Recommendation> recommend(int64_t const& userId, map<int8_t, vector<rt::Interact> >& userInteracts);
	virtual ~HierarchicalHybridMF();
protected:
	/// user, item and feature latent variables
	vector<DiagMVGaussian> m_entity;
	/// user prior
	DiagMVGaussian m_user_mean_prior;
	MVInverseGamma m_user_cov_prior;
	/// item prior
	MVInverseGamma m_item_cov_prior;
	DiagMVGaussian m_item_mean_prior;
	/// feature prior
	DiagMVGaussian m_feature_prior_mean;
	MVInverseGamma m_feature_prior_cov;
	/// rating variance
	InverseGamma m_rating_var;
	/// assume bias prior is diffuse
	Gaussian m_global_bias;
	map<int64_t,size_t> m_feat_cnt_map;
private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version ){
		ar & boost::serialization::base_object<RecModel>(*this);
		ar & m_entity & m_user_mean_prior & m_user_cov_prior & m_item_mean_prior & m_item_cov_prior & m_feature_prior_mean & m_feature_prior_cov
		& m_rating_var & m_global_bias  & m_feat_cnt_map;
	}
};

ostream& operator << (ostream& oss, HierarchicalHybridMF::RunTimeLog const& rhs);


} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
