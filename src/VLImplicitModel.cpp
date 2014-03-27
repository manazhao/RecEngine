/*
 * VLImplicitModel.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: qzhao2
 */

#include "VLImplicitModel.h"

VLImplicitModel::VLImplicitModel(DataConfig const& dataConfig, ModelParam const& modelParams):MFImplicitModel(dataConfig,modelParams) {
	// TODO Auto-generated constructor stub

}

VLImplicitModel::~VLImplicitModel() {
	// TODO Auto-generated destructor stub
}


void VLImplicitModel::_initParam(){
	/*
	 * initialize model parameters which include,
	 * 1) rating variance
	 * 2) global rating bias
	 * 3) user and prior
	 * 4) feature bias of the logistic regression for the implicit feedback
	 */
	m_ratingVar = 1; // rating variance as 1
	m_ratingBias = _averageTrainRating(); // global rating bias
	/// initialize the feature bias as 0
	for(PtrFeedbackVectorMap::iterator iter = m_featureUserMap.begin(); iter != m_featureUserMap.end(); ++iter){
		m_featUserBiasMap[iter->first] = 0;
	}
	///
	for(PtrFeedbackVectorMap::iterator iter = m_featureItemMap.begin(); iter != m_featureItemMap.end(); ++iter){
		m_featItemBiasMap[iter->first] = 0;
	}

	/// initialize the contact point as 0
	for(PtrFeedbackVectorMap::iterator iter = m_userFeatureMap.begin(); iter != m_userFeatureMap.end(); ++iter){
		PtrFeedbackVector& featVec = iter->second;
		for(PtrFeedbackVectorIterator iter1 = featVec.begin(); iter1 != featVec.end(); ++iter1){
			Feedback& tmpFb = **iter1;
			m_userFeatContMap[(string)tmpFb] = 1e-5;
		}
	}
	for(PtrFeedbackVectorMap::iterator iter = m_itemFeatureMap.begin(); iter != m_itemFeatureMap.end(); ++iter){
		PtrFeedbackVector& featVec = iter->second;
		for(PtrFeedbackVectorIterator iter1 = featVec.begin(); iter1 != featVec.end(); ++iter1){
			Feedback& tmpFb = **iter1;
			m_itemFeatContMap[(string)tmpFb] = 1e-5;
		}
	}
	/// initialize user prior
	colvec avgUserPosMean(m_modelParams.m_latentDim, fill::zeros);
	for (UserObjectMap::iterator iter = m_userProfileMap.begin(); iter != m_userProfileMap.end();
			++iter) {
		avgUserPosMean += iter->second.m_posMean;
	}
	avgUserPosMean /= m_userProfileMap.size();
	for (UserObjectMap::iterator iter = m_userProfileMap.begin(); iter != m_userProfileMap.end();
			++iter) {
		iter->second.m_priorMean = avgUserPosMean;
	}
	// init item latent prior mean as the mean of item posterior means
	colvec avgItemPosMean(m_modelParams.m_latentDim, fill::zeros);
	for (ItemObjectMap::iterator iter = m_itemProfileMap.begin(); iter != m_itemProfileMap.end();
			++iter) {
		avgItemPosMean += iter->second.m_posMean;
	}
	avgItemPosMean /= m_itemProfileMap.size();
	for (ItemObjectMap::iterator iter = m_itemProfileMap.begin(); iter != m_itemProfileMap.end();
			++iter) {
		iter->second.m_priorMean = avgItemPosMean;
	}

	// init feature latent prior mean as the mean of item posterior means
	colvec avgFeaturePosMean(m_modelParams.m_latentDim, fill::zeros);
	for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin(); iter != m_featureProfileMap.end();
			++iter) {
		avgFeaturePosMean += iter->second.m_posMean;
	}
	avgFeaturePosMean /= m_featureProfileMap.size();
	for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin(); iter != m_featureProfileMap.end();
			++iter) {
		iter->second.m_priorMean = avgFeaturePosMean;
	}

}

void VLImplicitModel::_updateUserByRating(UserObject& userObject,Feedback const& fb){
	float y = fb.m_val;
	ItemObject const& item = m_itemProfileMap[fb.m_toId];
	userObject.m_posCovInv += (1/m_ratingVar * (item.m_posMean * item.m_posMean.t() + (m_modelParams.m_isCovDiag ? diagmat(item.m_posCov) : item.m_posCov)));
	userObject.m_posMeanInv += ((y-m_ratingBias)/m_ratingVar) * item.m_posMean;
}

