/*
 * HierarchicalHybridMF.h
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#ifndef HIERARCHICALHYBRIDMF_H_
#define HIERARCHICALHYBRIDMF_H_
#include <armadillo>
using namespace arma;

namespace recsys {

/**
 * hierarchical hybrid model which uses implicit feedback as prior of
 * user/item latent vector.
 * As the Bayesian network framework is not ready yet, we do the implementation
 * in a rushing style.
 */
class HierarchicalHybridMF {
protected:
	mat m_user_lat_mean;
	mat m_user_lat_cov;
	mat m_item_lat_mean;
	mat m_item_lat_cov;
	mat m_feat_lat_mean;
	mat m_feat_lat_cov;
	float m_global_bias;
	/// rating variance
	float m_rating_var;
	size_t m_num_users;
	size_t m_num_items;
	size_t m_num_features;
protected:
	void _init_from_data_host();
public:
	HierarchicalHybridMF();
	virtual ~HierarchicalHybridMF();
};

} /* namespace recsys */

#endif /* HIERARCHICALHYBRIDMF_H_ */
