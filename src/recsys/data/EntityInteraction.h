/*
 * EntityInteraction.h
 *
 *  Created on: Mar 31, 2014
 *      Author: manazhao
 */

#ifndef ENTITYINTERACTION_H_
#define ENTITYINTERACTION_H_

#include "Entity.h"
#include <vector>
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
	};

public:
	/// pointer of EntityInteraction
	typedef shared_ptr<EntityInteraction> entity_interact_ptr;
	/// index EntityInteraction by entity ids
	typedef vector<entity_interact_ptr> entity_interact_vec;
	typedef shared_ptr<entity_interact_vec> entity_interact_vec_ptr;
	typedef map<size_t, entity_interact_vec_ptr> entity_interact_map;
	typedef map<ushort,entity_interact_map> type_entity_interact_map;
protected:
	Entity::entity_ptr m_from_entity;
	Entity::entity_ptr m_to_entity;
	ushort m_type;
	js::Object m_val;
	bool m_memory_mode;
	static SharedData m_sharedData;
protected:
	static type_entity_interact_map m_type_entity_interact_map;
protected:
	Entity::entity_ptr _index_entity(string const& id, ushort type, js::Object const& val);
	entity_interact_vec_ptr _create_vec_if_not_exist(ushort const& entType, size_t const& entId);
public:
	EntityInteraction(ushort const& type, js::Object const& val, bool memoryMode = true);
	void add_from_entity(string const& id, ushort type, js::Object const& val = js::Object());
	void add_to_entity(string const& id, ushort type, js::Object const& val = js::Object());
	entity_interact_ptr index_if_not_exist();
	virtual ~EntityInteraction();
public:
	static SharedData init_shared_data();;
	static entity_interact_vec_ptr query(string const& entityName, ushort const& entityType, bool memoryMode = true, bool isFrom = true);
	static entity_interact_vec_ptr query(size_t const& entityId, ushort const& entityType, bool memoryMode = true, bool isFrom = true);
};

ostream& operator<<(ostream& oss, EntityInteraction const& rhs);
void test_entity_interaction();

}

#endif /* ENTITYINTERACTION_H_ */
