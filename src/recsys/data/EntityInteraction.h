/*
 * EntityInteraction.h
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#ifndef ENTITYINTERACTION_H_
#define ENTITYINTERACTION_H_

#include "Entity.h"
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
using namespace std;

namespace recsys {

class EntityInteraction {
	friend ostream& operator<<(ostream& oss, EntityInteraction const& rhs);
public:
	enum INTERACTION_TYPE{
		VIEW_ITEM = 0,
		VIEW_LIST = 1 ,
		VIEW_USER = 2,
		VIEW_REC_LIST = 3,
		CLICK_URL = 4,
		CLICK_ITEM = 5,
		CLICK_LIST = 6,
		CLICK_REC_ENTRY = 7,
		RATE_ITEM = 8,
		RATE_LIST = 9,
		COMMENT_ITEM = 10,
		COMMENT_LIST = 11,
		ADD_FEATURE = 12
	};
	struct SharedData {
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryByFromStmtPtr;
		prepared_statement_ptr m_queryByToStmtPtr;
		prepared_statement_ptr m_queryStmtPtr;
	};

public:
	/// pointer of EntityInteraction
	typedef boost::shared_ptr<EntityInteraction> entity_interact_ptr;
	typedef boost::shared_ptr<char> char_ptr;
	/// index EntityInteraction by entity ids
	typedef vector<entity_interact_ptr> entity_interact_vec;
	typedef boost::shared_ptr<entity_interact_vec> entity_interact_vec_ptr;
	typedef map<int8_t,entity_interact_vec_ptr> type_interact_map;
	typedef map<Entity::mapped_id_type,map<int8_t,entity_interact_vec_ptr> > entity_type_interact_map;
	typedef map<Entity::mapped_id_type,set<Entity::mapped_id_type> > id_id_map;
public:
	Entity::entity_ptr m_from_entity;
	Entity::entity_ptr m_to_entity;
	int8_t m_type;
	/// direct value
	float m_val;
	js::Object m_desc;
	bool m_memory_mode;
public:
	static SharedData m_sharedData;
	static entity_type_interact_map m_entity_type_interact_map;
	static id_id_map m_id_id_map;
protected:
	Entity::entity_ptr _index_entity(string const& id, int8_t type, js::Object const& val);
	entity_interact_vec_ptr _create_vec_if_not_exist(int8_t const& entType, size_t const& entId);
public:
	EntityInteraction(int8_t const& type, float val, js::Object const& desc = js::Object(), bool memoryMode = true);
	EntityInteraction(int8_t const& type,js::Object const& desc = js::Object(), bool memoryMode = true);
	inline void set_from_entity(Entity::entity_ptr const& fromEntityPtr){
		m_from_entity = fromEntityPtr;
	}
	inline void set_to_entity(Entity::entity_ptr const& toEntityPtr){
		m_to_entity = toEntityPtr;
	}

	void add_from_entity(string const& id, int8_t type, js::Object const& val = js::Object());
	void add_to_entity(string const& id, int8_t type, js::Object const& val = js::Object());
	inline int8_t get_type() const{
		return m_type;
	}
	entity_interact_ptr index_if_not_exist();
	virtual ~EntityInteraction();
public:
	static SharedData init_shared_data();
	static bool entity_interact_exist(Entity::mapped_id_type const& fromId, Entity::mapped_id_type& toId, bool memoryMode);
	static entity_interact_vec_ptr query(string const& entityName, int8_t const& entityType, int8_t const& intType, bool memoryMode = true, bool isFrom = true);
	static entity_interact_vec_ptr query(size_t const& entityId, int8_t const& intType, bool memoryMode = true, bool isFrom = true);
};

ostream& operator<<(ostream& oss, EntityInteraction const& rhs);
void test_entity_interaction();

}

#endif /* ENTITYINTERACTION_H_ */
