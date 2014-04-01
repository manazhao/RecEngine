/*
 * EntityInteraction.cpp
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#include "EntityInteraction.h"

namespace recsys {

EntityInteraction::type_entity_interact_map EntityInteraction::m_type_entity_interact_map;

EntityInteraction::SharedData EntityInteraction::init_shared_data() {
	static bool inited = false;
	if (!inited) {
		/// create the entity table
		SQL& SQL_INST = SQL::ref();
		string createTbSql =
				"CREATE TABLE IF NOT EXISTS entity_interaction ("
						"from_id INT NOT NULL, from_type TINYINT NOT NULL, to_id INT NOT NULL, to_type TINYINT NOT NULL,"
						"PRIMARY KEY(from_id,from_type,to_id,to_type),"
						"value TEXT)";
		SQL_INST.m_statement->execute(createTbSql);
		/// create the PreparedStatement for entity insertion
		SharedData sharedData;
		sharedData.m_insertStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"INSERT INTO entity_interaction (from_id,from_type,to_id,to_type,value) VALUES (?,?,?,?,?)"));
		sharedData.m_queryByFromStmtPtr =
				prepared_statement_ptr(
						SQL_INST.m_connection->prepareStatement(
								"SELECT * FROM entity_interaction WHERE from_id = ? AND from_type = ?"));
		sharedData.m_queryByToStmtPtr =
				prepared_statement_ptr(
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
		string const& entityName, ushort const& entityType, bool memoryMode,
		bool isFrom) {
	// get the mapped id given the entityName and entityType
	entity_interact_vec_ptr resultVecPtr;
	Entity entity(entityName, entityType, js::Object(), memoryMode);
	if (entity.retrieve()) {
		resultVecPtr = query(entity.m_mapped_id, entity.m_type, memoryMode,
				isFrom);
	}
	return resultVecPtr;
}

EntityInteraction::entity_interact_vec_ptr EntityInteraction::query(
		size_t const& entityId, ushort const& entityType, bool memoryMode,
		bool isFrom) {
	entity_interact_vec_ptr resultVecPtr;
	if (memoryMode) {
		if (m_type_entity_interact_map[entityType].find(entityId)
				!= m_type_entity_interact_map[entityType].end())
			resultVecPtr = m_type_entity_interact_map[entityType][entityId];
	} else {
		prepared_statement_ptr queryStmtPtr = (
				isFrom ?
						m_sharedData.m_queryByFromStmtPtr :
						m_sharedData.m_queryByToStmtPtr);
		queryStmtPtr->setInt64(1, entityId);
		queryStmtPtr->setInt(2, entityType);
		auto_ptr<ResultSet> rs(queryStmtPtr->executeQuery());
		resultVecPtr.reset(new entity_interact_vec());
		while (rs->next()) {
			ushort type = rs->getInt("type");
			string valStr = rs->getString("val");
			entity_interact_ptr tmpPtr(
					new EntityInteraction(type, string_to_json(valStr),
							memoryMode));
			size_t fromId = rs->getInt64("from_id");
			size_t toId = rs->getInt64("to_id");
			ushort fromType = rs->getInt("from_type");
			ushort toType = rs->getInt("to_type");
			tmpPtr->m_from_entity = Entity::entity_ptr(
					new Entity(fromId, fromType, js::Object(), memoryMode));
			tmpPtr->m_from_entity->retrieve();
			tmpPtr->m_to_entity = Entity::entity_ptr(
					new Entity(toId, toType, js::Object(), memoryMode));
			tmpPtr->m_to_entity->retrieve();
			resultVecPtr->push_back(tmpPtr);
		}
	}
	return resultVecPtr;
}

EntityInteraction::entity_interact_vec_ptr EntityInteraction::_create_vec_if_not_exist(
		ushort const& entType, size_t const& entId) {
	if (m_type_entity_interact_map[entType].find(entId)
			== m_type_entity_interact_map[entType].end()) {
		m_type_entity_interact_map[entType][entId] = entity_interact_vec_ptr(
				new vector<entity_interact_ptr>());
	}
	return m_type_entity_interact_map[entType][entId];
}

EntityInteraction::entity_interact_ptr EntityInteraction::index_if_not_exist() {
	/// make sure both ends are properly added
	assert(m_from_entity && m_to_entity);
	///create the entity
	entity_interact_ptr entityInteractPtr(new EntityInteraction(*this));
	if (m_memory_mode) {
		entity_interact_vec_ptr fromEntVecPtr = _create_vec_if_not_exist(
				m_from_entity->m_type, m_from_entity->m_mapped_id);
		entity_interact_vec_ptr toEntVecPtr = _create_vec_if_not_exist(
				m_to_entity->m_type, m_to_entity->m_mapped_id);
		fromEntVecPtr->push_back(entityInteractPtr);
		toEntVecPtr->push_back(entityInteractPtr);
		return entityInteractPtr;
	} else {
		/// insert to database
		prepared_statement_ptr insertStmtPtr = m_sharedData.m_insertStmtPtr;
		insertStmtPtr->setInt64(1, m_from_entity->m_mapped_id);
		insertStmtPtr->setInt(2, m_from_entity->m_type);
		insertStmtPtr->setInt64(3, m_to_entity->m_mapped_id);
		insertStmtPtr->setInt(4, m_to_entity->m_type);
		insertStmtPtr->setString(5, json_to_string(m_val));
		insertStmtPtr->execute();
	}
	return entityInteractPtr;
}

EntityInteraction::~EntityInteraction() {
	// TODO Auto-generated destructor stub
}

ostream& operator<<(ostream& oss, EntityInteraction const& rhs) {
	oss << "Edge: [" << rhs.m_from_entity->m_id << "-" << rhs.m_from_entity->m_mapped_id << "]"
			<< " -> " << "[" << rhs.m_to_entity->m_id << "-"
			<< rhs.m_to_entity->m_mapped_id << "], type:" << rhs.m_type
			<< ", value:" << json_to_string(rhs.m_val);
	return oss;
}

}
