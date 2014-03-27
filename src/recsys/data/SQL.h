/*
 * SQLSerialize.h
 *
 *  Created on: Mar 8, 2014
 *      Author: qzhao2
 */

#ifndef SQLSERIALIZE_H_
#define SQLSERIALIZE_H_

#include <string>
#include <map>
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
using namespace sql;
using namespace std;

namespace recsys {

/**
 * only one instance of SQLSerialize is allowed.
 * The constructor is protected to achieve this.
 */
class SQL {
	friend class Feature;
	friend class Entity;
	friend class UserRecommendation;
	friend class UserActivity;
protected:
	string m_host;
	string m_userName;
	string m_password;
	string m_dbName;
	Driver* m_driver;
	Connection* m_connection;
	Statement* m_statement;
protected:
	SQL();
	SQL(SQL const& rhs);
	SQL operator=(SQL const& rhs);
	bool _initConnection();
	void _createEntityTables();
public:
	static SQL& ref() {
		static SQL instance;
		return instance;
	}
	void setSchema(string const& schema);
	bool execute(string const& stmtStr);
	virtual ~SQL();
};

} /* namespace recsys */

#endif /* SQLSERIALIZE_H_ */
