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
#include <cppconn/prepared_statement.h>
#include "SQL.h"

using namespace std;
using namespace boost;
using namespace sql;

namespace recsys {

typedef shared_ptr<PreparedStatement> prepared_statement_ptr;

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
protected:
	static type_name_map m_typeNameMap;
	static SharedData m_sharedData;
protected:
	string m_id;
	/// an unsigned integer id used internally.
	unsigned int m_mapped_id;
	size_t m_type;
	map<size_t, string> m_kvMap;
protected:
	unsigned int _get_max_mapped_id(bool &exist);
public:
	Entity(string const& id = "", size_t const& type = ENT_DEFAULT) :
			m_id(id),m_mapped_id(0),m_type(type) {
	}
	static type_name_map initTypeNameMap();
	static SharedData initSharedData();
	void reset() {
		m_kvMap.clear();
	}

	inline string getId() {
		return m_id;
	}

	inline unsigned int get_mapped_id(){
		return m_mapped_id;
	}

	map<size_t, string> const& getKVMap() const {
		return m_kvMap;
	}

	size_t attachFeature(string const& name, size_t const& type,
			string const& value);

	bool exist();
	string getNameByType(size_t const& type) {
		return m_typeNameMap[type];
	}

	void setKV(size_t const& key, string const& value) {
		m_kvMap[key] = value;
	}

	string getValue(size_t const& key) {
		if (m_kvMap.find(key) != m_kvMap.end()) {
			return m_kvMap[key];
		}
		return string();
	}
	string getEntityValue() const;
	void write();
	void read();
	virtual ~Entity() {
	}
};

ostream& operator<<(ostream& oss, Entity const&);

class Feature {
	struct SharedData {
		/// prepared statement
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryByNameStmtPtr;
		SharedData(PreparedStatement* stmtPtr = NULL) :
				m_insertStmtPtr(stmtPtr) {

		}
	};

public:
	enum FEATURE_TYPE {
		NUMERICAL=1, CATEGORY=2, ORDINAL=3
	};
protected:
	string m_name;
	size_t m_type;
	size_t m_id;
protected:
	static SharedData m_sharedSqlData;
public:
	Feature(string const& name, size_t const& type) :
			m_name(name), m_type(type), m_id(0) {

	}
	static SharedData initSharedData();
	bool exist();
	string getName() {
		return m_name;
	}
	size_t getType() {
		return m_type;
	}
	size_t getId() {
		return m_id;
	}
	void read();
	void write();
};
void test_feature();
void test_entity();

}

#endif /* OBJECT_H_ */
