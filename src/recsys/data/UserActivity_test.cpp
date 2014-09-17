/*
 * UserActivity_test.cpp
 *
 *  Created on: Mar 23, 2014
 *      Author: qzhao2
 */

#include "UserActivity.h"

namespace recsys {

void test_userActivity(){
	UserActivity ua1("qizhao",UserActivity::VIEW_USER,"");
	ua1.write();
	vector<UserActivity> activities = UserActivity::query_by_user("qizhao");
	for(unsigned int i = 0; i < activities.size(); i++){
		cout << activities[i];
	}
}

} /* namespace recsys */