void VLImplicitModel::_updateUserByRatings(UserObject& userObject, PtrFeedbackVector const& userRatings) {
	/**
	 * the inverse of the covariance matrix is updated upon each rating according to:
	 * $\Sigma_u^{-1}' = \Sigma_u^{-1} + \frac{\mu_i\mu_i^T + \Sigma_i}{\sigma^2}
	 * where $\mu_i, \Sigma_i, \sigma^2$ are item's covariance matrix, mean and the rating variance
	 */
	for(PtrFeedbackVector::const_iterator iter = userRatings.begin(); iter != userRatings.end(); ++iter){
		Feedback const& tmpFb = **iter;
		_updateUserByRating(userObject, tmpFb);
	}
}


void VLImplicitModel::_updateUserByFeature(UserObject& userObject, Feedback const& fb) {
	if(m_featureProfileMap.find(fb.m_toId) == m_featureProfileMap.end())
		return;
	float y = fb.m_val;
	FeatureObject const& feat = m_featureProfileMap[fb.m_toId];
	float xi = m_userFeatContMap[(string)fb];
	float bk = m_featUserBiasMap[fb.m_toId];
	float lambda = _lambdaXi(xi);
	userObject.m_posCovInv += 2 * lambda * (feat.m_posMean * feat.m_posMean.t() + (m_modelParams.m_isCovDiag?diagmat(feat.m_posCov):feat.m_posCov));
	userObject.m_posMeanInv += (y - 0.5 - 2 * lambda * bk) * feat.m_posMean;
}

void VLImplicitModel::_updateUserByFeatures(UserObject& userObject, PtrFeedbackVector const& userFeatures) {
	for(PtrFeedbackVector::const_iterator iter = userFeatures.begin(); iter != userFeatures.end(); ++iter){
		Feedback const& tmpFb = **iter;
		_updateUserByFeature(userObject, tmpFb);
	}
}

void VLImplicitModel::_updateItemByRating(ItemObject& itemObject, Feedback const& fb) {
	float y = fb.m_val;
	UserObject const& user = m_userProfileMap[fb.m_fromId];
	itemObject.m_posCovInv += 1/m_ratingVar * (user.m_posMean * user.m_posMean.t() + (m_modelParams.m_isCovDiag ? diagmat(user.m_posCov) : user.m_posCov));
	itemObject.m_posMeanInv += ((y-m_ratingBias)/m_ratingVar) * user.m_posMean;
}

void VLImplicitModel::_updateItemByRatings(ItemObject& itemObject, PtrFeedbackVector const& itemRatings) {
	for(PtrFeedbackVector::const_iterator iter = itemRatings.begin(); iter != itemRatings.end(); ++iter){
		Feedback& tmpFb = **iter;
		_updateItemByRating(itemObject, tmpFb);
	}
}

void VLImplicitModel::_updateItemByFeature(ItemObject& itemObject,  Feedback const& fb) {
	if(m_featureProfileMap.find(fb.m_toId) == m_featureProfileMap.end())
		return;
	float y = fb.m_val;
	FeatureObject const& feat = m_featureProfileMap[fb.m_toId];
	float xi = m_itemFeatContMap[(string)fb];
	float bk = m_featItemBiasMap[fb.m_toId];
	float lambda = _lambdaXi(xi);
	itemObject.m_posCovInv += 2 * lambda * (feat.m_posMean * feat.m_posMean.t() + (m_modelParams.m_isCovDiag?diagmat(feat.m_posCov):feat.m_posCov));
	itemObject.m_posMeanInv += (y - 0.5 - 2 * lambda * bk) * feat.m_posMean;
}

void VLImplicitModel::_updateItemByFeatures(ItemObject& itemObject, PtrFeedbackVector const& itemFeatures) {
	for(PtrFeedbackVector::const_iterator iter = itemFeatures.begin(); iter != itemFeatures.end(); ++iter){
		Feedback & tmpFb = **iter;
		_updateItemByFeature(itemObject, tmpFb);
	}
}

