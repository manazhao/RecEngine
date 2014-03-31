/*
 * EntityInteraction.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include "EntityInteraction.h"

namespace recsys {

EntityInteraction::type_entity_interact_map
		EntityInteraction::m_type_entity_interact_map;

EntityInteraction::SharedData EntityInteraction::init_shared_data() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref();
		string
				createTbSql =
						"CREATE TABLE IF NOT EXISTS entity_interaction ("
						"from_id INT NOT NULL, from_type TINYINT NOT NULL, to_id INT NOT NULL, to_type TINYINT NOT NULL,"
						"INDEX(from_id,from_type),"
						"value TEXT)";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO entity_interaction (from_id,from_type,to_id,to_type,value) VALUES (?,?,?,?,?)"));
		sharedData.m_queryByFromStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT * FROM entity_interaction WHERE from_id = ? AND from_type = ?"));
		sharedData.m_queryByToStmtPtr
				= prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT * FROM entity_interaction WHERE to_id = ? AND to_type = ?"));
		inited = true;
		return sharedData;
	}
	return EntityInteraction::m_sharedData;
}

EntityInteraction::SharedData EntityInteraction::m_sharedData =
		EntityInteraction::init_shared_data();

EntityInteraction::EntityInteraction(ushort const& type, js::Object const& val,
		bool memoryMode) :
	m_type(type), m_val(val), m_memory_mode(memoryMode) {
	// TODO Auto-generated constructor stub

}

Entity::entity_ptr EntityInteraction::_index_entity(string const& id,
		ushort type, js::Object const& val) {
	Entity entity(id, type, val,m_memory_mode);
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

EntityInteraction::entity_interact_vec EntityInteraction::query(
		string const& entityName, ushort const& entityType, bool memoryMode, bool isFrom) {
	// get the mapped id given the entityName and entityType
	Entity::entity_ptr entityPtr = Entity::query_by_nameAndType(entityName,
			entityType, memoryMode);
	return query(entityPtr->m_mapped_id, entityType,memoryMode);
}

EntityInteraction::entity_interact_vec EntityInteraction::query(
		size_t const& entityId, ushort const& entityType, bool memoryMode, bool isFrom) {
	static entity_interact_vec empty_vec;
	if(memoryMode){
		if (m_type_entity_interact_map[entityType].find(entityId)
				!= m_type_entity_interact_map[entityType].end()) {
			return m_type_entity_interact_map[entityType][entityId];
		}else{
			return empty_vec;
		}
	}else{
		prepared_statement_ptr queryStmtPtr = (isFrom ? m_sharedData.m_queryByFromStmtPtr : m_sharedData.m_queryByToStmtPtr);
		queryStmtPtr->setInt64(1,entityId);
		queryStmtPtr->setInt(2,entityType);
		auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
		while(rs->next()){

		}
	}
	return empty_vec;
}

EntityInteraction::entity_interact_ptr EntityInteraction::index_if_not_exist() {
	/// make sure both ends are properly added
	assert(m_from_entity && m_to_entity);
	///create the entity
	entity_interact_ptr entityInteractPtr(new EntityInteraction(*this));
	if(m_memory_mode){
		if(m_type_entity_interact_map[m_from_entity->m_type].find(m_from_entity->m_mapped_id) == m_type_entity_interact_map[m_from_entity->m_type].end()){
			m_type_entity_interact_map[m_from_entity->m_type][m_from_entity->m_mapped_id] = entity_interact_vec_ptr(new vector<entity_interact_ptr>());
		}
		m_type_entity_interact_map[m_from_entity->m_type][m_from_entity->m_mapped_id]->push_back(
				entityInteractPtr);
		m_type_entity_interact_map[m_to_entity->m_type][m_to_entity->m_mapped_id]->push_back(
				entityInteractPtr);
		return entityInteractPtr;
	}else{
		/// insert to database
		prepared_statement_ptr insertStmtPtr = m_sharedData.m_insertStmtPtr;
		insertStmtPtr->setInt64(1,m_from_entity->m_mapped_id);
		insertStmtPtr->setInt(2,m_from_entity->m_type);
		insertStmtPtr->setInt64(3,m_to_entity->m_mapped_id);
		insertStmtPtr->setInt(4,m_to_entity->m_type);
		insertStmtPtr->setString(5,json_to_string(m_val));
		insertStmtPtr->execute();
	}
	return entityInteractPtr;
}

EntityInteraction::~EntityInteraction() {
	// TODO Auto-generated destructor stub
}

}
