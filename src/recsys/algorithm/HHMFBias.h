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
