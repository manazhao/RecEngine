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
	virtual void _init_global_bias();
	virtual void _update_global_bias();
	virtual void _update_rating_var();
	void _update_user_from_ratings(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_item_from_ratings(int64_t const& entityId, map<int8_t,vector<Interact> > & typeInteracts);
	virtual void _update_entity(int64_t const& entityId, int8_t entityType, map<int8_t,vector<Interact> > & typeInteracts);
	virtual float _pred_error(int64_t const& userId, DatasetExt& dataset);
	virtual void _add_new_entity(int64_t const& entityId, int8_t const& entityType);
protected:
	void _init_user_bias();
	void _init_item_bias();
	void _init_bias();
	void _update_user_bias_prior_mean();
	void _update_item_bias_prior_mean();
	void _update_user_bias_prior_var();
	void _update_item_bias_prior_var();
	void _update_user_bias_prior();
	void _update_item_bias_prior();
	void _update_user_bias_from_ratings(int64_t const& userId, vector<Interact>& ratingInteracts);
	void _update_item_bias_from_ratings(int64_t const& itemId, vector<Interact>& ratingInteracts);
	void _update_user_bias_from_prior(int64_t const& userId, vector<Interact>& featureInteracts);
	void _update_item_bias_from_prior(int64_t const& itemId, vector<Interact>& featureInteracts);
	void _update_user_bias(int64_t const& userId, map<int8_t,vector<Interact> > & typeInteracts);
	void _update_item_bias(int64_t const& userId, map<int8_t,vector<Interact> > & typeInteracts);
	void _rating_bias_moments(float rating,int64_t const& userId, int64_t const& itemId,
			float & firstMoment, float& secondMoment);
public:
	HHMFBias();
	void dump_user_bias(string const& userFile);
	void dump_item_bias(string const& itemFile);
	virtual string model_summary();
	virtual vector<rt::Recommendation> recommend(int64_t const& userId, map<int8_t, vector<rt::Interact> >& userInteracts);
	virtual ~HHMFBias();
private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version ){
		ar & boost::serialization::base_object<HierarchicalHybridMF>(*this);
		ar & m_user_bias & m_item_bias & m_user_bias_mean_prior & m_user_bias_var_prior & m_item_bias_mean_prior & m_item_bias_var_prior;
	}

};

/**
 * @brief evaluate the first and second moments for the sum/subtraction of two independent random variables
 *
 * @arguments:
 * x11	first moment of first random variable
 * x12 	second moment of the first random variable
 * x21	first moment of the second random variable
 * x22	second moment of the second random variable
 *
 * r1	first moment of the  sum/subtraction
 * r2	second moment of the  sum/subtraction
 *
 */
void sum_moments(float  x11, float x12, float x21, float x22, float& r1, float& r2);
void sub_moments(float  x11, float  x12, float x21, float x22, float& r1, float& r2);

} /* namespace recsys */

#endif /* HHMFBIAS_H_ */
