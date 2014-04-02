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

namespace recsys {
/**
 * application configuration.
 * the settings could be passed through command line arguments or
 * load from configuration files
 */
class AppConfig {
	friend class EntityInteraction;
	friend class Entity;
	friend class UserActivity;
	friend class UserRecommendation;
protected:
	string m_sql_conf;
	bool m_use_db;
protected:
	AppConfig();
	AppConfig(AppConfig &rhs);
	AppConfig& operator=(AppConfig const& rhs);
	void _init_helper();
public:
	static AppConfig& ref() {
		static AppConfig APP_CONFIG;
		return APP_CONFIG;
	}
	void init(int argc,char** argv);
	virtual ~AppConfig();
};
}
#endif /* APPCONFIG_H_ */
