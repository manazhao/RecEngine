/*
 * HHMFBias.cpp
 *
 *  Created on: May 7, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HHMFBias.h>

namespace recsys {

HHMFBias::HHMFBias() {
	// TODO Auto-generated constructor stub

}

void HHMFBias::_init_training() {
	/// initialize model variables
	/// initialize parent class
	HierarchicalHybridMF::_init_training();
	/// initialize user and item bias terms
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		m_user_bias[*iter] = Gaussian(0, 1);
	}
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	for (set<int64_t>::const_iterator iter = itemIds.begin();
			iter != itemIds.end(); ++iter) {
		m_item_bias[*iter] = Gaussian(0, 1);
	}
	/// initialize prior for user and bias terms
	m_user_bias_mean_prior = Gaussian(0,1);
	m_user_bias_var_prior = InverseGamma(3,3);
	m_item_bias_mean_prior = Gaussian(0,1);
	m_item_bias_var_prior = InverseGamma(3,3);
	/// initialize user and item bias through alternative update
	_init_bias();
}

RecModel::TrainIterLog HHMFBias::_train_update(){

}

void HHMFBias::_init_user_bias(){
	set<int64_t>& userIds = m_active_dataset.type_ent_ids[Entity::ENT_USER];
	float averageUserBias = 0;
	for (set<int64_t>::const_iterator iter = userIds.begin();
			iter != userIds.end(); ++iter) {
		/// get rating interactions
		int64_t userId = *iter;
		vector<Interact>& ratingInteracts =
				m_active_dataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		float userBias = 0;
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
			float rating = iter1->ent_val;
			int64_t itemId = iter1->ent_id;
			float itemBias = m_item_bias[itemId].m_mean;
			userBias += (rating - m_global_bias.m_mean - itemBias);
		}
		if(ratingInteracts.size() > 0){
			userBias /= ratingInteracts.size();
		}
		/// update it
		m_user_bias[userId].m_mean = userBias;
		averageUserBias += userBias;
	}
	averageUserBias /= userIds.size();
	m_user_bias_mean_prior.m_mean = averageUserBias;
}

void HHMFBias::_init_item_bias(){
	/// update item bias
	set<int64_t>& itemIds = m_active_dataset.type_ent_ids[Entity::ENT_ITEM];
	float averageItemBias = 0;
	for (set<int64_t>::const_iterator iter = itemIds.begin();
			iter != itemIds.end(); ++iter) {
		/// get rating interactions
		int64_t itemId = *iter;
		vector<Interact>& ratingInteracts =
				m_active_dataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM];
		float itemBias = 0;
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
			float rating = iter1->ent_val;
			int64_t userId = iter1->ent_id;
			float userBias = m_user_bias[userId].m_mean;
			itemBias += (rating - m_global_bias.m_mean - userBias);
		}
		if(ratingInteracts.size() > 0){
			itemBias /= ratingInteracts.size();
		}else{
			cout << "item Id:" << itemId << endl;
		}
		/// update it
		m_item_bias[itemId] = Gaussian(itemBias, 1);
		averageItemBias += itemBias;
	}
	averageItemBias /= itemIds.size();
	m_item_bias_mean_prior.m_mean = averageItemBias;
}


void HHMFBias::_init_bias() {
	/// update global bias term
	cout << ">>> initialize bias terms through alternative updating" << endl;
	for (size_t iter_cnt = 0; iter_cnt < 5; iter_cnt++) {
		cout << "iteration: " << iter_cnt + 1;
		_init_global_bias();
		cout << ", global bias:" << m_global_bias.m_mean;
		_init_user_bias();
		cout << ", user bias mean:" << m_user_bias_mean_prior.m_mean;
		_init_item_bias();
		cout << ", item bias mean:" << m_item_bias_mean_prior.m_mean << endl;
	}
	///
}

HHMFBias::~HHMFBias() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
