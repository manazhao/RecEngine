/*
 * SQLContainer.cpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */
#include "SQLContainer.hpp"
#include "recsys/data/AppConfig.h"
#include <memory>
using namespace std;

namespace recsys{
namespace graph{

/// default initialization of static shared data
SQLEntityContainer::SharedData SQLEntityContainer::SHARED_DATA;

//// static shared variable initialization
void SQLEntityContainer::init_shared(){
	/// initialize shared variables. Here are mainly the sql statements
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref(AppConfig::ref().m_sql_conf);
		string
				createTbSql =
						"CREATE TABLE IF NOT EXISTS entity (id INT NOT NULL, type TINYINT(1) NOT NULL, value NUMERIC, PRIMARY KEY(id) )";
		SQL_INST.m_statement->execute(createTbSql);

		SharedData& sharedData = SHARED_DATA;
		sharedData.m_insertStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO entity (id,type,value) VALUES (?,?,?)"));
		sharedData.m_queryByIdStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * FROM entity where id = ?"));
		sharedData.m_updateStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"UPDATE entity SET value = ? WHERE id = ?"));
		inited = true;
	}
}


/// non template member function

//// check whether entity exist given idx
shared_ent_ptr SQLEntityContainer::exist(Entity::ent_idx_type const& idx) const{
//	std::unique_ptr<ResultSet> rs(SHARED_DATA.m_queryByIdStmtPtr->setInt64(1,idx)->executeQuery());
}

}
}

