/*
 * UserActivity.cpp
 *
 *  Created on: Mar 23, 2014
 *      Author: qzhao2
 */

#include "UserActivity.h"
#include <ctime>


namespace recsys {

UserActivity::SharedData UserActivity::initSharedData(){
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref();
		string createTbSql =
				"CREATE TABLE IF NOT EXISTS user_activity (id INT AUTO_INCREMENT PRIMARY KEY, user_id VARCHAR(255) NOT NULL, type TINYINT NOT NULL, context TEXT, time DATETIME)";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO user_activity (user_id,type,context,time) VALUES (?,?,?,?)"));
		sharedData.m_queryByUserStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT user_id,type,context,time FROM user_activity WHERE user_id = ?"));
		inited = true;
		return sharedData;
	}
	return UserActivity::m_sharedData;
}

UserActivity::SharedData UserActivity::m_sharedData = UserActivity::initSharedData();

bool UserActivity::exist(){
	prepared_statement_ptr& queryStmt = m_sharedData.m_queryByUserStmtPtr;
	queryStmt->setString(1,m_user_id);
	auto_ptr<ResultSet> rs(queryStmt->executeQuery());
	return rs->next();
}

string UserActivity::_get_current_time(){
	time_t t = time(0);
	struct tm* now = localtime(&t);
	stringstream ss;
	ss << now->tm_year + 1900 << "-" << now->tm_mon + 1 << "-" << now->tm_mday << " " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec;
	return ss.str();
}

vector<UserActivity> UserActivity::query_by_user(string const& userId){
	vector<UserActivity> userActivities;
	prepared_statement_ptr& queryStmt = m_sharedData.m_queryByUserStmtPtr;
	queryStmt->setString(1,userId);
	auto_ptr<ResultSet> rs(queryStmt->executeQuery());
	while(rs->next()){
		unsigned short type = rs->getInt("type");
		string context = rs->getString("context");
		string dateTime = rs->getString("time");
		UserActivity tmpActivity(userId,type,context);
		tmpActivity.m_time = dateTime;
		userActivities.push_back(tmpActivity);
	}
	return userActivities;
}

void UserActivity::write(){
	prepared_statement_ptr& insertStmt = m_sharedData.m_insertStmtPtr;
	insertStmt->setString(1,m_user_id);
	insertStmt->setInt(2,m_type);
	insertStmt->setString(3,m_context);
	m_time = _get_current_time();
	insertStmt->setDateTime(4,m_time);
	insertStmt->execute();
}

void UserActivity::read(){

}

UserActivity::UserActivity(string const& userId, unsigned short type, string const& context)
:m_user_id(userId),m_type(type),m_context(context){
	// TODO Auto-generated constructor stub

}

UserActivity::~UserActivity() {
	// TODO Auto-generated destructor stub
}

ostream& operator<<(ostream& oss, UserActivity const& rhs){
	oss << rhs.m_user_id << "," << rhs.m_type << "," << rhs.m_context << rhs.m_time << endl;
	return oss;
}

} /* namespace recsys */
