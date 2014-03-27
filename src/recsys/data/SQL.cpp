/*
 * SQL.cpp
 *
 *  Created on: Mar 8, 2014
 *      Author: qzhao2
 */

#include "SQL.h"

namespace recsys {

SQL::SQL() :
		m_host("localhost"), m_userName("root"), m_password("StU8e@uh"),m_dbName("amazon"), m_driver(
				NULL), m_connection(NULL), m_statement(NULL) {
	// TODO Auto-generated constructor stub
	cout << "-----------initialize database connection-----------" << endl;
	assert(_initConnection());
	setSchema(m_dbName);
	cout << "-----------initialization done!-----------" << endl;
}

void SQL::setSchema(string const& schema){
	cout << "changing database to:" + schema << endl;
	m_statement->execute("create database IF NOT EXISTS " + schema);
	m_statement->execute("use " + schema);
}

bool SQL::_initConnection(){
	try{
		m_driver = get_driver_instance();
	}catch(sql::SQLException& e){
		cerr << "Could not get database driver. Error message:" << e.what() << endl;
		return false;
	}
	try{
		m_connection = m_driver->connect(m_host,m_userName,m_password);
	}catch(SQLException& e){
		cerr << "Could not connect to the database with the provided credentials. Error message:" << e.what() << endl;
		return false;
	}
	m_statement = m_connection->createStatement();
	return true;
}

bool SQL::execute(string const& stmtStr) {
	try {
		m_statement->execute(stmtStr);
	} catch (SQLException& e) {
		cerr << "error in executing:" << stmtStr << " . Error message:"
				<< e.what() << endl;
		return false;
	}
	return true;
}

SQL::~SQL() {
	// TODO Auto-generated destructor stub
	// release all sql resources
	if(m_statement){
		delete m_statement;
	}
	if(m_connection){
		delete m_connection;
	}
}

} /* namespace recsys */
