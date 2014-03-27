/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: qzhao2
 */
#include "Entity.h"
#include <memory>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>

using namespace boost;

namespace recsys {

Entity::type_name_map Entity::initTypeNameMap() {
	type_name_map typeNameMap;
	typeNameMap[ENT_USER] = "user";
	typeNameMap[ENT_ITEM] = "item";
	return typeNameMap;
}

Entity::type_name_map Entity::m_typeNameMap = Entity::initTypeNameMap();

Entity::SharedData Entity::initSharedData() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref();
		string createTbSql =
				"CREATE TABLE IF NOT EXISTS entity (id VARCHAR(255) NOT NULL, mapped_id  INT NOT NULL, type TINYINT(1) NOT NULL, value TEXT, INDEX (mapped_id), PRIMARY KEY(id,type) )";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO entity (id,mapped_id,type,value) VALUES (?,?,?,?)"));
		sharedData.m_queryStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * FROM entity where id = ? AND type = ?"));
		sharedData.m_updateStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"UPDATE entity SET mapped_id = ?, value = ? WHERE id = ? AND type= ?"));
		sharedData.m_maxIdStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT MAX(mapped_id) AS max_id FROM entity WHERE type = ?"));

		inited = true;
		return sharedData;
	}
	return Entity::m_sharedData;
}

Entity::SharedData Entity::m_sharedData = Entity::initSharedData();

unsigned int Entity::_get_max_mapped_id(bool &isNull) {
	prepared_statement_ptr& maxIdQueryStmtPtr =
			Entity::m_sharedData.m_maxIdStmtPtr;
	maxIdQueryStmtPtr->setInt(1, m_type);
	auto_ptr<ResultSet> rs(maxIdQueryStmtPtr->executeQuery());
	rs->next();
	isNull = rs->isNull("max_id");
	return rs->getInt("max_id");
}

bool Entity::exist() {
	prepared_statement_ptr& queryStmtPtr = Entity::m_sharedData.m_queryStmtPtr;
	queryStmtPtr->setString(1, m_id);
	queryStmtPtr->setInt(2, m_type);
	auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
	return rs->next();
}

void Entity::read() {
	prepared_statement_ptr& queryStmtPtr = Entity::m_sharedData.m_queryStmtPtr;
	queryStmtPtr->setString(1, m_id);
	queryStmtPtr->setInt(2, m_type);
	auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
	if (rs->next()) {
		/// get the mapped id
		m_mapped_id = rs->getInt("mapped_id");
		string kvMapStr = rs->getString("value");
		/// recover as pairs
		vector<string> kvPairs;
		split(kvPairs, kvMapStr, is_any_of("|"));
		for (size_t i = 0; i < kvPairs.size(); i++) {
			string kvStr = kvPairs[i];
			vector<string> kvs;
			split(kvs, kvStr, is_any_of("-"));
			assert(kvs.size() == 2);
			size_t key = lexical_cast<size_t>(kvs[0]);
			string value = kvs[1];
			m_kvMap[key] = value;
		}
	}
}

size_t Entity::attachFeature(string const& name, size_t const& type,
		string const& value) {
	/// first check the existence of the feature
	Feature feat(name, type);
	feat.write();
	feat.read();
	setKV(feat.getId(), value);
	return feat.getId();
}

void Entity::write() {
	/// get the value of the entity
	string entityValue = getEntityValue();
	if (exist()) {
		/// just update an existing entity
		read();
		prepared_statement_ptr& updateStmtPtr =
				Entity::m_sharedData.m_updateStmtPtr;
		bool isNull;
		updateStmtPtr->setInt(1, m_mapped_id);
		updateStmtPtr->setString(2, entityValue);
		updateStmtPtr->setString(3, m_id);
		updateStmtPtr->setInt(4, m_type);
		updateStmtPtr->executeUpdate();

	} else {
		prepared_statement_ptr& insertStmtPtr =
				Entity::m_sharedData.m_insertStmtPtr;
		bool isNull;
		m_mapped_id = _get_max_mapped_id(isNull);
		if(!isNull){
			m_mapped_id++;
		}
		insertStmtPtr->setString(1, m_id);
		insertStmtPtr->setInt(2, m_mapped_id);
		insertStmtPtr->setInt(3, m_type);
		insertStmtPtr->setString(4, entityValue);
		insertStmtPtr->execute();
	}
}

