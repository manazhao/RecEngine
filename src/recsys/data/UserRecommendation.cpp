/*
 * UserRecommendation.cpp
 *
 *  Created on: Mar 22, 2014
 *      Author: qzhao2
 */

#include "UserRecommendation.h"

namespace recsys {

UserRecommendation::SharedData UserRecommendation::initSharedData() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref();
		string createTbSql =
				"CREATE TABLE IF NOT EXISTS user_recommendation (id INT AUTO_INCREMENT PRIMARY KEY, user_id VARCHAR(255) NOT NULL, rec_list TEXT, time DATETIME NOT NULL)";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO user_recommendation (user_id,rec_list,time) VALUES (?,?,?)"));
		sharedData.m_queryStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * FROM user_recommendation WHERE user_id = ?"));
		inited = true;
		return sharedData;
	}
	return UserRecommendation::m_sharedData;
}

UserRecommendation::SharedData UserRecommendation::m_sharedData =
		UserRecommendation::initSharedData();

void UserRecommendation::add_item_recommendation(Recommendation const& rec) {
	m_rec_map["i"].push_back(rec);
}

string UserRecommendation::_get_current_time() {
	time_t t = time(0);
	struct tm* now = localtime(&t);
	stringstream ss;
	ss << now->tm_year + 1900 << "-" << now->tm_mon + 1 << "-" << now->tm_mday
			<< " " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec;
	return ss.str();
}

void UserRecommendation::_json_to_map() {
	stringstream ss;
	ss << m_rec_list;
	Object jsonRoot;
	Reader::Read(jsonRoot, ss);
	const Array& itemRecList = jsonRoot["i"];
	rec_vec tmpRecList;
	for (Array::const_iterator iter = itemRecList.Begin();
			iter != itemRecList.End(); ++iter) {
		Object tmpRec = *iter;
		Recommendation tmpRec1;
		String id = tmpRec["i"];
		Number score = tmpRec["s"];
		tmpRec1.m_id = id;
		tmpRec1.m_score = score;
		tmpRecList.push_back(tmpRec1);
	}
	m_rec_map["i"] = tmpRecList;
}

void UserRecommendation::_map_to_json() {
	Object recListArr;
	for (rec_vec_map::iterator iter = m_rec_map.begin();
			iter != m_rec_map.end(); ++iter) {
		string type = iter->first;
		rec_vec& recList = iter->second;
		Array recArr;
		for (rec_vec::const_iterator iter1 = recList.begin();
				iter1 != recList.end(); ++iter1) {
			const Recommendation& rec = *iter1;
			Object recObj;
			recObj["i"] = String(rec.m_id);
			recObj["s"] = Number(rec.m_score);
			recArr.Insert(recObj);
		}
		recListArr[type] = (recArr);
	}
	stringstream ss;
	Writer::Write(recListArr, ss);
	m_rec_list = ss.str();
	/// remove the extra spaces in the json string
	m_rec_list.erase(
			std::remove_if(m_rec_list.begin(), m_rec_list.end(), ::isspace),
			m_rec_list.end());
//	cout << m_rec_list << endl;
}

bool UserRecommendation::exist() {
	prepared_statement_ptr& queryStmtPtr =
			UserRecommendation::m_sharedData.m_queryStmtPtr;
	queryStmtPtr->setString(1, m_user_id);
	auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
	return rs->next();
}

void UserRecommendation::write() {
	_map_to_json();
	m_time = _get_current_time();
	prepared_statement_ptr& insertStmt =
			UserRecommendation::m_sharedData.m_insertStmtPtr;
	insertStmt->setString(1, m_user_id);
	insertStmt->setString(2, m_rec_list);
	insertStmt->setString(3,m_time);
	insertStmt->execute();
}

void UserRecommendation::read() {
}

UserRecommendation::~UserRecommendation() {
	// TODO Auto-generated destructor stub
}


vector<UserRecommendation> UserRecommendation::query_by_user(string const& userId){
	prepared_statement_ptr& queryStmt =
			UserRecommendation::m_sharedData.m_queryStmtPtr;
	queryStmt->setString(1, userId);
	auto_ptr<ResultSet> rs(queryStmt->executeQuery());
	vector<UserRecommendation> recs;
	while (rs->next()) {
		string userId = rs->getString("id");
		string recList = rs->getString("rec_list");
		string time = rs->getString("time");
		UserRecommendation tmpRec(userId,recList);
		tmpRec.m_time = time;
		tmpRec._json_to_map();
		recs.push_back(tmpRec);
	}
	return recs;
}

ostream& operator<<(ostream& oss, UserRecommendation const& rec) {
	oss << "user id:" << rec.m_user_id << endl;
	for (UserRecommendation::rec_vec_map::const_iterator iter =
			rec.m_rec_map.begin(); iter != rec.m_rec_map.end(); ++iter) {
		string type = iter->first;
		UserRecommendation::rec_vec const& recSet = iter->second;
		oss << type << ":";
		for (UserRecommendation::rec_vec::const_iterator iter1 = recSet.begin();
				iter1 != recSet.end(); ++iter1) {
			oss << iter1->m_id << " - " << iter1->m_score << ",";
		}
		oss << rec.m_time << endl;
	}
	return oss;
}

} /* namespace recsys */
