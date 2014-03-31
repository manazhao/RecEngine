/*
 * Object.h
 *
 *  Created on: Mar 7, 2014
 *      Author: qzhao2
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <string>
#include <map>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <recsys/data/json/elements.h>
#include <recsys/data/json/reader.h>
#include <recsys/data/json/writer.h>
#include <cppconn/prepared_statement.h>
#include "SQL.h"

using namespace std;
using namespace boost;
using namespace sql;
/// just namespace shortcut
namespace js=json;

namespace recsys {

string json_to_string(js::Object& jsObj);
js::Object string_to_json(string const& jsonStr);

typedef shared_ptr<PreparedStatement> prepared_statement_ptr;
typedef unsigned short ushort;

class Entity {
	friend ostream& operator<<(ostream&, Entity const&);
public:
	enum ENTITY_TYPE {
		ENT_DEFAULT, ENT_USER, ENT_ITEM
	};
public:
	struct SharedData {
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryStmtPtr;
		prepared_statement_ptr m_updateStmtPtr;
		prepared_statement_ptr m_maxIdStmtPtr;
	};
public:
	typedef map<size_t, string> type_name_map;
	typedef shared_ptr<Entity> entity_ptr;
	/// retrieve the entity based on the internal id (an integer)
	typedef map<size_t,entity_ptr> entity_ptr_map;
	/// retrieve the entity map given entity type
	typedef map<ushort,entity_ptr_map> type_entity_map;
	/// keep track of the maximum id given entity type
	typedef map<ushort,size_t> type_max_id_map;
	/// map entity original name to internal integer index
	typedef map<string,size_t> name_id_map;
	typedef map<ushort,name_id_map> type_name_id_map;
	/// internal id to entity name lookup table
	typedef map<size_t,string> id_name_map;
	typedef map<ushort,id_name_map> type_id_name_map;
protected:
	static type_name_map m_typeNameMap;
	static SharedData m_sharedData;
	/// store all entities in main memory
	static type_entity_map m_type_entity_map;
	/// store the maximum id for a given entity type
	static type_max_id_map m_type_max_id_map;
	/// entity name to internal id lookup table
	static type_name_id_map m_type_name_id_map;
	/// internal id to entity name lookup
	static type_id_name_map m_type_id_name_map;
protected:
	/// whether keep all objects in memory
	bool m_memory_mode;
	string m_id;
	/// an unsigned integer id used internally.
	unsigned int m_mapped_id;
	size_t m_type;
	/// use a json object to hold all information about the entity
	/// e.g. attributes(features) about a user or an item
	js::Object m_json_value;
protected:
	unsigned int _get_max_mapped_id(bool &exist);
public:
	Entity(string const& id = "", size_t const& type = ENT_DEFAULT, bool memoryMode = true) :m_memory_mode(memoryMode),
			m_id(id),m_mapped_id(0),m_type(type) {
	}
	inline string get_id() {
		return m_id;
	}
	inline size_t get_mapped_id(){
		return m_mapped_id;
	}
	bool exist();
	void write();
	void read();
	virtual ~Entity() {
	}
public:
	static type_name_map init_type_name_map();
	static SharedData init_shared_data();
	static entity_ptr query_by_nameAndType(string const& name, ushort const& type, bool memory_mode = true);
};

ostream& operator<<(ostream& oss, Entity const&);
enum FEATURE_TYPE {
	NUMERICAL=1, CATEGORY=2, ORDINAL=3
};


void test_entity();

}

#endif /* OBJECT_H_ */
