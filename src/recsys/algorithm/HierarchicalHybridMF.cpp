/*
 * HierarchicalHybridMF.cpp
 *
 *  Created on: Apr 2, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/timer.hpp>
#include <algorithm>
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>

namespace recsys {

void HierarchicalHybridMF::_init() {
	/// first prepare the datasets
	_prepare_datasets();
	/// allocate space for model variables
	_prepare_model_variables();
	/// we are all set, embark the fun journey!
}

void HierarchicalHybridMF::_prepare_model_variables() {
	cout << "############## initialize model variables ##############" << endl;
	timer t;
	/// initialize entity latent variables
	m_entity.assign(m_dataset.ent_type_interacts.size(),DiagMVGaussian(vec(m_lat_dim,arma::fill::randn),vec(m_lat_dim,arma::fill::ones)));
	/// initialize prior variables
	m_user_prior_mean = DiagMVGaussian(vec(m_lat_dim,fill::zeros),vec(m_lat_dim,fill::ones));
	m_user_prior_cov = MVInverseGamma(vec(m_lat_dim,fill::ones),vec(m_lat_dim,fill::ones));
	m_item_prior_mean = m_user_prior_mean;
	m_item_prior_cov = m_user_prior_cov;
	m_feature_prior_mean = m_user_prior_mean;
	m_feature_prior_cov = m_user_prior_cov;
	/// rating variance, big variance
	m_rating_var = InverseGamma(1,1);
	/// initialize bias as standard Gaussian
	m_bias = Gaussian(0,1);
	cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed() << " >>>>>>>>>>>>>>"
			<< endl;

}

void HierarchicalHybridMF::_update_user(int64_t const& entityId, map<int8_t,vector<Interact> > const& typeInteracts){
	/// first reset the natural parameter of user vector
	m_entity[entityId].reset();
	/// update with rating feedback
	vector<Interact> const& ratingInteracts = typeInteracts[EntityInteraction::RATE_ITEM];
	/// sufficient satitstics for logx and 1/x
	NatParamVec rvsuff1 = m_rating_var.suff_mean(1);
	NatParamVec rvsuff2 = m_rating_var.suff_mean(2);
	/// global bias
	for(vector<Ineract>::const_iterator iter = ratingInteracts.begin(); iter < ratingInteracts.end(); ++iter){
		Interact const& tmpInteract = *iter;
		float tmpRating = tmpInteract.ent_val;
		int64_t itemId = tmpInteract.ent_id;
		DiagMVGaussian const& itemLat = m_entity[itemId];
		NatParamVec tmpMessage(2 * m_lat_dim);
		float tmpRating1 = tmpRating - m_bias.moment(1);
		tmpMessage.m_vec.cols(0,m_lat_dim - 1) = tmpRating1 * rvsuff2 * itemLat.moment(1);
		tmpMessage.m_vec.cols(m_lat_dim, 2 * m_lat_dim - 1) = -0.5 * rvsuff2 * itemLat.moment(2);
		/// apply the update
		m_entity[entityId] += tmpMessage;
	}
	/// update from the prior
	vector<Interact> const& featureInteracts = typeInteracts[EntityInteraction::ADD_FEATURE];
	NatParamVec upCovSuff1 = m_user_prior_cov.suff_mean(1);
	NatParamVec upCovSuff2 = m_user_prior_cov.suff_mean(2);
	/// user prior mean
	NatParamVec upMean = m_user_prior_mean.moment(1);
	size_t numFeats = featureInteracts.size();
	NatParamVec tmpMessage(2 * m_lat_dim);
	tmpMessage.m_vec.cols(0,m_lat_dim -1) = upMean.m_vec;
	for(vector<Interact>::const_iterator iter = featureInteracts.begin(); iter < featureInteracts.end(); ++iter){
		int64_t featId = iter->ent_id;
		DiagMVGaussian const& featLat = m_entity[featId];
		tmpMessage.m_vec.cols(0, m_lat_dim -1) += 1/numFeats * featLat.moment(1).m_vec;
	}
	tmpMessage.m_vec(0,m_lat_dim - 1) %= upCovSuff2.m_vec;
	tmpMessage.m_vec(m_lat_dim,2 * m_lat_dim -1) = upCovSuff2.m_vec;
	/// apply the update
	m_entity[entityId] += tmpMessage;
}

void HierarchicalHybridMF::_update_item(int64_t const& entityId, map<int8_t,vector<Interact> > const& typeInteracts){
	/// first reset the natural parameter of user vector
	m_entity[entityId].reset();
	/// update with rating feedback
	vector<Ineract> const& ratingInteracts = typeInteracts[EntityInteraction::RATE_ITEM];
	/// sufficient satitstics for logx and 1/x
	NatParamVec rvsuff1 = m_rating_var.suff_mean(1);
	NatParamVec rvsuff2 = m_rating_var.suff_mean(2);
	/// global bias
	for(vector<Ineract>::const_iterator iter = ratingInteracts.begin(); iter < ratingInteracts.end(); ++iter){
		Interact const& tmpInteract = *iter;
		float tmpRating = tmpInteract.ent_val;
		int64_t userId = tmpInteract.ent_id;
		DiagMVGaussian const& itemLat = m_entity[userId];
		NatParamVec tmpMessage(2 * m_lat_dim);
		float tmpRating1 = tmpRating - m_bias.moment(1);
		tmpMessage.m_vec.cols(0,m_lat_dim - 1) = tmpRating1 * rvsuff2 * itemLat.moment(1);
		tmpMessage.m_vec.cols(m_lat_dim, 2 * m_lat_dim - 1) = -0.5 * rvsuff2 * itemLat.moment(2);
		/// apply the update
		m_entity[entityId] += tmpMessage;
	}
	/// update from the prior
	vector<Interact> const& featureInteracts = typeInteracts[EntityInteraction::ADD_FEATURE];
	NatParamVec upCovSuff1 = m_user_prior_cov.suff_mean(1);
	NatParamVec upCovSuff2 = m_user_prior_cov.suff_mean(2);
	/// user prior mean
	NatParamVec upMean = m_user_prior_mean.moment(1);
	size_t numFeats = featureInteracts.size();
	NatParamVec tmpMessage(2 * m_lat_dim);
	tmpMessage.m_vec.cols(0,m_lat_dim -1) = upMean.m_vec;
	for(vector<Interact>::const_iterator iter = featureInteracts.begin(); iter < featureInteracts.end(); ++iter){
		int64_t featId = iter->ent_id;
		DiagMVGaussian const& featLat = m_entity[featId];
		tmpMessage.m_vec.cols(0, m_lat_dim -1) += 1/numFeats * featLat.moment(1).m_vec;
	}
	tmpMessage.m_vec(0,m_lat_dim - 1) %= upCovSuff2.m_vec;
	tmpMessage.m_vec(m_lat_dim,2 * m_lat_dim -1) = upCovSuff2.m_vec;
	/// apply the update
	m_entity[entityId] += tmpMessage;
}

void HierarchicalHybridMF::_update_diff(){
	/// update the user/item prior mean
	vector<int64_t> const& userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	vector<int64_t> const& itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	vector<int64_t> mergedIds;
	mergedIds.insert(mergedIds.end(),userIds.begin(),userIds.end());
	mergedIds.insert(mergedIds.begin(),itemIds.begin(),itemIds.end());
	for(vector<int64_t>::const_iterator iter = mergedIds.begin(); iter < mergedIds.end(); ++iter){
		m_feat_sum[*iter] = vec(m_lat_dim,fill::zeros);
		vector<Interact> const& featureInteracts = m_train_dataset.ent_type_interacts[*iter][EntityInteraction::ADD_FEATURE];
		for(vector<Interact>::const_iterator iter1 = featureInteracts.begin(); iter1 != featureInteracts.end(); ++iter1){
			int64_t featId = iter1->ent_id;
			m_feat_sum[*iter] += m_entity[featId].moment(1);
		}
	}
}

void HierarchicalHybridMF::_update_feature(int64_t const& featId, map<int8_t,vector<Interact> > const& typeInteracts){
	vector<Interact> const& interacts = typeInteracts[EntityInteraction::ADD_FEATURE];
	NatParamVec updateMessage(2 * m_lat_dim);
	NatParamVec featLatMean = m_entity[featId].moment(1);
	float tmp1 = 0;
	for(vector<Interact>::const_iterator iter = interacts.begin(); iter < interacts.end(); ++iter){
		int64_t entityId = iter->ent_id;
		vector<Interact> const& entityInteracts = m_train_dataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
		size_t numFeats = entityInteracts.size();
		DiagMVGaussian const& entityLat = m_entity[entityId];
		tmp1 += 1/numFeats;
		///
		vec const& tmpDiff = entityLat.moment(1) -  m_feat_sum[entityId] + featLatMean;
		updateMessage.m_vec.cols(0,m_lat_dim) = 1/sqrt(numFeats) * tmpDiff;
	}
	NatParamVec upCovSuff2 = m_user_prior_cov.suff_mean(2);
	updateMessage.m_vec.cols(0,m_lat_dim) %= (upCovSuff2.m_vec);
	updateMessage.m_vec.cols(m_lat_dim, 2 * m_lat_dim - 1)  = -0.5 * tmp1 * upCovSuff2.m_vec;
	/// apply the update
	m_entity[featId] = updateMessage;
}

void HierarchicalHybridMF::_update_user_prior(){
	/// update the mean of the prior
	m_user_prior_mean.reset();
	NatParamVec updateMessage(2 * m_lat_dim);
	vecotr<int64_t> const& userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	NatParamVec upCovSuff2 = m_user_prior_cov.suff_mean(2);
	for(vector<int64_t>::const_iterator iter = userIds.begin(); iter < userIds.end(); ++iter){
		int64_t userId = *iter;
		vector<EntityInteraction> const& featInterats = m_train_dataset.ent_type_interacts[userId][EntityInteraction::ADD_FEATURE];
		size_t numFeats = featInterats.size();
		DiagMVGaussian const& userLat = m_entity[userId];
		updateMessage.m_vec.cols(0,m_lat_dim - 1) = (userLat.moment(1) - m_feat_sum[userId]);
	}
	updateMessage.m_vec.cols(0,m_lat_dim - 1) %= upCovSuff2.m_vec;
	updateMessage.m_vec.cols(m_lat_dim, 2 * m_lat_dim - 1) = (-0.5 * upCovSuff2);
	/// update the diagonal covariance matrix, each entry of which is InverseGamma distribution


}

void HierarchicalHybridMF::_update_item_prior(){

}

void HierarchicalHybridMF::_update_feature_prior(){

}

void HierarchicalHybridMF::_update_rating_var(){

}

void HierarchicalHybridMF::_update_bias(){

}

void HierarchicalHybridMF::train_model(){
	/// train Bayesian model on the training dataset
	const size_t maxIter = 20;
	set<int64_t> const& userIds = m_train_dataset.type_ent_ids[Entity::ENT_USER];
	set<int64_t> const& itemIds = m_train_dataset.type_ent_ids[Entity::ENT_ITEM];
	set<int64_t> const& featureIds = m_train_dataset.type_ent_ids[Entity::ENT_FEATURE];
	vector<map<int8_t, vector<Interact> > >const& type_interacts = m_train_dataset.ent_type_interacts;
	typedef set<int64_t>::const_iterator id_set_const_iter;
	////
	_update_diff();
	for(size_t iter = 0; iter < maxIter; iter++){
		/// update user entities
		for(id_set_const_iter iter = userIds.begin(); iter != userIds.end(); ++iter){
			size64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > const& tmpEntityInteracts = type_interacts[entityId];
			_update_user(entityId, tmpEntityInteracts);
		}
		for(id_set_const_iter iter = itemIds.begin(); iter != itemIds.end(); ++iter){
			size64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > const& tmpEntityInteracts = type_interacts[entityId];
			_update_item(entityId, tmpEntityInteracts);
		}
		for(id_set_const_iter iter = featureIds.begin(); iter != featureIds.end(); ++iter){
			size64_t entityId = *iter;
			/// get user rating and feature interactions
			map<int8_t, vector<Interact> > const& tmpEntityInteracts = type_interacts[tmpUserId];
			_update_feature(entityId, tmpEntityInteracts);
		}
		/// update prior
		_update_user_prior();
		_update_item_prior();
		_update_feature_prior();
		_update_rating_var();
		_update_bias();
	}
}

void HierarchicalHybridMF::_prepare_datasets() {
	try {
		m_transport->open();
		cout << "############## retrieve datasets from data host  ##############" << endl;
		timer t;
		m_client.get_dataset(m_dataset,rt::DSType::DS_ALL);
		m_client.get_dataset(m_train_dataset,rt::DSType::DS_TRAIN);
		m_client.get_dataset(m_test_dataset,rt::DSType::DS_TEST);
		m_client.get_dataset(m_cs_dataset,rt::DSType::DS_CS);
		cout << ">>>>>>>>>>>>>> Time elapsed:" << t.elapsed() << " >>>>>>>>>>>>>>" << endl;
		m_transport->close();
	} catch (TException &tx) {
		printf("ERROR: %s\n", tx.what());
	}
}


HierarchicalHybridMF::HierarchicalHybridMF() :
		m_num_users(0), m_num_items(0), m_num_features(0), m_socket(
				new TSocket("localhost", 9090)), m_transport(
				new TBufferedTransport(m_socket)), m_protocol(
				new TBinaryProtocol(m_transport)), m_client(m_protocol),m_lat_dim(10) {
	_init();
}

HierarchicalHybridMF::~HierarchicalHybridMF() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
