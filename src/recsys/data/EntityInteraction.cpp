/*
 * EntityInteraction.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include "EntityInteraction.h"
#include "AppConfig.h"

namespace recsys {

EntityInteraction::entity_interact_map
		EntityInteraction::m_entity_type_interact_map;

EntityInteraction::SharedData EntityInteraction::init_shared_data() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref(AppConfig::ref().m_sql_conf);
		string createTbSql = "CREATE TABLE IF NOT EXISTS entity_interaction ("
			"from_id INT NOT NULL, to_id INT NOT NULL, type TINYINT NOT NULL"
			"PRIMARY KEY(from_id,to_id),"
			"value NUMERIC,"
			"desc TEXT)";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO entity_interaction (from_id,to_id,type,value, desc) VALUES (?,?,?,?,?)"));
		sharedData.m_queryByFromStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT * FROM entity_interaction WHERE from_id = ? AND type = ?"));
		sharedData.m_queryByToStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT * FROM entity_interaction WHERE to_id = ? AND type = ?"));
		sharedData.m_queryStmtPtr = prepared_statement_ptr(
				SQL_INST.m_connection->prepareStatement(
						"SELECT * FROM entity_interaction WHERE from_id = ? AND to_id = ?"
				)
		);
		inited = true;
		EntityInteraction::m_sharedData = sharedData;
		return sharedData;
	}
	return EntityInteraction::m_sharedData;
}

EntityInteraction::SharedData EntityInteraction::m_sharedData;
EntityInteraction::id_id_map EntityInteraction::m_id_id_map;

EntityInteraction::EntityInteraction(ushort const& type, float val,
		js::Object const& desc, bool memoryMode) :
	m_type(type), m_val(val), m_desc(desc),
			m_memory_mode(memoryMode) {
}

EntityInteraction::EntityInteraction(ushort const& type,
		js::Object const& desc, bool memoryMode) :
	m_type(type), m_val(0), m_desc(desc),
			m_memory_mode(memoryMode) {
}


Entity::entity_ptr EntityInteraction::_index_entity(string const& id,
		ushort type, js::Object const& val) {
	Entity entity(id, type, val, m_memory_mode);
	Entity::entity_ptr entityPtr = entity.index_if_not_exist();
	return entityPtr;
}

void EntityInteraction::add_from_entity(string const& id, ushort type,
		js::Object const& val) {
	/// index the entity by its internal id
	m_from_entity = _index_entity(id, type, val);
}

void EntityInteraction::add_to_entity(string const& id, ushort type,
		js::Object const& val) {
	m_to_entity = _index_entity(id, type, val);
}

EntityInteraction::entity_interact_vec_ptr EntityInteraction::query(
		string const& entityName, ushort const& entityType,
		ushort const& intType, bool memoryMode, bool isFrom) {
	// get the mapped id given the entityName and entityType
	entity_interact_vec_ptr resultVecPtr;
	Entity entity(entityName, entityType, js::Object(), memoryMode);
	if (entity.retrieve()) {
		resultVecPtr = query(entity.m_mapped_id,intType,
				memoryMode, isFrom);
	}
	return resultVecPtr;
}

EntityInteraction::entity_interact_vec_ptr EntityInteraction::query(
		size_t const& entityId, ushort const& intType, bool memoryMode,
		bool isFrom) {
	entity_interact_vec_ptr resultVecPtr;
	if (memoryMode) {
		if (m_entity_type_interact_map.find(entityId)
				!= m_entity_type_interact_map.end()
				&& m_entity_type_interact_map[entityId].find(intType)
						!= m_entity_type_interact_map[entityId].end())
			resultVecPtr = m_entity_type_interact_map[entityId][intType];
	} else {
		prepared_statement_ptr queryStmtPtr =
				(isFrom ? m_sharedData.m_queryByFromStmtPtr
						: m_sharedData.m_queryByToStmtPtr);
		queryStmtPtr->setInt64(1, entityId);
		queryStmtPtr->setInt(2, intType);
		auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
		resultVecPtr.reset(new entity_interact_vec());
		while (rs->next()) {
			ushort type = rs->getInt("type");
			float val = rs->getDouble("value");
			string desc = rs->getString("desc");
			entity_interact_ptr tmpPtr(
					new EntityInteraction(type, val, string_to_json(desc),
							memoryMode));
			Entity::mapped_id_type fromId = rs->getInt64("from_id");
			Entity::mapped_id_type toId = rs->getInt64("to_id");
			tmpPtr->m_from_entity = Entity::entity_ptr(
					new Entity(fromId, js::Object(), memoryMode));
			tmpPtr->m_from_entity->retrieve();
			tmpPtr->m_to_entity = Entity::entity_ptr(
					new Entity(toId, js::Object(), memoryMode));
			tmpPtr->m_to_entity->retrieve();
			resultVecPtr->push_back(tmpPtr);
		}
	}
	return resultVecPtr;
}

bool EntityInteraction::entity_interact_exist(
		Entity::mapped_id_type const& fromId, Entity::mapped_id_type& toId, bool memoryMode) {
	if(memoryMode){
		return m_id_id_map[fromId].find(toId) != m_id_id_map[fromId].end();
	}else{
		/// make sql query
		prepared_statement_ptr& queryStmt = m_sharedData.m_queryStmtPtr;
		queryStmt->setInt64(1,fromId);
		queryStmt->setInt64(2,toId);
		auto_ptr<ResultSet> rs(queryStmt->executeQuery());
		return rs->next();
	}
}

EntityInteraction::entity_interact_ptr EntityInteraction::index_if_not_exist() {
	/// make sure both ends are properly added
	assert(m_from_entity && m_to_entity);
	///create the entity
	entity_interact_ptr entityInteractPtr(new EntityInteraction(*this));
	bool fromExist = entity_interact_exist(m_from_entity->m_mapped_id,
			m_to_entity->m_mapped_id,m_memory_mode);
	bool toExist = entity_interact_exist(m_to_entity->m_mapped_id,
			m_from_entity->m_mapped_id,m_memory_mode);

	if (m_memory_mode) {
		if (!fromExist) {
			if(m_entity_type_interact_map[m_from_entity->m_mapped_id].find(m_type) == m_entity_type_interact_map[m_from_entity->m_mapped_id].end())
				m_entity_type_interact_map[m_from_entity->m_mapped_id][m_type] = entity_interact_vec_ptr(new entity_interact_vec());
			m_entity_type_interact_map[m_from_entity->m_mapped_id][m_type]->push_back(entityInteractPtr);
			m_id_id_map[m_from_entity->m_mapped_id].insert(m_to_entity->m_mapped_id);
		}
		/// bidirectional graph
		if (!toExist) {
			if(m_entity_type_interact_map[m_to_entity->m_mapped_id].find(m_type) == m_entity_type_interact_map[m_to_entity->m_mapped_id].end())
				m_entity_type_interact_map[m_to_entity->m_mapped_id][m_type] = entity_interact_vec_ptr(new entity_interact_vec());
			m_entity_type_interact_map[m_to_entity->m_mapped_id][m_type]->push_back(entityInteractPtr);
			m_id_id_map[m_to_entity->m_mapped_id].insert(m_from_entity->m_mapped_id);
		}
		return entityInteractPtr;
	} else {
		/// insert to database
		if(!fromExist){
			prepared_statement_ptr insertStmtPtr = m_sharedData.m_insertStmtPtr;
			insertStmtPtr->setInt64(1, m_from_entity->m_mapped_id);
			insertStmtPtr->setInt64(2, m_to_entity->m_mapped_id);
			insertStmtPtr->setInt(3, m_to_entity->m_type);
			insertStmtPtr->setDouble(4,m_val);
			insertStmtPtr->setString(5, json_to_string(m_desc));
			insertStmtPtr->execute();
		}
	}
	return entityInteractPtr;
}

EntityInteraction::~EntityInteraction() {
	// TODO Auto-generated destructor stub
}

ostream& operator<<(ostream& oss, EntityInteraction const& rhs) {
	oss << "Edge: [" << rhs.m_from_entity->m_id << "-"
			<< rhs.m_from_entity->m_mapped_id << "]" << " -> " << "["
			<< rhs.m_to_entity->m_id << "-" << rhs.m_to_entity->m_mapped_id
			<< "], type:" << rhs.m_type << ", desc:" << json_to_string(
			rhs.m_desc);
	return oss;
}

}
