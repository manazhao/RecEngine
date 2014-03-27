/*
 * UserActivity.h
 *
 *  Created on: Mar 23, 2014
 *      Author: qzhao2
 */

#ifndef USERACTIVITY_H_
#define USERACTIVITY_H_
#include <string>
#include <map>
#include <set>
#include <vector>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <cppconn/prepared_statement.h>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"
#include "SQL.h"

using namespace std;
using namespace boost;
using namespace sql;
using namespace json;


namespace recsys {
typedef shared_ptr<PreparedStatement> prepared_statement_ptr;

class UserActivity {
	friend ostream& operator<<(ostream&, UserActivity const&);
public:
	enum ACTIVITY_TYPE{
		VIEW_ITEM = 0,
		VIEW_LIST = 1 ,
		VIEW_USER = 2,
		VIEW_REC_LIST = 3,
		CLICK_URL = 4,
		CLICK_ITEM = 5,
		CLICK_LIST = 6,
		CLICK_REC_ENTRY = 7,
		RATE_ITEM = 8,
		RATE_LIST = 9,
		COMMENT_ITEM = 10,
		COMMENT_LIST = 11
	};
	struct SharedData {
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryByUserStmtPtr;
	};
protected:
	string m_user_id;
	/// activity type
	unsigned short m_type;
	// context; described by json text
	string m_context;
	string m_time;
	static SharedData m_sharedData;
protected:
	string _get_current_time();
public:
	UserActivity(string const& userId, unsigned short type, string const& context);
	static SharedData initSharedData();
	bool exist();
	void write();
	void read();
	static vector<UserActivity> query_by_user(string const& userId);
	virtual ~UserActivity();
};

void test_userActivity();

} /* namespace recsys */

#endif /* USERACTIVITY_H_ */
