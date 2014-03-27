/*
 * UserRecommendation.h
 *
 *  Created on: Mar 22, 2014
 *      Author: qzhao2
 */

#ifndef USERRECOMMENDATION_H_
#define USERRECOMMENDATION_H_
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
class UserRecommendation {
	friend ostream& operator<<(ostream&, UserRecommendation const&);
public:
	struct SharedData {
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryStmtPtr;
		prepared_statement_ptr m_updateStmtPtr;
	};
public:
	struct Recommendation {
		string m_id;
		float m_score;
		Recommendation(string id = string(), float score = float()) :
				m_id(id), m_score(score) {
		}
		bool operator<(Recommendation const& rhs) const{
			return m_score > rhs.m_score;
		}
	};
	typedef vector<Recommendation> rec_vec;
	typedef map<string, rec_vec> rec_vec_map;
protected:
	static SharedData m_sharedData;
protected:
	string m_user_id;
	string m_rec_list;
	string m_time;
	rec_vec_map m_rec_map;
protected:
	void _json_to_map();
	void _map_to_json();
	string _get_current_time();
public:
	UserRecommendation(string const& id = string(), string const& recList = string()) :
			m_user_id(id), m_rec_list(recList) {
	}
	static SharedData initSharedData();
	void add_item_recommendation(Recommendation const& rec);
	bool exist();
	void write();
	void read();
	static vector<UserRecommendation> query_by_user(string const& userId);
public:
	UserRecommendation();
	virtual ~UserRecommendation();
};

void test_user_recommendation();

} /* namespace recsys */

#endif /* USERRECOMMENDATION_H_ */
