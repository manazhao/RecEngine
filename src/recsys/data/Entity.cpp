/*
 * Object.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: qzhao2
 */
#include "Entity.h"
#include "AppConfig.h"
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
		SQL& SQL_INST = SQL::ref(AppConfig::ref().m_sql_conf);
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
		sharedData.m_queryByIdStmtPtr  = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * FROM entity where mapped_id = ? AND type = ?"));
		sharedData.m_updateStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"UPDATE entity SET value = ? WHERE mapped_id = ? AND type= ?"));
		sharedData.m_maxIdStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT MAX(mapped_id) AS max_id FROM entity WHERE type = ?"));
		inited = true;
		Entity::m_sharedData = sharedData;
		return sharedData;
	}
	return Entity::m_sharedData;
}

Entity::SharedData Entity::m_sharedData;

Entity::type_entity_map Entity::m_type_entity_map;
/// store the maximum id for a given entity type
Entity::type_max_id_map Entity::m_type_max_id_map;
/// entity name to internal id lookup table
Entity::type_name_id_map Entity::m_type_name_id_map;
/// internal id to entity name lookup
Entity::type_id_name_map Entity::m_type_id_name_map;

Entity::Entity(size_t const& id, ushort const& type, js::Object const& val,
		bool memoryMode) :m_memory_mode(memoryMode),
	m_mapped_id(id), m_type(type), m_value(val)  {

}

string json_to_string(js::Object const& jsObj) {
	/// encode the json object to string
	stringstream ss;
	js::Writer::Write(jsObj, ss);
	string str = ss.str();
	str.erase(
			std::remove_if(str.begin(), str.end(), ::isspace),
			str.end());
	return str;
}

js::Object string_to_json(string const& jsonStr) {
	stringstream ss;
	ss << jsonStr;
	js::Object obj;
	js::Reader::Read(obj, ss);
	return obj;
}

unsigned int Entity::_get_max_mapped_id(bool &isNull) {
	if (m_memory_mode) {
		if (m_type_max_id_map.find(m_type) == m_type_max_id_map.end()) {
			isNull = true;
			return 0;
		} else {
			return m_type_max_id_map[m_type];
		}
	} else {
		prepared_statement_ptr& maxIdQueryStmtPtr =
				Entity::m_sharedData.m_maxIdStmtPtr;
		maxIdQueryStmtPtr->setInt(1, m_type);
		auto_ptr<ResultSet> rs(maxIdQueryStmtPtr->executeQuery());
		rs->next();
		isNull = rs->isNull("max_id");
		return rs->getInt("max_id");
	}
}

bool Entity::retrieve() {
	string jsonStr;
	bool found = false;
	bool byOrigId = !m_id.empty();
	if (!m_memory_mode) {
		string jsonStr;
		if(byOrigId){
			prepared_statement_ptr& queryStmtPtr = m_sharedData.m_queryStmtPtr;
			queryStmtPtr->setString(1,m_id);
			queryStmtPtr->setInt(2,m_type);
			auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
			if(rs->next()){
				/// get the mapped id
				m_mapped_id = rs->getInt("mapped_id");
				jsonStr = rs->getString("value");
				m_value = string_to_json(jsonStr);
				found = true;
			}
		}else{
			prepared_statement_ptr& queryStmtPtr = m_sharedData.m_queryByIdStmtPtr;
			queryStmtPtr->setInt64(1,m_mapped_id);
			queryStmtPtr->setInt(2,m_type);
			auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
			if(rs->next()){
				m_id = rs->getString("id");
				jsonStr = rs->getString("value");
				m_value = string_to_json(jsonStr);
				found = true;
			}
		}

	} else {
		/// try to retrieve from the map
		if(byOrigId){
			if (m_type_name_id_map[m_type].find(m_id)
					!= m_type_name_id_map[m_type].end()) {
				m_mapped_id = m_type_name_id_map[m_type][m_id];
				/// retrieve the json string from the entity map
//				cout << "original:" << *this << endl;
//				cout << "from map:" <<  *(m_type_entity_map[m_type][m_mapped_id]) << endl;
				*this = *(m_type_entity_map[m_type][m_mapped_id]);
				found = true;
			}
		}else{
			if(m_type_entity_map[m_type].find(m_mapped_id) != m_type_entity_map[m_type].end()){
				*this = *(m_type_entity_map[m_type][m_mapped_id]);
				found = true;
			}
		}
	}
	return found;
}

void Entity::get_mapped_id(string const& name, ushort const& type, bool& exist, size_t& mappedId, bool memoryMode){
	if(memoryMode){
		exist = Entity::m_type_name_id_map[type].find(name) != Entity::m_type_name_id_map[type].end();
		if(exist){
			mappedId = Entity::m_type_name_id_map[type][name];
		}
	}else{
		/// retrieve from database
		Entity ent(name,type,js::Object(),memoryMode);
		exist = ent.retrieve();
		if(exist){
			mappedId = ent.m_mapped_id;
		}
	}
}

Entity::entity_ptr Entity::index_if_not_exist() {
	entity_ptr resultEntityPtr;
	Entity bk(*this);
	if(retrieve()){
		if(m_memory_mode){
			resultEntityPtr = m_type_entity_map[m_type][m_mapped_id];
			/// replacement
			resultEntityPtr->m_value = bk.m_value;
		}else{
			prepared_statement_ptr& updateStmtPtr =
					Entity::m_sharedData.m_updateStmtPtr;
			updateStmtPtr->setString(1, json_to_string(bk.m_value));
			updateStmtPtr->setInt(2, m_mapped_id);
			updateStmtPtr->setInt(3, m_type);
			updateStmtPtr->executeUpdate();
			m_value = bk.m_value;
			resultEntityPtr = entity_ptr(new Entity(*this));
		}
	}else {
		bool isNull;
		m_mapped_id = _get_max_mapped_id(isNull);
		if (!isNull) {
			m_mapped_id++;
		}
		/// update the maximum id of a given entity type
		m_type_max_id_map[m_type] = m_mapped_id;
		resultEntityPtr = entity_ptr(new Entity(*this));
		if (m_memory_mode) {
			/// update the name <-> id lookup table
			m_type_name_id_map[m_type][m_id] = m_mapped_id;
			m_type_id_name_map[m_type][m_mapped_id] = m_id;
			m_type_entity_map[m_type][m_mapped_id] = resultEntityPtr;
		} else {
			prepared_statement_ptr& insertStmtPtr =
					Entity::m_sharedData.m_insertStmtPtr;
			insertStmtPtr->setString(1, m_id);
			insertStmtPtr->setInt(2, m_mapped_id);
			insertStmtPtr->setInt(3, m_type);
			insertStmtPtr->setString(4, json_to_string(m_value));
			insertStmtPtr->execute();
		}
	}
	return resultEntityPtr;
}

ostream& operator<<(ostream& oss, Entity const& entity) {
	oss << "id:" << entity.m_id  << "-" << entity.m_mapped_id << ",type:" << entity.m_type << ",value:" << json_to_string(
			entity.m_value) << endl;
	return oss;
}

}

