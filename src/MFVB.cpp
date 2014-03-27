//============================================================================
// Name        : MFVB.cpp
// Author      : Qi Zhao
// Version     :
// Copyright   : All rights reserved by Qi Zhao
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "armadillo"
#include <string>
#include "MFImplicitModel.h"
#include "VLImplicitModel.h"
#include "csv.h"
#include "recsys/data/SQL.h"
#include "recsys/data/Entity.h"
#include "recsys/data/UserRecommendation.h"
#include "recsys/data/UserActivity.h"

using namespace arma;
using namespace std;
using namespace recsys;


void runModel(){
	///setup model parameters
	ModelParam MODEL_PARAM;
	MODEL_PARAM.m_isCovDiag = true;
	MODEL_PARAM.m_latentDim = 30;
	MODEL_PARAM.m_maxIter = 10;
	MODEL_PARAM.m_useFeature = true;
	string paramStr((string) MODEL_PARAM);

	//0.988736
	DataConfig DATA_CONFIG;
	DATA_CONFIG.m_resultFolder = "/home/qzhao2/soe/data/ml-1m/lfimplicit/result_" + paramStr;
	DATA_CONFIG.m_ratingFile = "/home/qzhao2/soe/data/ml-1m/lfimplicit/ratings.dat";
	DATA_CONFIG.m_userFeatureFile = "/home/qzhao2/soe/data/ml-1m/lfimplicit/user.feat.sp";
	DATA_CONFIG.m_itemFeatureFile = "/home/qzhao2/soe/data/ml-1m/lfimplicit/item.feat.sp";
	DATA_CONFIG.m_trainIdxFile = "/home/qzhao2/soe/data/ml-1m/lfimplicit/cv_1/train.idx_cv_1/train.idx";
	DATA_CONFIG.m_testIdxFile = "/home/qzhao2/soe/data/ml-1m/lfimplicit/cv_1/train.idx_cv_1/test.idx";
	MFImplicitModel model(DATA_CONFIG, MODEL_PARAM);
//	VLImplicitModel model(DATA_CONFIG, MODEL_PARAM);
	model.init();
	model.optimize();
//	model.dumpFeatureCovariance();
//	model.dumpFeatureBias();
//	model.dumpFeatureNorm();
//	model.testRMSE(true); ///
//	model.testRMSE(false); /// 0.989201
}

//int main() {
//	return 0;
//}
