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

Entity::type_name_map Entity::init_type_name_map() {
	type_name_map typeNameMap;
	typeNameMap[ENT_USER] = "user";
	typeNameMap[ENT_ITEM] = "item";
	return typeNameMap;
}

Entity::type_name_map Entity::m_typeNameMap = Entity::init_type_name_map();

Entity::SharedData Entity::init_shared_data() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref();
		string
				createTbSql =
						"CREATE TABLE IF NOT EXISTS entity (id VARCHAR(255) NOT NULL, mapped_id  INT NOT NULL, type TINYINT(1) NOT NULL, value TEXT, INDEX (mapped_id), PRIMARY KEY(id,type) )";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO entity (id,mapped_id,type,value) VALUES (?,?,?,?)"));
		sharedData.m_queryStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * FROM entity where id = ? AND type = ?"));
		sharedData.m_updateStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"UPDATE entity SET mapped_id = ?, value = ? WHERE id = ? AND type= ?"));
		sharedData.m_maxIdStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT MAX(mapped_id) AS max_id FROM entity WHERE type = ?"));

		inited = true;
		return sharedData;
	}
	return Entity::m_sharedData;
}

Entity::SharedData Entity::m_sharedData = Entity::init_shared_data();

Entity::entity_ptr Entity::query_by_nameAndType(string const& name, ushort const& type, bool memoryMode = true){
	Entity ent(name,type,memoryMode);
	if(ent.exist()){
		ent.read();
		if(memoryMode){
			return m_type_entity_map[type][ent.m_mapped_id];
		}
		/// we have to create one
		return entity_ptr(new Entity(this));
	}
	/// null pointer returned
	return entity_ptr();
}

string json_to_string(js::Object& jsObj){
	/// encode the json object to string
	stringstream ss;
	js::Writer::Write(jsObj,ss);
	return ss.str();
}

js::Object string_to_json(string const& jsonStr){
	stringstream ss;
	ss << jsonStr;
	js::Object obj;
	js::Reader::Read(obj,jsonStr);
	return obj;
}


unsigned int Entity::_get_max_mapped_id(bool &isNull) {
	if(m_memory_mode){
		if(m_type_max_id_map.find(m_type) == m_type_max_id_map.end()){
			isNull = true;
			return 0;
		}else{
			return m_type_max_id_map[m_type];
		}
	}else{
		prepared_statement_ptr& maxIdQueryStmtPtr =
				Entity::m_sharedData.m_maxIdStmtPtr;
		maxIdQueryStmtPtr->setInt(1, m_type);
		auto_ptr<ResultSet> rs(maxIdQueryStmtPtr->executeQuery());
		rs->next();
		isNull = rs->isNull("max_id");
		return rs->getInt("max_id");
	}
}

bool Entity::exist() {
	if (!m_memory_mode) {
		prepared_statement_ptr& queryStmtPtr =
				Entity::m_sharedData.m_queryStmtPtr;
		queryStmtPtr->setString(1, m_id);
		queryStmtPtr->setInt(2, m_type);
		auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
		return rs->next();
	} else {
		return !(m_type_name_id_map[m_type].find(m_id)
				== m_type_name_id_map.end());
	}
}

void Entity::read() {
	string jsonStr;
	if(!m_memory_mode){
		prepared_statement_ptr& queryStmtPtr = Entity::m_sharedData.m_queryStmtPtr;
		queryStmtPtr->setString(1, m_id);
		queryStmtPtr->setInt(2, m_type);
		auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
		if (rs->next()) {
			/// get the mapped id
			m_mapped_id = rs->getInt("mapped_id");
			jsonStr = rs->getString("value");
		}
	}else{
		/// try to retrieve from the map
		if(m_type_name_id_map[m_type].find(m_id) != m_type_name_id_map.end()){
			m_mapped_id = m_type_name_id_map[m_type][m_id];
			/// retrieve the json string from the entity map
			jsonStr = m_type_entity_map[m_type].find(m_mapped_id);
		}
	}
	if(!jsonStr.empty()){
		m_json_value = string_to_json(jsonStr);
	}
}

void Entity::write() {
	if (exist()) {
		/// just update an existing entity
		read();
		if(m_memory_mode){
			/// replace the entity
			m_type_entity_map[m_type][m_mapped_id] = shared_ptr<Entity>(this);
		}else{
			prepared_statement_ptr& updateStmtPtr =
					Entity::m_sharedData.m_updateStmtPtr;
			updateStmtPtr->setInt(1, m_mapped_id);
			updateStmtPtr->setString(2, jsonStr);
			updateStmtPtr->setString(3, m_id);
			updateStmtPtr->setInt(4, m_type);
			updateStmtPtr->executeUpdate();
		}

	} else {
		bool isNull;
		m_mapped_id = _get_max_mapped_id(isNull);
		if(!isNull){
			m_mapped_id++;
		}
		if(m_memory_mode){
			m_type_name_id_map[m_id] = m_mapped_id;
			m_type_id_name_map[m_mapped_id] = m_id;
			m_type_entity_map[m_mapped_id] = shared_ptr<Entity>(new Entity(*this));
		}else{
			prepared_statement_ptr& insertStmtPtr =
					Entity::m_sharedData.m_insertStmtPtr;
			insertStmtPtr->setString(1, m_id);
			insertStmtPtr->setInt(2, m_mapped_id);
			insertStmtPtr->setInt(3, m_type);
			insertStmtPtr->setString(4, json_to_string(m_json_value));
			insertStmtPtr->execute();
		}
	}
}

ostream& operator<<(ostream& oss, Entity const& entity) {
	oss << "id:" << entity.m_id << ",value:" << json_to_string(entity.m_json_value) << endl;
	return oss;
}


void test_entity() {
	Entity entity("zq", Entity::ENT_USER);
	entity.attachFeature("age", Feature::NUMERICAL, "30");
	entity.attachFeature("gender", Feature::CATEGORY, "1");
	entity.read();
	entity.attachFeature("occupation", Feature::CATEGORY, "student");
	entity.write();
	entity = Entity("someone", Entity::ENT_USER);
	entity.attachFeature("age", Feature::NUMERICAL, "20");
	entity.attachFeature("gender", Feature::CATEGORY, "0");
	entity.attachFeature("occupation", Feature::CATEGORY, "engineer");
	entity.write();
	entity = Entity("xbox", Entity::ENT_ITEM);
	entity.attachFeature("merchant", Feature::CATEGORY, "ms");
	entity.attachFeature("price", Feature::NUMERICAL, "499");
	entity.write();
	entity = Entity("0x59373", Entity::ENT_ITEM);
	entity.attachFeature("merchant", Feature::CATEGORY, "amazon");
	entity.attachFeature("price", Feature::NUMERICAL, "79.99");
	entity.write();
	entity = Entity("0xdfdfdf", Entity::ENT_ITEM);
	entity.attachFeature("merchant", Feature::CATEGORY, "amazon");
	entity.attachFeature("price", Feature::NUMERICAL, "79.99");
	entity.write();
	cout << entity;
}

}