void VLImplicitModel::_updateFeatureByUser(FeatureObject& featureObject, Feedback const& fb){
	float y = fb.m_val;
	UserObject const& user = m_userProfileMap[fb.m_fromId];
	float xi = m_userFeatContMap[(string)fb];
	float lambda = _lambdaXi(xi);
	float bk = m_featUserBiasMap[featureObject.m_id];
	featureObject.m_posCovInv += 2 * lambda * (user.m_posMean * user.m_posMean.t() + (m_modelParams.m_isCovDiag?diagmat(user.m_posCov):user.m_posCov));
	featureObject.m_posMeanInv += (y - 0.5 - 2 * lambda * bk) * user.m_posMean;
}

void VLImplicitModel::_updateFeatureByUsers(FeatureObject& featureObject, PtrFeedbackVector const& fb) {
	PtrFeedbackVector const& featVec = m_featureUserMap[featureObject.m_id];
	for(PtrFeedbackVector::const_iterator iter = featVec.begin(); iter != featVec.end(); ++iter){
		/// check whether the user exists
		Feedback const& fb = **iter;
		if(m_userProfileMap.find(fb.m_fromId) == m_userProfileMap.end())
			continue;
		_updateFeatureByUser(featureObject,fb);
	}
}

void VLImplicitModel::_updateFeatureByItem(FeatureObject& featureObject, Feedback const& fb){
	float y = fb.m_val;
	ItemObject const& item = m_itemProfileMap[fb.m_fromId];
	float xi = m_itemFeatContMap[(string)fb];
	float lambda = _lambdaXi(xi);
	float bk = m_featItemBiasMap[featureObject.m_id];
	featureObject.m_posCovInv += 2 * lambda * (item.m_posMean * item.m_posMean.t() + (m_modelParams.m_isCovDiag?diagmat(item.m_posCov):item.m_posCov));
	featureObject.m_posMeanInv += (y - 0.5 - 2 * lambda * bk) * item.m_posMean;
}

void VLImplicitModel::_updateFeatureByItems(FeatureObject& featureObject, PtrFeedbackVector const& fb) {
	PtrFeedbackVector const& featVec = m_featureItemMap[featureObject.m_id];
	for(PtrFeedbackVector::const_iterator iter = featVec.begin(); iter != featVec.end(); ++iter){
		/// check whether the user exists
		Feedback const& fb = **iter;
		if(m_itemProfileMap.find(fb.m_fromId) == m_itemProfileMap.end())
			continue;
		_updateFeatureByItem(featureObject,fb);
	}
}

void VLImplicitModel::_updateFeatUserBias(){
	for(PtrFeedbackVectorMap::iterator iter = m_featureUserMap.begin(); iter != m_featureUserMap.end(); ++iter){
		size_t featId = iter->first;
		PtrFeedbackVector const& featUsers = iter->second;
		FeatureObject const& featObject = m_featureProfileMap[featId];
		float denom = 1e-10;
		float factor = 1e-10;
		for(PtrFeedbackVector::const_iterator iter1 = featUsers.begin(); iter1 != featUsers.end(); ++iter1){
			Feedback const& fb = **iter1;
			float y = fb.m_val;
			if(m_userProfileMap.find(fb.m_fromId) == m_userProfileMap.end())
				continue;
			UserObject const& userObject = m_userProfileMap[fb.m_fromId];
			float xi = m_userFeatContMap[(string)fb];
			float lambda = _lambdaXi(xi);
			denom += lambda;
			factor += (y-0.5 + 2 * lambda * (as_scalar(userObject.m_posMean.t() * featObject.m_posMean)));
		}
		m_featUserBiasMap[featId] = factor/(2*denom);
	}
}

