/*
 * Object.h
 *
 *  Created on: Mar 7, 2014
 *      Author: qzhao2
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <boost/unordered_map.hpp>
#include <string>
#include <map>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <recsys/data/json/elements.h>
#include <recsys/data/json/reader.h>
#include <recsys/data/json/writer.h>
#include <cppconn/prepared_statement.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "SQL.h"

using namespace std;
using namespace boost;
using namespace sql;
/// just namespace shortcut
namespace js=json;

namespace recsys {

struct JSObjectWrapper{
	js::Object m_obj;
	template <class T>
	JSObjectWrapper& add(string const &key, T const& val){
		/// cast to string anyway
		string strVal = lexical_cast<string>(val);
		m_obj[key] = js::String(strVal);
		return *this;
	}
	operator js::Object (){
		return m_obj;
	}
	template<class T>
	T operator[](string key) const{
		js::String val = m_obj[key];
		return lexical_cast<T>(val.Value());
	}


};

string json_to_string(js::Object const& jsObj);
js::Object string_to_json(string const& jsonStr);

enum FEAT_TYPE {FEAT_CAT,FEAT_REAL};
typedef shared_ptr<PreparedStatement> prepared_statement_ptr;
typedef unsigned short ushort;
class EntityInteraction;
class Entity {
	friend ostream& operator<<(ostream&, EntityInteraction const&);
	friend ostream& operator<<(ostream&, Entity const&);
	friend class EntityInteraction;
	friend class HandleDataHandler;
public:
	enum ENTITY_TYPE {
		ENT_DEFAULT, ENT_USER, ENT_ITEM,ENT_FEATURE
	};
public:
	struct SharedData {
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryStmtPtr;
		prepared_statement_ptr m_queryByIdStmtPtr;
		prepared_statement_ptr m_updateStmtPtr;
		prepared_statement_ptr m_maxIdStmtPtr;
	};
public:
	typedef shared_ptr<Entity> entity_ptr;
	/// use char_ptr to store all kinds of values, e.g. float, int and etc.
	typedef shared_ptr<char> char_ptr;
	typedef size_t mapped_id_type;
	// map internal id to composite entity name (name + type)
	typedef map<mapped_id_type,string> id_name_map;
	typedef unordered_map<string,mapped_id_type> name_id_map;
	/// maximum mapped id
	/// retrieve the entity based on the internal id (an integer)
	typedef map<mapped_id_type,entity_ptr> entity_ptr_map;
public:
	static name_id_map m_name_id_map;
	static id_name_map m_id_name_map;
	static SharedData m_shared_data;
	static entity_ptr_map m_entity_map;
	static mapped_id_type m_max_id;
public:
	/// whether keep all objects in memory
	bool m_memory_mode;
	string m_id;
	/// a unique id that would be used internally
	/// all entities (user, item, feature, etc) would be mapped in the same space
	size_t m_mapped_id;
	/// entity type
	ushort m_type;
	/// composite key
	string m_comp_key;
	/// use a json object to hold all information about the entity
	/// e.g. attributes(features) about a user or an item
	js::Object m_desc;
protected:
public:
	Entity(string const& id, size_t const& type, js::Object const& val = js::Object(), bool memoryMode = true) :m_memory_mode(memoryMode),
			m_id(id),m_mapped_id(0),m_type(type),m_desc(val) {
	}
	Entity(size_t const& id,js::Object const& val = js::Object(), bool memoryMode = true);

	inline string get_id() {
		return m_id;
	}
	inline size_t get_mapped_id(){
		return m_mapped_id;
	}
	bool retrieve();
	static void get_mapped_id(string const& name, ushort const& type, bool& exist, size_t& mappedId, bool memoryMode = true);
	unsigned int _get_next_mapped_id(bool &exist);

	static string create_composit_key(string const& key, ushort type){
		stringstream ss;
		ss << type << "_" << key;
		return ss.str();
	}
	static void break_composite_key(string const& cKey, string& key, ushort& type){
		vector<string> splits;
		boost::split(splits,cKey,boost::is_any_of("_"));
		key = splits[1];
		type = boost::lexical_cast<ushort>(splits[0]);
	}
	entity_ptr index_if_not_exist();
	virtual ~Entity() {
	}
public:
	static SharedData init_shared_data();
};

ostream& operator<<(ostream& oss, Entity const&);



}

#endif /* OBJECT_H_ */
