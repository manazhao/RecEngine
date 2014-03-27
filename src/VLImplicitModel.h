/*
 * VLImplicitModel.h
 *
 *  Created on: Feb 14, 2014
 *      Author: qzhao2
 */

#ifndef VLIMPLICITMODEL_H_
#define VLIMPLICITMODEL_H_

#include "MFImplicitModel.h"
#include <math.h>
struct FeatureScore {
	unsigned int m_featureIndex;
	float m_score;
	FeatureScore(unsigned int featureIndex = 0, float score = 0) :
			m_featureIndex(featureIndex), m_score(score) {
	}
	bool operator<(FeatureScore const& rhs) const {
		return m_score < rhs.m_score;
	}
};

ostream& operator<<(ostream& oss, FeatureScore const& rhs);

class VLImplicitModel: public MFImplicitModel {
private:
	map<size_t, float> m_featUserBiasMap; /// the bias of the sigmoid function of user and feature f(y_uk|p_u,s_k,b_k)=1/(e^-y_uk(p_u.s_k+b_k)+1)
	map<size_t, float> m_featItemBiasMap;
	map<string, float> m_userFeatContMap; /// the optimal contact point for user-feature variational sigmoid function
	map<string, float> m_itemFeatContMap; /// the optimal contact point for item-feature variational sigmoid function
protected:
	set<unsigned int> m_topFeatureSet;
	colvec m_featurePriorMean;
	mat m_featurePriorCov;
protected:
	/**
	 * \brief calculate the tangent of the lower bound of sigmoid function given the contact
	 * point as $\xi$
	 *
	 * The tangent is calculated as, $\lambda(\xi) = \frac{1}{2\xi}(\sigma(\xi) - 1/2)$
	 * where $\sigma(\xi)$ is the sigmoid function which defines as,
	 * $\sigma(\xi) = \frac{1}{\exp{-\xi}+1}$
	 *
	 * @return the tangent
	 */
	inline float _lambdaXi(float xi) const {
		return 1 / (2 * xi) * (1 / (1 + exp(-xi)) - 0.5);
	}

	void _updateUserByRating(UserObject& userObject, Feedback const& fb);
	void _updateUserByRatings(UserObject& userObject,
			PtrFeedbackVector const& userRatings);
	void _updateUserByFeature(UserObject& userObject, Feedback const& fb);
	void _updateUserByFeatures(UserObject& userObject,
			PtrFeedbackVector const& userFeatures);

	void _updateItemByRating(ItemObject& itemObject, Feedback const& fb);
	void _updateItemByRatings(ItemObject& itemObject,
			PtrFeedbackVector const& itemRatings);
	void _updateItemByFeature(ItemObject& itemObject, Feedback const& fb);
	void _updateItemByFeatures(ItemObject& itemObject,
			PtrFeedbackVector const& itemFeatures);

	void _updateFeatureByUser(FeatureObject& featureObject, Feedback const& fb);
	void _updateFeatureByUsers(FeatureObject& featureObject,
			PtrFeedbackVector const& fb);
	void _updateFeatureByItem(FeatureObject& featureObject, Feedback const& fb);
	void _updateFeatureByItems(FeatureObject& featureObject,
			PtrFeedbackVector const& fb);

