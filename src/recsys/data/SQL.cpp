/*
 * SQL.cpp
 *
 *  Created on: Mar 8, 2014
 *      Author: qzhao2
 */

#include "SQL.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include "recsys/data/json/elements.h"
#include "recsys/data/json/reader.h"

using namespace boost;
using namespace json;
namespace fs = boost::filesystem;

namespace recsys {

SQL::SQL(string const& confFile) :m_conf_file(confFile)
		,m_driver(
				NULL), m_connection(NULL), m_statement(NULL) {
	// TODO Auto-generated constructor stub
	assert(_load_from_conf());
	cout << "-----------initialize database connection-----------" << endl;
	assert(_initConnection());
	cout << "-----------initialization done!-----------" << endl;
}

bool SQL::_load_from_conf(){
	/// load the settings from configuration file
	/// ensure the existence of the configuration
	bool status = true;
	if(!fs::exists(m_conf_file)){
		cerr << "file:" << m_conf_file << " does not exist" << endl;
		status = false;
	}else{
		/// parse the configuration file in json format
		fstream fs(m_conf_file.c_str());
		if(!fs.good()){
			cerr << "failed to open the sql configuration file:" << m_conf_file << endl;
			status = false;
		}else{
			Object confObj;
			json::Reader::Read(confObj,fs);
			json::String user = (confObj["user"]);
			m_userName = user.Value();
			json::String host = confObj["host"];
			m_host = host.Value();
			json::String password = confObj["password"];
			m_password = password.Value();
			json::String schema = confObj["schema"];
			m_dbName = schema.Value();
		}
		fs.close();
	}
	return status;
}

void SQL::_set_schema(string const& schema){
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
		cerr << "Could not connect to the database with the provided credential. Error message:" << e.what() << endl;
		return false;
	}
	m_statement = m_connection->createStatement();
	/// change to the schema
	_set_schema(m_dbName);
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
