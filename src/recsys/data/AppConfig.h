/*
 * AppConfig.h
 *
 *  Created on: Apr 1, 2014
 *      Author: qzhao2
 */

#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#include <string>
using namespace std;

/**
 * application configuration.
 * the settings could be passed through command line arguments or
 * load from configuration files
 */
class AppConfig {
public:
	string m_sql_conf;
protected:
	AppConfig();
public:
	static AppConfig ref(){
		static AppConfig APP_CONFIG;
		return APP_CONFIG;
	}
	virtual ~AppConfig();
};

#endif /* APPCONFIG_H_ */
