/*
 * HHMFBias.h
 *
 *  Created on: May 7, 2014
 *      Author: qzhao2
 */

#ifndef HHMFBIAS_H_
#define HHMFBIAS_H_

#include <recsys/algorithm/HierarchicalHybridMF.h>

namespace recsys {

class HHMFBias: public recsys::HierarchicalHybridMF {
protected:
	//// user bias term
	map<int64_t,Gaussian> m_user_bias;
	/// item bias term
	map<int64_t,Gaussian> m_item_bias;
	/// prior for the user bias and item bias
	Gaussian m_user_bias_mean_prior;
	InverseGamma m_user_bias_var_prior;
	Gaussian m_item_bias_mean_prior;
	InverseGamma m_item_bias_var_prior;
protected:
	virtual void _init_training();
	virtual TrainIterLog _train_update();
	void _init_user_bias();
	void _init_item_bias();
	void _init_bias();
	void _init_global_bias();
	//// variational Bayesian update on model variable
	void _update_global_bias();
	void _update_user_bias_prior_mean();
	void _update_item_bias_prior_mean();
	void _update_user_bias_prior_var();
	void _update_item_bias_prior_var();
	void _update_user_bias_prior();
	void _update_item_bias_prior();
	void _update_user_bias_from_rating(int64_t const& userId, vector<Interact>& ratingInteracts);
	void _update_item_bias_from_rating(int64_t const& itemId, vector<Interact>& ratingInteracts);
	void _update_user_bias_from_prior(int64_t const& userId, vector<Interact>& featureInteracts);
	void _update_item_bias_from_prior(int64_t const& itemId, vector<Interact>& featureInteracts);
	void _update_user_bias(int64_t const& userId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_item_bias(int64_t const& userId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_rating_var();
	void _rating_bias_moments(float rating,int64_t const& userId, int64_t const& itemId,
			float & firstMoment, float& secondMoment);
	void _update_entity_from_ratings(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	virtual float _pred_error(int64_t const& userId, DatasetExt& dataset);
public:
	HHMFBias();
	virtual ~HHMFBias();
private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version ){
		ar & boost::serialization::base_object<HierarchicalHybridMF>(*this);
		ar & m_user_bias & m_item_bias & m_user_bias_mean_prior & m_user_bias_var_prior & m_item_bias_mean_prior & m_item_bias_var_prior;
	}

};

} /* namespace recsys */

#endif /* HHMFBIAS_H_ */