void VLImplicitModel::_updateFeatItemBias(){
	for(PtrFeedbackVectorMap::iterator iter = m_featureItemMap.begin(); iter != m_featureItemMap.end(); ++iter){
		size_t featId = iter->first;
		PtrFeedbackVector const featItems = iter->second;
		FeatureObject const& featObject = m_featureProfileMap[featId];
		float denom = 0;
		float factor = 0;
		for(PtrFeedbackVector::const_iterator iter1 = featItems.begin(); iter1 != featItems.end(); ++iter1){
			Feedback const& fb = **iter1;
			float y = fb.m_val;
			if(m_itemProfileMap.find(fb.m_fromId) == m_itemProfileMap.end())
				continue;
			ItemObject const& itemObject = m_itemProfileMap[fb.m_fromId];
			float xi = m_itemFeatContMap[(string)fb];
			float lambda = _lambdaXi(xi);
			denom += lambda;
			factor += (y-0.5 + 2 * lambda * (as_scalar(itemObject.m_posMean.t() * featObject.m_posMean)));
		}
		m_featItemBiasMap[featId] = factor/(2*denom);
	}
}

void VLImplicitModel::_updateUserFeatCont(PtrFeedbackVectorMap const& userFeatureMap ){
	PtrFeedbackVectorMap const*  userFeatureMapPtr = &userFeatureMap;
	if(userFeatureMapPtr->empty()){
		userFeatureMapPtr = &m_userFeatureMap;
	}
	for(PtrFeedbackVectorMap::const_iterator iter = userFeatureMapPtr->begin(); iter != userFeatureMapPtr->end(); ++iter){
		if(m_userProfileMap.find(iter->first) == m_userProfileMap.end())
			continue;
		UserObject const& userObject = m_userProfileMap[iter->first];
		PtrFeedbackVector const& userFeats = iter->second;
		mat const& userCov = userObject.m_posCov;
		colvec const& userMean = userObject.m_posMean;
		for(PtrFeedbackVector::const_iterator iter1 = userFeats.begin(); iter1 < userFeats.end(); ++iter1){
			Feedback const& fb = **iter1;
			float userFeatBias = m_featUserBiasMap[fb.m_toId];
			FeatureObject const& featObject = m_featureProfileMap[fb.m_toId];
			mat const& featCov = featObject.m_posCov;
			colvec const& featMean = featObject.m_posMean;
			float prod1 = as_scalar(userMean.t() * featMean);
			float a2Mean = accu(featCov % userCov) + prod1 * prod1;
			if(m_modelParams.m_isCovDiag){
				a2Mean += (accu(userMean % featCov % userMean));
				a2Mean += (accu(featMean % userCov % featMean));
			}else{
				a2Mean += as_scalar(userMean.t() * featCov * userMean);
				a2Mean += as_scalar(featMean.t() * userCov * featMean);
			}
			m_userFeatContMap[(string)fb] = sqrt(a2Mean + 2 * userFeatBias * prod1 + userFeatBias * userFeatBias);
		}
	}
}

void VLImplicitModel::_updateItemFeatCont(PtrFeedbackVectorMap const& itemFeatureMap ){
	PtrFeedbackVectorMap const*  itemFeatureMapPtr = &itemFeatureMap;
	if(itemFeatureMapPtr->empty()){
		itemFeatureMapPtr = &m_itemFeatureMap;
	}

	for(PtrFeedbackVectorMap::const_iterator iter = itemFeatureMapPtr->begin(); iter != itemFeatureMapPtr->end(); ++iter){
		if(m_itemProfileMap.find(iter->first) == m_itemProfileMap.end())
			continue;
		ItemObject const& itemObject = m_itemProfileMap[iter->first];
		PtrFeedbackVector const& itemFeats = iter->second;
		mat const& itemCov = itemObject.m_posCov;
		colvec const& itemMean = itemObject.m_posMean;
		for(PtrFeedbackVector::const_iterator iter1 = itemFeats.begin(); iter1 < itemFeats.end(); ++iter1){
			Feedback const& fb = **iter1;
			float itemFeatBias = m_featItemBiasMap[fb.m_toId];
			FeatureObject const& featObject = m_featureProfileMap[fb.m_toId];
			mat const& featCov = featObject.m_posCov;
			colvec const& featMean = featObject.m_posMean;
			float prod1 = as_scalar(itemMean.t() * featMean);
			float a2Mean = accu(featCov % itemCov) + prod1 * prod1;
			if(m_modelParams.m_isCovDiag){
				a2Mean += (accu(itemMean % featCov % itemMean));
				a2Mean += (accu(featMean % itemCov % featMean));
			}else{
				a2Mean += as_scalar(itemMean.t() * featCov * itemMean);
				a2Mean += as_scalar(featMean.t() * itemCov * featMean);
			}
			m_itemFeatContMap[(string)fb] = sqrt(a2Mean + 2 * itemFeatBias * prod1 + itemFeatBias * itemFeatBias);
		}
	}
}

