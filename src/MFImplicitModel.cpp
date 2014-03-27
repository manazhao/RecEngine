/*
 * MFImplicitModel.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#include "MFImplicitModel.h"
#include "csv.h"
#include <math.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

MFImplicitModel::MFImplicitModel(DataConfig const& dataConfig, ModelParam const& modelParams) :
		m_ratingBias(0), m_ratingVar(1),m_dataConfig(dataConfig), m_modelParams(modelParams)

{
	/// try to create the result folder if not exists yet
	if(!boost::filesystem::exists(m_dataConfig.m_resultFolder)){
		cout << "result folder:" << m_dataConfig.m_resultFolder << " does not exist yet, try to create it" << endl;
		if(!boost::filesystem::create_directory(m_dataConfig.m_resultFolder)){
			cerr << "failed to create: " << m_dataConfig.m_resultFolder << ", please check the input" << endl;
			throw std::exception();
		}
	}else{
		cout << "WARNING: files under " << m_dataConfig.m_resultFolder << " might get overwritten!" << endl;
	}
}

void MFImplicitModel::init() {
	/// load data
	_loadRatings();
	if (m_modelParams.m_useFeature) {
		_loadUserFeatures();
		_loadItemFeatures();
	}
	///
	_loadTrainingSamples();
	_loadTestingSamples();
}

MFImplicitModel::~MFImplicitModel() {
	// TODO Auto-generated destructor stub
}

vector<size_t> MFImplicitModel::_loadIndex(string const& fileName){
	vector<size_t> indexVec;
	fstream fs;
	fs.open(fileName.c_str(),fstream::in);
	for(CSVIterator iter(fs,'\t'); iter != CSVIterator(); ++iter){
		CSVRow const& row = *iter;
		if(row.size() == 0)
			continue;
		size_t index = lexical_cast<size_t>(row[0]);
		indexVec.push_back(index);
	}
	return indexVec;
}

void MFImplicitModel::_loadTestingSamples(){
	m_test_samples.clear();
	m_test_samples = _loadIndex(m_dataConfig.m_testIdxFile);
}
void MFImplicitModel::_loadTrainingSamples(){
	/// load the training sample index
	m_train_samples = _loadIndex(m_dataConfig.m_trainIdxFile);
	for(size_t i = 0; i < m_train_samples.size(); i++){
		size_t tmpSmpIdx = m_train_samples[i] - 1;
		shared_ptr<Feedback> ratingPtr = m_ratings[tmpSmpIdx];
		size_t userId = ratingPtr->m_fromId;
		size_t itemId = ratingPtr->m_toId;
		m_userRatingMap[userId].push_back(ratingPtr);
		m_itemRatingMap[itemId].push_back(ratingPtr);
		//create UserObject and ItemObject if not exist
		if (m_userProfileMap.find(userId) == m_userProfileMap.end()) {
			m_userProfileMap[userId] = UserObject(userId, m_modelParams.m_latentDim,
					m_modelParams.m_isCovDiag);
			m_train_users.insert(userId);
			PtrFeedbackVector& userFeats = m_userFeatureMap[userId];
			for(PtrFeedbackVector::iterator iter = userFeats.begin(); iter < userFeats.end(); ++iter){
				Feedback& fb = **iter;
				if(m_featureProfileMap.find(fb.m_toId) == m_featureProfileMap.end()){
					m_featureProfileMap[fb.m_toId] = FeatureObject(fb.m_toId,m_modelParams.m_latentDim,
					m_modelParams.m_isCovDiag);
				}
			}
		}
		if (m_itemProfileMap.find(itemId) == m_itemProfileMap.end()) {
			m_itemProfileMap[itemId] = ItemObject(itemId, m_modelParams.m_latentDim,
					m_modelParams.m_isCovDiag);
			m_train_items.insert(itemId);
			PtrFeedbackVector& itemFeats = m_itemFeatureMap[itemId];
			for(PtrFeedbackVector::iterator iter = itemFeats.begin(); iter < itemFeats.end(); ++iter){
				Feedback& fb = **iter;
				if(m_featureProfileMap.find(fb.m_toId) == m_featureProfileMap.end()){
					m_featureProfileMap[fb.m_toId] = FeatureObject(fb.m_toId,m_modelParams.m_latentDim,
					m_modelParams.m_isCovDiag);
				}
			}

		}
	}
}

void MFImplicitModel::_loadRatings() {
	///
	fstream fs;
	cout << "load rating file:" << m_dataConfig.m_ratingFile << endl;
	fs.open(m_dataConfig.m_ratingFile.c_str(), fstream::in | ios::binary);
	for (CSVIterator iter(fs, '\t'); iter != CSVIterator(); ++iter) {
		CSVRow const& row = *iter;
		if (row.size() < 3)
			continue;
		unsigned int userId = lexical_cast<unsigned int>(row[0]);
		unsigned int itemId = lexical_cast<unsigned int>(row[1]);
		float val = boost::lexical_cast<float>(row[2]);
		shared_ptr<Feedback> ratingPtr(new Feedback(userId, itemId, val));
		m_ratings.push_back(ratingPtr);
	}
	fs.close();
}

void MFImplicitModel::_loadUserFeatures() {
	fstream fs;
	cout << "reading user features from file:" << m_dataConfig.m_userFeatureFile << endl;
	fs.open(m_dataConfig.m_userFeatureFile.c_str(), fstream::in);
	for (CSVIterator iter(fs, '\t'); iter != CSVIterator(); ++iter) {
		CSVRow const& row = *iter;
		if (row.size() < 3)
			continue;
		unsigned int userId = lexical_cast<unsigned int>(row[0]);
		unsigned int featId = lexical_cast<unsigned int>(row[1]);
		float val = boost::lexical_cast<float>(row[2]);
		shared_ptr<Feedback> ifPtr(new Feedback(userId, featId, val));
		m_userFeatureMap[userId].push_back(ifPtr);
		m_featureUserMap[featId].push_back(ifPtr);
//		if (m_featureProfileMap.find(featId) == m_featureProfileMap.end()) {
//			m_featureProfileMap[featId] = FeatureObject(featId,
//					m_modelParams.m_latentDim, m_modelParams.m_isCovDiag);
//		}
	}
	fs.close();
}

void MFImplicitModel::_loadItemFeatures() {
	fstream fs;
	cout << "reading item features from file:" << m_dataConfig.m_itemFeatureFile << endl;
	fs.open(m_dataConfig.m_itemFeatureFile.c_str(), fstream::in);
	for (CSVIterator iter(fs, '\t'); iter != CSVIterator(); ++iter) {
		CSVRow const& row = *iter;
		if (row.size() < 3)
			continue;
		unsigned int itemId = lexical_cast<unsigned int>(row[0]);
		unsigned int featId = lexical_cast<unsigned int>(row[1]);
		float val = boost::lexical_cast<float>(row[2]);
		shared_ptr<Feedback> ifPtr(new Feedback(itemId, featId, val));
		m_itemFeatureMap[itemId].push_back(ifPtr);
		m_featureItemMap[featId].push_back(ifPtr);
//		if (m_featureProfileMap.find(featId) == m_featureProfileMap.end()) {
//			m_featureProfileMap[featId] = FeatureObject(featId,
//					m_modelParams.m_latentDim, m_modelParams.m_isCovDiag);
//		}
	}
	fs.close();
}

float MFImplicitModel::_averageTrainRating(){
	float ratingBias = 0;
	for(vector<size_t>::iterator iter = m_train_samples.begin(); iter < m_train_samples.end(); iter++){
		size_t sampleIdx = (*iter) - 1;
		float rating = m_ratings[sampleIdx]->m_val;
		ratingBias += rating;
	}
	ratingBias /= m_train_samples.size();
	return ratingBias;
}

void MFImplicitModel::_initParam() {
	/// get the global bias
	m_ratingBias = _averageTrainRating();
	m_ratingVar = 1;
	// init user feature variance as 1
	for (PtrFeedbackVectorMap::iterator iter = m_featureUserMap.begin();
			iter != m_featureUserMap.end(); ++iter) {
		m_featureUserVarMap[iter->first] = 1;
	}
	// init item feature variance as 1
	for (PtrFeedbackVectorMap::iterator iter = m_featureItemMap.begin();
			iter != m_featureItemMap.end(); ++iter) {
		m_featureItemVarMap[iter->first] = 1;
	}
	// init user latent prior mean as the mean of user posterior means
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
}

void MFImplicitModel::_updateUser(unsigned int userId) {
	PtrFeedbackVector& ratingsByUser = m_userRatingMap[userId];
	colvec uPosMean(m_modelParams.m_latentDim, fill::zeros);
	mat uPosCov = mat(m_modelParams.m_latentDim, m_modelParams.m_latentDim,
			fill::zeros);
	/// covariance matrix from items rated by the user
	for (PtrFeedbackVectorIterator iter = ratingsByUser.begin();
			iter < ratingsByUser.end(); ++iter) {
		unsigned int itemId = (*iter)->m_toId;
		float rating = (*iter)->m_val;
		colvec itemPosMean = m_itemProfileMap[itemId].m_posMean;
		mat itemPosCov = m_itemProfileMap[itemId].m_posCov;
		if (m_modelParams.m_isCovDiag) {
			itemPosCov = diagmat(itemPosCov);
		}
		uPosCov += (1 / m_ratingVar
				* (itemPosMean * itemPosMean.t() + itemPosCov));
		uPosMean += (rating - m_ratingBias) / m_ratingVar * itemPosMean;
	}
	/// covariance matrix from user features
	for (PtrFeedbackVectorIterator iter = m_userFeatureMap[userId].begin();
			iter < m_userFeatureMap[userId].end(); ++iter) {
		unsigned int featId = (*iter)->m_toId;
		colvec featPosMean = m_featureProfileMap[featId].m_posMean;
		mat featPosCov = m_featureProfileMap[featId].m_posCov;
		if (m_modelParams.m_isCovDiag) {
			featPosCov = diagmat(featPosCov);
		}
		float featVar = m_featureUserVarMap[featId];
		uPosCov += (1 / featVar * (featPosMean * featPosMean.t() + featPosCov));
		float feedVal = (*iter)->m_val;
		uPosMean += (feedVal / featVar * featPosMean);
	}
	/// covariance  matrix from user prior
	mat priorCov = diagmat(1 / m_userProfileMap[userId].m_priorCov);
	colvec priorMean = priorCov * m_userProfileMap[userId].m_priorMean;
	uPosCov += priorCov;
	uPosMean += priorMean;
	uPosCov = inv(uPosCov);
	uPosMean = uPosCov * uPosMean;
	m_userProfileMap[userId].m_posMean = uPosMean;
	if (m_modelParams.m_isCovDiag) {
		m_userProfileMap[userId].m_posCov = uPosCov.diag();
	} else {
		m_userProfileMap[userId].m_posCov = uPosCov;
	}
}

void MFImplicitModel::_updateItem(unsigned int itemId) {
	PtrFeedbackVector& ratingsForItem = m_itemRatingMap[itemId];

	colvec uPosMean(m_modelParams.m_latentDim, fill::zeros);
	mat uPosCov = mat(m_modelParams.m_latentDim, m_modelParams.m_latentDim,
			fill::zeros);
	/// covariance matrix from items rated by the user
	for (PtrFeedbackVectorIterator iter = ratingsForItem.begin();
			iter < ratingsForItem.end(); ++iter) {
		unsigned int userId = (*iter)->m_fromId;
		float rating = (*iter)->m_val;
		colvec userPosMean = m_userProfileMap[userId].m_posMean;
		mat userPosCov = m_userProfileMap[userId].m_posCov;
		if (m_modelParams.m_isCovDiag) {
			userPosCov = diagmat(userPosCov);
		}
		uPosCov += (1 / m_ratingVar
				* (userPosMean * userPosMean.t() + userPosCov));
		uPosMean += ((rating - m_ratingBias) / m_ratingVar * userPosMean);
	}
	/// covariance matrix from item features
	for (PtrFeedbackVectorIterator iter = m_itemFeatureMap[itemId].begin();
			iter < m_itemFeatureMap[itemId].end(); ++iter) {
		unsigned int featId = (*iter)->m_toId;
		colvec featPosMean = m_featureProfileMap[featId].m_posMean;
		mat featPosCov = m_featureProfileMap[featId].m_posCov;
		if (m_modelParams.m_isCovDiag) {
			featPosCov = diagmat(featPosCov);
		}
		float featVar = m_featureItemVarMap[featId];
		uPosCov += ((featPosMean * featPosMean.t() + featPosCov) / featVar);
		float feedVal = (*iter)->m_val;
		uPosMean = uPosMean + feedVal / featVar * featPosMean;
	}
	/// covariance  matrix from user prior
	mat priorCov = diagmat(1 / m_itemProfileMap[itemId].m_priorCov);
	colvec priorMean = (priorCov * m_itemProfileMap[itemId].m_priorMean);
	uPosCov += priorCov;
	uPosMean += priorMean;
	uPosCov = inv(uPosCov);
	uPosMean = uPosCov * uPosMean;
	if (m_modelParams.m_isCovDiag) {
		m_itemProfileMap[itemId].m_posCov = uPosCov.diag();
	} else {
		m_itemProfileMap[itemId].m_posCov = uPosCov;
	}
	m_itemProfileMap[itemId].m_posMean = uPosMean;
}

void MFImplicitModel::_updateFeature(unsigned int featId) {
	colvec featPosMean(m_modelParams.m_latentDim, fill::zeros);
	mat featPosCov = mat(m_modelParams.m_latentDim, m_modelParams.m_latentDim,
			fill::zeros);
	float userFeatVar = m_featureUserVarMap[featId];
	/// for each feature, find its feedbacks from both users and items
	PtrFeedbackVector& userFeedbacks = m_featureUserMap[featId];
	for (PtrFeedbackVectorIterator iter = userFeedbacks.begin();
			iter < userFeedbacks.end(); ++iter) {
		unsigned int userId = (*iter)->m_fromId;
		if(m_userProfileMap.find(userId) == m_userProfileMap.end())
			continue;
		float val = (*iter)->m_val;
		UserObject& uObject = m_userProfileMap[userId];
		colvec userPosMean = uObject.m_posMean;
		mat userPosCov = uObject.m_posCov;
		if (m_modelParams.m_isCovDiag) {
			userPosCov = diagmat(userPosCov);
		}
		featPosCov += ((userPosMean * userPosMean.t() + userPosCov)
				/ userFeatVar);
		featPosMean += (val / userFeatVar * uObject.m_posMean);
	}
	PtrFeedbackVector& itemFeedbacks = m_featureItemMap[featId];
	float itemFeatVar = m_featureItemVarMap[featId];
	for (PtrFeedbackVectorIterator iter = itemFeedbacks.begin();
			iter < itemFeedbacks.end(); ++iter) {
		unsigned int itemId = (*iter)->m_fromId;
		if(m_itemProfileMap.find(itemId) == m_itemProfileMap.end())
			continue;
		float val = (*iter)->m_val;
		ItemObject& iObject = m_itemProfileMap[itemId];
		colvec itemPosMean = iObject.m_posMean;
		mat itemPosCov = iObject.m_posCov;
		if (m_modelParams.m_isCovDiag) {
			itemPosCov = diagmat(itemPosCov);
		}
		featPosCov += ((itemPosMean * itemPosMean.t() + itemPosCov)
				/ itemFeatVar);
		featPosMean += (val / itemFeatVar * iObject.m_posMean);
	}
	// now update the posterior mean and covariance matrix
	featPosCov = inv(featPosCov);
	featPosMean = featPosCov * featPosMean;
	if (m_modelParams.m_isCovDiag) {
		m_featureProfileMap[featId].m_posCov = featPosCov.diag();
	} else {
		m_featureProfileMap[featId].m_posCov = featPosCov;
	}
	m_featureProfileMap[featId].m_posMean = featPosMean;
}

void MFImplicitModel::_updateRatingVariance() {
	m_ratingVar = 0;

	for(vector<size_t>::iterator iter = m_train_samples.begin(); iter != m_train_samples.end(); ++iter){
		Feedback const& fb = *(m_ratings[(*iter)-1]);
		unsigned int userId = fb.m_fromId;
		unsigned int itemId = fb.m_toId;
		float rating = fb.m_val;
		colvec userPosMean = m_userProfileMap[userId].m_posMean;
		colvec itemPosMean = m_itemProfileMap[itemId].m_posMean;
		mat userPosCov = m_userProfileMap[userId].m_posCov;
		mat itemPosCov = m_itemProfileMap[itemId].m_posCov;
		float predRating = as_scalar(userPosMean.t() * itemPosMean);
		float diff = rating - predRating - m_ratingBias;
		/// calculate the fitting error by prediction
		m_ratingVar += (diff * diff);
		/// calculate the covariance trace
		float covTrace = accu((userPosCov % itemPosCov));
		if (m_modelParams.m_isCovDiag) {
			covTrace += accu(userPosMean % itemPosCov % userPosMean);
			covTrace += accu(itemPosMean % userPosCov % itemPosMean);
		} else {
			covTrace += as_scalar(userPosMean.t() * itemPosCov * userPosMean);
			covTrace += as_scalar(itemPosMean.t() * userPosCov * itemPosMean);
		}
		m_ratingVar += covTrace;
	}
	m_ratingVar /= m_train_samples.size();

	cout << "rating var:" << m_ratingVar << endl;
}

void MFImplicitModel::_updateRatingBias() {
	m_ratingBias = 0;
	for(vector<size_t>::iterator iter = m_train_samples.begin(); iter != m_train_samples.end(); ++iter){
		Feedback& fb = *(m_ratings[(*iter) - 1]);
		unsigned int userId = fb.m_fromId;
		unsigned int itemId = fb.m_toId;
		float rating = fb.m_val;
		colvec& userPosMean = m_userProfileMap[userId].m_posMean;
		colvec& itemPosMean = m_itemProfileMap[itemId].m_posMean;
		float predRating = as_scalar(userPosMean.t() * itemPosMean);
		m_ratingBias += (rating - predRating);
	}
	m_ratingBias /= m_train_samples.size();
	cout << "rating bias:" << m_ratingBias << endl;
}

void MFImplicitModel::_updateUserFeatureVariance(unsigned int featId) {
	if (m_featureUserMap.find(featId) == m_featureUserMap.end()
			|| m_featureUserMap[featId].empty())
		return;
	float featUserVar = 0;
	FeatureObject& featObject = m_featureProfileMap[featId];
	colvec featPosMean = featObject.m_posMean;
	mat featPosCov = featObject.m_posCov;
	size_t numTrainUsers = 0;
	for (PtrFeedbackVectorIterator iter = m_featureUserMap[featId].begin();
			iter < m_featureUserMap[featId].end(); ++iter) {
		unsigned int userId = (*iter)->m_fromId;
		if(m_userProfileMap.find(userId) == m_userProfileMap.end())
			continue;
		numTrainUsers++;
		float val = (*iter)->m_val;
		UserObject& userObject = m_userProfileMap[userId];
		colvec userPosMean = userObject.m_posMean;
		mat userPosCov = userObject.m_posCov;
		float predVal = as_scalar(userPosMean.t() * featPosMean);
		float diff = predVal - val;
		featUserVar += (diff * diff);
		float covTrace = accu((userPosCov % featPosCov));
		if (m_modelParams.m_isCovDiag) {
			covTrace += accu(userPosMean % featPosCov % userPosMean);
			covTrace += accu(featPosMean % userPosCov % featPosMean);
		} else {
			covTrace += (as_scalar(userPosMean.t() * featPosCov * userPosMean));
			covTrace += (as_scalar(featPosMean.t() * userPosCov * featPosMean));
		}
		featUserVar += covTrace;
	}
	featUserVar /= numTrainUsers;
	m_featureUserVarMap[featId] = featUserVar;
}

void MFImplicitModel::_updateItemFeatureVariance(unsigned int featId) {
	if (m_featureItemMap.find(featId) == m_featureItemMap.end()
			|| m_featureItemMap[featId].empty())
		return;
	float featItemVar = 0;
	FeatureObject& featObject = m_featureProfileMap[featId];
	colvec featPosMean = featObject.m_posMean;
	mat featPosCov = featObject.m_posCov;
	size_t numTrainItem = 0;
	for (PtrFeedbackVectorIterator iter = m_featureItemMap[featId].begin();
			iter < m_featureItemMap[featId].end(); ++iter) {
		unsigned int itemId = (*iter)->m_fromId;
		if(m_itemProfileMap.find(itemId) == m_itemProfileMap.end())
			continue;
		numTrainItem++;
		float val = (*iter)->m_val;
		ItemObject& itemObject = m_itemProfileMap[itemId];
		colvec itemPosMean = itemObject.m_posMean;
		mat itemPosCov = itemObject.m_posCov;
		float predVal = as_scalar(itemPosMean.t() * featPosMean);
		float diff = predVal - val;
		featItemVar += (diff * diff);
		float covTrace = accu((itemPosCov % featPosCov));
		if (m_modelParams.m_isCovDiag) {
			covTrace += accu(itemPosMean % featPosCov % itemPosMean);
			covTrace += accu(featPosMean % itemPosCov % featPosMean);
		} else {
			covTrace += (as_scalar(itemPosMean.t() * featPosCov * itemPosMean));
			covTrace += (as_scalar(featPosMean.t() * itemPosCov * featPosMean));
		}
		featItemVar += covTrace;
	}
	featItemVar /= numTrainItem;
	m_featureItemVarMap[featId] = featItemVar;
}

void MFImplicitModel::_updateUserPrior() {
	colvec priorMean(m_modelParams.m_latentDim, fill::zeros);
	for (UserObjectMap::iterator iter = m_userProfileMap.begin(); iter != m_userProfileMap.end();
			++iter) {
		UserObject& uObject = iter->second;
		colvec userPosMean = uObject.m_posMean;
		priorMean += userPosMean;
	}
	priorMean = priorMean / m_userProfileMap.size();
	mat priorCov(m_modelParams.m_latentDim, 1, fill::zeros);
	for (UserObjectMap::iterator iter = m_userProfileMap.begin(); iter != m_userProfileMap.end();
			++iter) {
		UserObject& uObject = iter->second;
		colvec userPosMean = uObject.m_posMean;
		mat& userPosCov = uObject.m_posCov;
		// consider the fitting error
		colvec diff = userPosMean - priorMean;
		priorCov = priorCov + (diff % diff);
		if (m_modelParams.m_isCovDiag) {
			priorCov = priorCov + userPosCov;
		} else {
			priorCov = priorCov + diagvec(userPosCov);
		}
	}
	priorCov = priorCov / m_userProfileMap.size();
	m_userPriorMean = priorMean;
	m_userPriorCov = priorCov;
	for (UserObjectMap::iterator iter = m_userProfileMap.begin(); iter != m_userProfileMap.end();
			++iter) {
		UserObject& uObject = iter->second;
		uObject.m_priorMean = priorMean;
		uObject.m_priorCov = priorCov;
	}
}

void MFImplicitModel::_updateItemPrior() {
	colvec priorMean(m_modelParams.m_latentDim, fill::zeros);
	for (ItemObjectMap::iterator iter = m_itemProfileMap.begin(); iter != m_itemProfileMap.end();
			++iter) {
		ItemObject& iObject = iter->second;
		colvec itemPosMean = iObject.m_posMean;
		priorMean = priorMean + itemPosMean;
	}
	priorMean = priorMean / m_itemProfileMap.size();
	mat priorCov(m_modelParams.m_latentDim, 1, fill::zeros);
	for (ItemObjectMap::iterator iter = m_itemProfileMap.begin(); iter != m_itemProfileMap.end();
			++iter) {
		ItemObject& iObject = iter->second;
		colvec itemPosMean = iObject.m_posMean;
		mat itemPosCov = iObject.m_posCov;
		// consider the fitting error
		colvec diff = itemPosMean - priorMean;
		priorCov = priorCov + (diff % diff);
		if (m_modelParams.m_isCovDiag) {
			priorCov = priorCov + (itemPosCov);
		} else {
			priorCov = priorCov + diagvec(itemPosCov);
		}
	}
	priorCov = priorCov / m_itemProfileMap.size();
	m_itemPriorMean = priorMean;
	m_itemPriorCov = priorCov;
	for (ItemObjectMap::iterator iter = m_itemProfileMap.begin(); iter != m_itemProfileMap.end();
			++iter) {
		ItemObject& iObject = iter->second;
		iObject.m_priorMean = priorMean;
		iObject.m_priorCov = priorCov;
	}
}


void MFImplicitModel::optimize() {
	/// init model
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
			_updateUser(iter->first);
		}
		/// update each item
		cout << "update item profile..." << endl;
		for (ItemObjectMap::iterator iter = m_itemProfileMap.begin();
				iter != m_itemProfileMap.end(); ++iter) {
			_updateItem(iter->first);
		}
		cout << "update feature profile..." << endl;
		/// update each feature
		for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin();
				iter != m_featureProfileMap.end(); ++iter) {
			_updateFeature(iter->first);
		}
		/// update model parameters, start with global bias
		_updateRatingBias();
		cout << "update rating variance..." << endl;
		/// rating variance
		_updateRatingVariance();
		cout << "update user feature variance..." << endl;
		/// user feature variance
		for (PtrFeedbackVectorMap::iterator iter = m_featureUserMap.begin();
				iter != m_featureUserMap.end(); ++iter) {
			_updateUserFeatureVariance(iter->first);
		}
		cout << "update item feature variance..." << endl;
		/// item feature variance
		for (PtrFeedbackVectorMap::iterator iter = m_featureItemMap.begin();
				iter != m_featureItemMap.end(); ++iter) {
			_updateItemFeatureVariance(iter->first);
		}
		cout << "update user prior..." << endl;
		/// update user prior
		_updateUserPrior();
		cout << "update item prior..." << endl;
		/// update item prior
		_updateItemPrior();
		/// evaluate the training RMSE
		float iterRMSE = _trainRMSE();
		cout << "iter:" << emIter << ",training RMSE:" << iterRMSE << endl;
	}
}

void MFImplicitModel::_userProfileOnPrior(UserObject& userObject){
	userObject.m_priorMean = userObject.m_posMean = m_userPriorMean;
	userObject.m_priorCov = userObject.m_posCov = m_userPriorCov;
}

void MFImplicitModel::_userProfileOnFeatures(UserObject& userObject, PtrFeedbackVector const& feats){
	mat posCovInv = userObject.m_posCov;
	if(m_modelParams.m_isCovDiag){
		posCovInv = diagmat(1/posCovInv);
	}else{
		posCovInv = inv(posCovInv);
	}
	colvec posMeanInv = posCovInv * userObject.m_posMean;
	for(PtrFeedbackVector::const_iterator iter = feats.begin(); iter < feats.end(); iter++){
		size_t featId = (*iter)->m_toId;
		if(m_featureProfileMap.find(featId) != m_featureProfileMap.end()){
			FeatureObject& fObject = m_featureProfileMap[featId];
			colvec& fPosMean = fObject.m_posMean;
			mat& fPosCov = fObject.m_posCov;
			if(m_modelParams.m_isCovDiag){
				fPosCov = diagmat(fPosCov);
			}
			float featVar = m_featureUserVarMap[featId];
			float featVal = (*iter)->m_val;
			posCovInv += (featVal/featVar) * (fPosCov + fPosMean * fPosMean.t());
			posMeanInv += (featVal/featVar) * fPosMean;
		}
	}
	userObject.m_posCov = inv(posCovInv);
	userObject.m_posMean = userObject.m_posCov * posMeanInv;
	if(m_modelParams.m_isCovDiag){
		userObject.m_posCov = userObject.m_posCov.diag();
	}
}

float MFImplicitModel::_trainRMSE() {
	float rmse = 0;
	for(vector<size_t>::iterator iter = m_train_samples.begin(); iter != m_train_samples.end(); ++iter){
		Feedback const& fb = *(m_ratings[(*iter)-1]);
		unsigned int userId = fb.m_fromId;
		unsigned int itemId = fb.m_toId;
		float rating = fb.m_val;
		UserObject& userObject = m_userProfileMap[userId];
		ItemObject& itemObject = m_itemProfileMap[itemId];
		float predRating = as_scalar(
				userObject.m_posMean.t() * itemObject.m_posMean);
		float diff = rating - predRating - m_ratingBias;
		rmse += (diff * diff);
	}
	rmse /= m_train_samples.size();
	return sqrtf(rmse);
}

float MFImplicitModel::testRMSE(){
	float rmse = 0;
	size_t numRatings = 0;
	for(vector<size_t>::iterator iter = m_test_samples.begin(); iter < m_test_samples.end(); ++iter){
		size_t sampleIdx = (*iter) - 1;
		Feedback const& fb = *(m_ratings[sampleIdx]);
		size_t userId = fb.m_fromId;
		size_t itemId = fb.m_toId;
		if(m_itemProfileMap.find(itemId) == m_itemProfileMap.end())
			continue;
		if(m_userProfileMap.find(userId) == m_userProfileMap.end()){
			/// a new user
			UserObject uObject(userId,m_modelParams.m_latentDim,m_modelParams.m_isCovDiag);
			_userProfileOnPrior(uObject);
			/// get user features
			if(m_userFeatureMap.find(userId) != m_userFeatureMap.end()){
				PtrFeedbackVector const& uFeats = m_userFeatureMap[userId];
				_userProfileOnFeatures(uObject,uFeats);
			}
			m_userProfileMap[userId] = uObject;
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