	void _updateFeatUserBias();
	void _updateFeatItemBias();
	void _updateUserFeatCont(PtrFeedbackVectorMap const& userFeatureMap =
			PtrFeedbackVectorMap());
	void _updateItemFeatCont(PtrFeedbackVectorMap const& itemFeatureMap =
			PtrFeedbackVectorMap());
	void _updateNewUserOnFeats(UserObject& userObject,
			PtrFeedbackVector const& userFeats);
	void _updateFeaturePrior();
	virtual void _initParam();
	void _selectTopKFeature(PtrFeedbackVector& featVec){
		PtrFeedbackVector topkFeatVec;
		if(!m_topFeatureSet.empty()){
			for(PtrFeedbackVector::iterator iter = featVec.begin(); iter < featVec.end(); ++iter){
				Feedback& fb = **iter;
				if(m_topFeatureSet.find(fb.m_toId) != m_topFeatureSet.end()){
					topkFeatVec.push_back(*iter);
				}
			}
			featVec = topkFeatVec;
		}
	}
	void _dumpIFFit(){
		for(PtrFeedbackVectorMap::iterator iter = m_userFeatureMap.begin(); iter != m_userFeatureMap.end(); ++iter){
			PtrFeedbackVector& userFeats = iter->second;
			if(m_userProfileMap.find(iter->first) == m_userProfileMap.end())
				continue;
			cout << "------implicit feedback fit for user:" << iter->first << endl;
			UserObject& uObject = m_userProfileMap[iter->first];
			colvec& userMean = uObject.m_posMean;
			for(PtrFeedbackVector::iterator iter1 = userFeats.begin(); iter1 < userFeats.end(); ++iter1){
				Feedback& fb = **iter1;
				FeatureObject& fObject = m_featureProfileMap[fb.m_toId];
				colvec& fMean = fObject.m_posMean;
				float prod = as_scalar(fMean.t() * userMean);
				float y = 1/(exp(-prod) + 1);
				cout << y << ",";
			}
			cout << endl;
		}
//
//		for(PtrFeedbackVectorMap::iterator iter = m_itemFeatureMap.begin(); iter != m_itemFeatureMap.end(); ++iter){
//			PtrFeedbackVector& itemFeats = iter->second;
//			if(m_itemProfileMap.find(iter->first) == m_itemProfileMap.end())
//				continue;
//			cout << "------implicit feedback fit for item:" << iter->first << endl;
//			ItemObject& iObject = m_itemProfileMap[iter->first];
//			colvec& userMean = iObject.m_posMean;
//			for(PtrFeedbackVector::iterator iter1 = itemFeats.begin(); iter1 < itemFeats.end(); ++iter1){
//				Feedback& fb = **iter1;
//				FeatureObject& fObject = m_featureProfileMap[fb.m_toId];
//				colvec& fMean = fObject.m_posMean;
//				float prod = as_scalar(fMean.t() * userMean);
//				float y = 1/(exp(-prod) + 1);
//				cout << y << ",";
//			}
//			cout << endl;
//		}

	}

public:
	VLImplicitModel(DataConfig const& dataConfig,
			ModelParam const& modelParams);
	void topKFeatures(unsigned int k = 5);
	virtual float testRMSE(bool useFeature = true);
	virtual void optimize();
	virtual ~VLImplicitModel();
	void dumpFeatureNorm(){
		for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin();
				iter != m_featureProfileMap.end(); ++iter) {
			FeatureObject& fObject = iter->second;
			cout << iter->first << ":"
					<< arma::norm(fObject.m_posMean,2) << endl;
		}
	}
	void dumpFeatureBias() {
		cout << "user feature bias:" << endl;
		for (map<size_t, float>::iterator iter = m_featUserBiasMap.begin();
				iter != m_featUserBiasMap.end(); ++iter) {
			cout << iter->first << ":" << iter->second << endl;
		}
		cout << "item feature bias:" << endl;
		for (map<size_t, float>::iterator iter = m_featItemBiasMap.begin();
				iter != m_featItemBiasMap.end(); ++iter) {
			cout << iter->first << ":" << iter->second << endl;
		}
	}
	void dumpFeatureCovariance() {
		for (FeatureObjectMap::iterator iter = m_featureProfileMap.begin();
				iter != m_featureProfileMap.end(); ++iter) {
			FeatureObject& fObject = iter->second;
			cout << iter->first << ":"
					<< as_scalar(arma::mean(fObject.m_posCov)) << endl;
		}
	}

};

#endif /* VLIMPLICITMODEL_H_ */