void VLImplicitModel::_updateNewUserOnFeats(UserObject& userObject, PtrFeedbackVector const& userFeats){
	map<unsigned int,PtrFeedbackVector> userFeatMap;
	userFeatMap[userObject.m_id] = userFeats;
	UserObject origUserObject = userObject;
//	cout << "----------user feature contact point for user:" << userObject.m_id << endl;
	for(int i = 0; i < 4; i++){
		userObject = origUserObject;
		userObject.posInv(false);
		_updateUserByFeatures(userObject,userFeats);
		///dump the feature contact point parameter values
		userObject.posInv(true);
		///update the variational logistic regression parameters
		_updateUserFeatCont(userFeatMap);
	}
}


void VLImplicitModel::_updateFeaturePrior() {
	colvec priorMean(m_modelParams.m_latentDim, fill::zeros);
	for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin(); iter != m_featureProfileMap.end();
			++iter) {
		FeatureObject& fObject = iter->second;
		colvec featPosMean = fObject.m_posMean;
		priorMean += featPosMean;
	}
	priorMean /= m_featureProfileMap.size();
	mat priorCov(m_modelParams.m_latentDim, 1, fill::zeros);
	for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin(); iter != m_featureProfileMap.end();
			++iter) {
		FeatureObject& fObject = iter->second;
		colvec featPosMean = fObject.m_posMean;
		mat& featPosCov = fObject.m_posCov;
		// consider the fitting error
		colvec diff = featPosMean - priorMean;
		priorCov = priorCov + (diff % diff);
		if (m_modelParams.m_isCovDiag) {
			priorCov = priorCov + featPosCov;
		} else {
			priorCov = priorCov + diagvec(featPosCov);
		}
	}
	priorCov = priorCov / m_featureProfileMap.size();
	m_featurePriorMean = priorMean;
	m_featurePriorCov = priorCov;
	for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin(); iter != m_featureProfileMap.end();
			++iter) {
		FeatureObject& fObject = iter->second;
		fObject.m_priorMean = priorMean;
		fObject.m_priorCov = priorCov;
	}
}


void VLImplicitModel::optimize(){
	cout << "initialize model parameters..." << endl;
	_initParam();
	cout << "\n\nstart EM with " << m_modelParams.m_maxIter << " iterations"
			<< endl;
	for (unsigned int emIter = 0; emIter < m_modelParams.m_maxIter; emIter++) {
		/// update each user
		cout << "----------------------------iteration:" << emIter
				<< "----------------------------" << endl;
		cout << "update user profile..." << endl;
		for (UserObjectMap::iterator iter = m_userProfileMap.begin();
				iter != m_userProfileMap.end(); ++iter) {
			UserObject& user = iter->second;
			user.m_posMean = user.m_priorMean;
			user.m_posCov = user.m_priorCov;
			user.posInv(false);
			PtrFeedbackVector const& userRatings = m_userRatingMap[iter->first];
			_updateUserByRatings(user,userRatings);
			PtrFeedbackVector const& userFeatures = m_userFeatureMap[iter->first];
			_updateUserByFeatures(user,userFeatures);
			user.posInv(true);
		}
		/// update each item
		cout << "update item profile..." << endl;
		for (ItemObjectMap::iterator iter = m_itemProfileMap.begin();
				iter != m_itemProfileMap.end(); ++iter) {
			ItemObject& item = iter->second;
			item.m_posMean = item.m_priorMean;
			item.m_posCov = item.m_priorCov;
			item.posInv(false);
			PtrFeedbackVector const& itemRatings = m_itemRatingMap[iter->first];
			_updateItemByRatings(item,itemRatings);
			PtrFeedbackVector const& itemFeatures = m_itemFeatureMap[iter->first];
			_updateItemByFeatures(item,itemFeatures);
			item.posInv(true);
		}

		cout << "update feature profile..." << endl;
		/// update each feature
		for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin();
				iter != m_featureProfileMap.end(); ++iter) {
			FeatureObject & featureObject = iter->second;
			featureObject.m_posMean = featureObject.m_priorMean;
			featureObject.m_posCov = featureObject.m_priorCov;
			featureObject.posInv(false);
//			featureObject.reset();
			if(m_featureUserMap.find(iter->first) != m_featureUserMap.end()){
				PtrFeedbackVector const& featureUsers = m_featureUserMap[iter->first];
				_updateFeatureByUsers(featureObject,featureUsers);
			}
			if(m_featureItemMap.find(iter->first) != m_featureItemMap.end()){
				PtrFeedbackVector const& featureItems = m_featureItemMap[iter->first];
				_updateFeatureByItems(featureObject,featureItems);
			}
			featureObject.posInv(true);
		}
		/// update model parameters, start with global bias
		_updateRatingBias();
		cout << "update rating variance..." << endl;
		_updateRatingVariance();
		///
//		cout << "update feature user bias..." << endl;
		_updateFeatUserBias();
//		cout << "update feature item bias..." << endl;
		_updateFeatItemBias();
		cout << "update user feature contact point..." << endl;
		_updateUserFeatCont();
		cout << "update item feature contact point..." << endl;
		_updateItemFeatCont();
		cout << "update user prior..." << endl;
		/// update user prior
		_updateUserPrior();
		cout << "update item prior..." << endl;
		/// update item prior
		_updateItemPrior();
		cout << "update feature prior..." << endl;
		_updateFeaturePrior();
		/// evaluate the training RMSE
		float iterRMSE = _trainRMSE();
		cout << "iter:" << emIter << ",training RMSE:" << iterRMSE << endl;
	}
	_dumpIFFit();
}
ostream& operator<<(ostream& oss, FeatureScore const& rhs){
	oss << rhs.m_featureIndex << ":" << rhs.m_score << endl;
	return oss;
}

