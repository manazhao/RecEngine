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
#include <boost/shared_ptr.hpp>
#include <cppconn/prepared_statement.h>

using namespace sql;
using namespace std;

namespace recsys {

/**
 * only one instance of SQLSerialize is allowed.
 * The constructor is protected to achieve this.
 */
class SQL {
public:
	string m_host;
	string m_userName;
	string m_password;
	string m_dbName;
	string m_conf_file;
	Driver* m_driver;
	Connection* m_connection;
	Statement* m_statement;
protected:
	SQL(string const& confFile);
	SQL(SQL const& rhs);
	SQL operator=(SQL const& rhs);
	bool _init_connection();
	bool _load_from_conf();
	void _create_entity_tables();
	void _set_schema(string const& schema);
public:
	static SQL& init_instance(string const& confFile){
		return ref(confFile);
	}
	static SQL& ref(string const& confFile) {
		static boost::shared_ptr<SQL> instance_ptr;
		if(!instance_ptr.get() || confFile != instance_ptr->m_conf_file){
			instance_ptr.reset(new SQL(confFile));
		}
		return *instance_ptr;
	}
	bool execute(string const& stmtStr);
	virtual ~SQL();
};

typedef boost::shared_ptr<sql::PreparedStatement> prepared_statement_ptr;


} /* namespace recsys */

#endif /* SQLSERIALIZE_H_ */
