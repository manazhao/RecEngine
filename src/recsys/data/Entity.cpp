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

Entity::SharedData Entity::init_shared_data() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref(AppConfig::ref().m_sql_conf);
		string
				createTbSql =
						"CREATE TABLE IF NOT EXISTS entity (id VARCHAR(255) NOT NULL, mapped_id  INT NOT NULL, type TINYINT(1) NOT NULL, value TEXT, INDEX (mapped_id), PRIMARY KEY(id) )";
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
						"SELECT * FROM entity where mapped_id = ?"));
		sharedData.m_updateStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"UPDATE entity SET value = ? WHERE mapped_id = ?"));
		sharedData.m_maxIdStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT MAX(mapped_id) AS max_id FROM entity"));
		inited = true;
		Entity::m_shared_data = sharedData;
		return sharedData;
	}
	return Entity::m_shared_data;
}

//// static members of Entity class
Entity::SharedData Entity::m_shared_data;
Entity::name_id_map Entity::m_name_id_map;
Entity::id_name_map Entity::m_id_name_map;
Entity::entity_ptr_map Entity::m_entity_ptr_map;
Entity::mapped_id_type Entity::m_max_id = 0;
///

Entity::Entity(size_t const& id,js::Object const& val,
		bool memoryMode) :m_memory_mode(memoryMode),
	m_mapped_id(id), m_type(ENT_DEFAULT), m_desc(val)  {

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

unsigned int Entity::_get_next_mapped_id() {
	if (m_memory_mode) {
		if(m_entity_ptr_map.empty()){
			return 0;
		}
		return ++m_max_id;
	} else {
		prepared_statement_ptr& maxIdQueryStmtPtr =
				Entity::m_shared_data.m_maxIdStmtPtr;
		auto_ptr<ResultSet> rs(maxIdQueryStmtPtr->executeQuery());
		rs->next();
		if(rs->isNull("max_id")){
			return 0;
		}
		m_max_id =  rs->getInt("max_id");
		return ++m_max_id;
	}
}

bool Entity::retrieve() {
	string jsonStr;
	bool found = false;
	bool byOrigId = !m_id.empty();
	if (!m_memory_mode) {
		string jsonStr;
		if(byOrigId){
			prepared_statement_ptr& queryStmtPtr = m_shared_data.m_queryStmtPtr;
			queryStmtPtr->setString(1,m_id);
			queryStmtPtr->setInt(2,m_type);
			auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
			if(rs->next()){
				/// get the mapped id
				m_mapped_id = rs->getInt("mapped_id");
				jsonStr = rs->getString("value");
				m_desc = string_to_json(jsonStr);
				found = true;
			}
		}else{
			prepared_statement_ptr& queryStmtPtr = m_shared_data.m_queryByIdStmtPtr;
			queryStmtPtr->setInt64(1,m_mapped_id);
			auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
			if(rs->next()){
				m_id = rs->getString("id");
				m_type = rs->getInt("type");
				jsonStr = rs->getString("value");
				m_desc = string_to_json(jsonStr);
				found = true;
			}
		}

	} else {
		/// try to retrieve from the map
		if(byOrigId){
			/// get the composite key
			m_comp_key = create_composit_key(m_id,m_type);
			/// check the existence of the key
			if(m_name_id_map.find(m_comp_key) != m_name_id_map.end()){
				m_mapped_id = m_name_id_map[m_comp_key];
				*this = *(m_entity_ptr_map[m_mapped_id]);
				found = true;
			}
		}else{
			if(m_entity_ptr_map.find(m_mapped_id) != m_entity_ptr_map.end()){
				*this = *(m_entity_ptr_map[m_mapped_id]);
				found = true;
			}
		}
	}
	return found;
}

void Entity::get_mapped_id(string const& name, int8_t const& type, bool& exist, size_t& mappedId, bool memoryMode){
	if(memoryMode){
		/// get the composite key
		string cKey = create_composit_key(name,type);
		exist = m_name_id_map.find(cKey) != m_name_id_map.end();
		if(exist){
			mappedId = Entity::m_name_id_map[cKey];
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
			resultEntityPtr = m_entity_ptr_map[m_mapped_id];
			/// replacement
			resultEntityPtr->m_desc = bk.m_desc;
		}else{
			prepared_statement_ptr& updateStmtPtr =
					Entity::m_shared_data.m_updateStmtPtr;
			updateStmtPtr->setString(1, json_to_string(bk.m_desc));
			updateStmtPtr->setInt(2, m_mapped_id);
			updateStmtPtr->executeUpdate();
			/// replacement
			m_desc = bk.m_desc;
			resultEntityPtr = entity_ptr(new Entity(*this));
		}
	}else {
		m_mapped_id = _get_next_mapped_id();
		resultEntityPtr = entity_ptr(new Entity(*this));
		if (m_memory_mode) {
			/// update the name <-> id lookup table
			m_name_id_map[m_comp_key] = m_mapped_id;
			m_id_name_map[m_mapped_id] = m_comp_key;
			m_entity_ptr_map[m_mapped_id]= resultEntityPtr;
		} else {
			prepared_statement_ptr& insertStmtPtr =
					Entity::m_shared_data.m_insertStmtPtr;
			insertStmtPtr->setString(1, m_id);
			insertStmtPtr->setInt(2, m_mapped_id);
			insertStmtPtr->setInt(3, m_type);
			insertStmtPtr->setString(4, json_to_string(m_desc));
			insertStmtPtr->execute();
		}
	}
	return resultEntityPtr;
}

ostream& operator<<(ostream& oss, Entity const& entity) {
	oss << "id:" << entity.m_id  << "-" << entity.m_mapped_id << ",type:" << entity.m_type << ",value:" << json_to_string(
			entity.m_desc) << endl;
	return oss;
}

}