void VLImplicitModel::topKFeatures(unsigned int k){
	vector<FeatureScore> featureScores;
	for(FeatureObjectMap::iterator iter = m_featureProfileMap.begin(); iter != m_featureProfileMap.end(); ++iter){
		FeatureObject& fObject = iter->second;
		featureScores.push_back(FeatureScore(fObject.m_id,as_scalar(arma::mean(fObject.m_posCov))));
	}
	std::sort(featureScores.begin(),featureScores.end());
	m_topFeatureSet.clear();
	for(unsigned int i = 0; i < k; i++){
		m_topFeatureSet.insert(featureScores[i].m_featureIndex);
	}
}

float VLImplicitModel::testRMSE(bool useFeature){
	float rmse = 0;
	size_t numRatings = 0;
//	topKFeatures(12);
	for(vector<size_t>::iterator iter = m_test_samples.begin(); iter < m_test_samples.end(); ++iter){
		size_t sampleIdx = (*iter) - 1;
		Feedback const& fb = *(m_ratings[sampleIdx]);
		size_t userId = fb.m_fromId;
		size_t itemId = fb.m_toId;
		/// skip new items
		if(m_itemProfileMap.find(itemId) == m_itemProfileMap.end())
			continue;
		if(m_userProfileMap.find(userId) == m_userProfileMap.end()){
			/// a new user
			m_userProfileMap[userId] = UserObject(userId,m_modelParams.m_latentDim,m_modelParams.m_isCovDiag);
			UserObject& uObject = m_userProfileMap[userId];
			_userProfileOnPrior(uObject);
			if(useFeature){
				/// get the inverse covariance and the corresponding mean
				/// get user features
				if(m_userFeatureMap.find(userId) != m_userFeatureMap.end()){
					PtrFeedbackVector& uFeats = m_userFeatureMap[userId];
					_selectTopKFeature(uFeats);
					_updateNewUserOnFeats(uObject,uFeats);
				}
			}
		}
		float rating = fb.m_val;
		UserObject& userObject = m_userProfileMap[userId];
		ItemObject& itemObject = m_itemProfileMap[itemId];
		float predRating = as_scalar(
				userObject.m_posMean.t() * itemObject.m_posMean);
		float diff = rating - predRating - m_ratingBias;
		rmse += (diff * diff);
		numRatings++;
	}
	rmse = sqrtf(rmse/numRatings);
	cout << "testing RMSE:" << rmse << endl;
	return rmse;
}


