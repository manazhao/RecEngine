/*
 * AppConfig.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: qzhao2
 */

#include <recsys/data/AppConfig.h>
#include <boost/filesystem.hpp>
#include "Entity.h"
#include "EntityInteraction.h"
#include "UserRecommendation.h"
#include "SQL.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace recsys{

AppConfig::AppConfig():m_use_db(false) {
	// TODO Auto-generated constructor stub

}

void AppConfig::init(int argc, char** argv){
	po::options_description desc("Allowed options");
	desc.add_options()
			("help","help message on use this application")
			("use-db","use mysql database")
			("mysql-conf",po::value<string>(),"mysql database configuration file in JSON format");
	po::variables_map vm;
	po::store(po::parse_command_line(argc,argv,desc),vm);
	if(vm.count("help")){
		cout << desc << "\n";
		exit(1);
	}
	if(vm.count("use-db")){
		if(vm.count("mysql-conf")){
			string sql_conf = vm["mysql-conf"].as<string>();
			m_use_db = true;
			m_sql_conf = sql_conf;
		}else{
			cout << "mysql configuration file MUST be specified if the use-db switch is on" << endl;
			cout << desc << "\n";
			exit(1);
		}
	}
	AppConfig::ref()._init_helper();
}


void AppConfig::_init_helper(){
	/// initialize the database
	cout << "============================ setup application ============================" << endl;
	if(m_use_db){
		assert(boost::filesystem::exists(m_sql_conf));
		/// initialize the entities that are defined so far
		SQL::init_instance(m_sql_conf);
		/// create necessary tables and prepare statements for some classes
		cout << "---create tables and prepared sql statements for some classes" << endl;
		Entity::init_shared_data();
		EntityInteraction::init_shared_data();
		UserRecommendation::init_shared_data();
	}
}

AppConfig::~AppConfig() {
	// TODO Auto-generated destructor stub
}

}
