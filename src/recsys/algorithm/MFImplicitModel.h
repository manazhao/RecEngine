/*
 * MFImplicitModel.h
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#ifndef MFIMPLICITMODEL_H_
#define MFIMPLICITMODEL_H_

#include <map>
#include <set>
#include <vector>
#include <string>
#include <armadillo>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include "Feedback.h"
#include "UserObject.h"
#include "ItemObject.h"
#include "FeatureObject.h"

using namespace std;
using namespace arma;
using namespace boost;

struct ModelParam {
	unsigned int m_latentDim;
	bool m_isCovDiag;
	unsigned int m_maxIter;
	bool m_useFeature;
public:
	ModelParam(unsigned int latentDim = 10, bool isCovDiag = true,
			unsigned int maxIter = 20, bool useFeature = false) :
			m_latentDim(latentDim), m_isCovDiag(isCovDiag), m_maxIter(maxIter), m_useFeature(
					useFeature) {
	}
	operator string (){
		stringstream ss;
		ss << m_latentDim << "_" << m_isCovDiag << "_" << m_maxIter << "_" << m_useFeature;
		return ss.str();
	}
};

struct DataConfig{
	string m_ratingFile;
	string m_userFeatureFile;
	string m_itemFeatureFile;
	string m_trainIdxFile;
	string m_testIdxFile;
	string m_resultFolder;
};

class MFImplicitModel {
public:
	typedef vector<shared_ptr<Feedback> > PtrFeedbackVector;
	typedef PtrFeedbackVector::iterator PtrFeedbackVectorIterator;
	typedef map<unsigned int, UserObject> UserObjectMap;
	typedef map<unsigned int, ItemObject> ItemObjectMap;
	typedef map<unsigned int, FeatureObject> FeatureObjectMap;
	typedef map<unsigned int, PtrFeedbackVector> PtrFeedbackVectorMap;
private:
	map<unsigned int, float> m_featureUserVarMap; /// user feature feedback variance
	map<unsigned int, float> m_featureItemVarMap; /// item feature feedback variance

protected:
	/// model related
	UserObjectMap m_userProfileMap; /// all users
	ItemObjectMap m_itemProfileMap; /// all items
	FeatureObjectMap m_featureProfileMap; /// all implicit feedbacks
	PtrFeedbackVectorMap m_userRatingMap; /// ratings of a given user
	PtrFeedbackVectorMap m_itemRatingMap; /// ratings of a given item
	PtrFeedbackVectorMap m_userFeatureMap; ///
	PtrFeedbackVectorMap m_itemFeatureMap;
	PtrFeedbackVectorMap m_featureUserMap;
	PtrFeedbackVectorMap m_featureItemMap;
	float m_ratingBias; /// global rating bias
	float m_ratingVar; /// rating variance
	colvec m_userPriorMean;
	mat m_userPriorCov;
	colvec m_itemPriorMean;
	mat m_itemPriorCov;
	PtrFeedbackVector m_ratings; /// all user-item rating pairs
	vector<size_t> m_train_samples;
	set<unsigned int> m_train_users;
	set<unsigned int> m_train_items;
	vector<size_t> m_test_samples;
	set<unsigned int> m_test_users;
	set<unsigned int> m_test_items;
	DataConfig m_dataConfig;
	/// model parameters
	ModelParam m_modelParams;

protected:
	vector<size_t> _loadIndex(string const& fileName);
	void _userProfileOnPrior(UserObject& userObject);
	void _userProfileOnFeatures(UserObject& userObject, PtrFeedbackVector const& feats);
	void _loadTrainingSamples();
	void _loadTestingSamples();
	void _loadRatings();
	void _loadUserFeatures();
	void _loadItemFeatures();
	virtual void _initParam();
	virtual void _updateUser(unsigned int userId);
	virtual void _updateItem(unsigned int itemId);
	virtual void _updateFeature(unsigned int featId);
	void _updateRatingVariance();
	void _updateRatingBias();
	void _updateUserFeatureVariance(unsigned int featId);
	void _updateItemFeatureVariance(unsigned int featId);
	void _updateUserPrior();
	void _updateItemPrior();
	float _trainRMSE();
	////
	float _averageTrainRating();
public:
	MFImplicitModel(DataConfig const& dataConfig, ModelParam const& modelParams);
	void init();
	virtual void optimize();
	virtual float testRMSE();
	virtual ~MFImplicitModel();
};

#endif /* MFIMPLICITMODEL_H_ */
