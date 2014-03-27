/*
 * UserRecommendation_test.cpp
 *
 *  Created on: Mar 22, 2014
 *      Author: qzhao2
 */

#include "UserRecommendation.h"

namespace recsys {
void test_user_recommendation(){
#define TEST_READ
#ifdef TEST_WRITE
	UserRecommendation uRec1("qizhao");
	uRec1.add_item_recommendation(UserRecommendation::Recommendation("kinnect",4.3));
	uRec1.add_item_recommendation(UserRecommendation::Recommendation("machinelearning",5.0));
	uRec1.write();
#endif
#ifdef TEST_READ
	vector<UserRecommendation> recs = UserRecommendation::query_by_user("qizhao");
	for(unsigned int i = 0; i < recs.size(); i++){
		cout << recs[i] << endl;
	}
#endif
}

} /* namespace recsys */