string Entity::getEntityValue() const {
	// generate string out of the map
	std::stringstream ss;
	for (map<size_t, string>::const_iterator iter = m_kvMap.begin();
			iter != m_kvMap.end(); ++iter) {
		/// separate the key and value by special character '-'
		ss << (iter == m_kvMap.begin() ? "" : "|") << iter->first << "-"
				<< iter->second;
	}
	return ss.str();
}

Feature::SharedData Feature::initSharedData() {
	static bool inited = false;
	if (!inited) {
		/// create the table
		SQL& SQL_INST = SQL::ref();
		string createTbSql =
				"CREATE TABLE IF NOT EXISTS feature(id INT(4) AUTO_INCREMENT NOT NULL, name VARCHAR(128), type TINYINT(1) NOT NULL, PRIMARY KEY(id) )";
		assert(SQL_INST.execute(createTbSql));
		/// create the prepared statement as well
		SharedData sharedData;
		sharedData.m_insertStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"INSERT INTO feature(name,type) VALUES(?,?)"));
		sharedData.m_queryByNameStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * from feature WHERE name = ?"));
		inited = true;
		return sharedData;
	}
	return Feature::m_sharedSqlData;
}

Feature::SharedData Feature::m_sharedSqlData = Feature::initSharedData();

bool Feature::exist() {
	prepared_statement_ptr& queryStmtPtr =
			Feature::m_sharedSqlData.m_queryByNameStmtPtr;
	queryStmtPtr->setString(1, m_name);
	auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
	return rs->next();
}

void Feature::read() {
	prepared_statement_ptr& queryStmtPtr =
			Feature::m_sharedSqlData.m_queryByNameStmtPtr;
	queryStmtPtr->setString(1, m_name);
	auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
	if (rs->next()) {
		m_id = rs->getInt(1);
		m_type = rs->getInt(2);
	}
}

void Feature::write() {
	if (!exist()) {
		Feature::m_sharedSqlData.m_insertStmtPtr->setString(1, m_name);
		Feature::m_sharedSqlData.m_insertStmtPtr->setInt(2, m_type);
		try {
			Feature::m_sharedSqlData.m_insertStmtPtr->execute();
		} catch (SQLException& e) {
			cerr << "error in inserting Feature object: " << e.what() << endl;
		}
	} else {
		cout << "feature - [" << m_name << "] exists, abort the insertion"
				<< endl;
	}
}

ostream& operator<<(ostream& oss, Entity const& entity) {
	oss << "id:" << entity.m_id << ",value:" << entity.getEntityValue() << endl;
	return oss;
}

void test_feature() {
	Feature feat("gender", Feature::CATEGORY);
	feat.write();
}

void test_entity() {
	Entity entity("zq", Entity::ENT_USER);
	entity.attachFeature("age", Feature::NUMERICAL, "30");
	entity.attachFeature("gender", Feature::CATEGORY, "1");
	entity.read();
	entity.attachFeature("occupation", Feature::CATEGORY, "student");
	entity.write();
	entity = Entity("someone",Entity::ENT_USER);
	entity.attachFeature("age",Feature::NUMERICAL,"20");
	entity.attachFeature("gender",Feature::CATEGORY,"0");
	entity.attachFeature("occupation",Feature::CATEGORY,"engineer");
	entity.write();
	entity = Entity("xbox",Entity::ENT_ITEM);
	entity.attachFeature("merchant",Feature::CATEGORY,"ms");
	entity.attachFeature("price",Feature::NUMERICAL,"499");
	entity.write();
	entity = Entity("0x59373",Entity::ENT_ITEM);
	entity.attachFeature("merchant",Feature::CATEGORY,"amazon");
	entity.attachFeature("price",Feature::NUMERICAL,"79.99");
	entity.write();
	entity = Entity("0xdfdfdf",Entity::ENT_ITEM);
	entity.attachFeature("merchant",Feature::CATEGORY,"amazon");
	entity.attachFeature("price",Feature::NUMERICAL,"79.99");
	entity.write();


	cout << entity;
}

}

