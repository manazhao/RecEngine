/*
 * BasicImpl.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef BASICIMPL_HPP_
#define BASICIMPL_HPP_
#include "Basic.hpp"
#include "boost/lexical_cast.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace recsys {
namespace graph {

ostream& operator<<(ostream& oss, NullValue const& val) {
	return oss;
}

//// member functions for Entity class

inline Entity::Entity(ent_type const& type, ent_idx_type const& idx) :
	m_type(type), m_idx(idx) {
}

inline Entity::ent_idx_type const& Entity::get_idx() const {
	return m_idx;
}

inline Entity::ent_type const& Entity::get_type() const {
	return m_type;
}

/// members for Entity<T>
template<typename DataTypeT>
Entity_T<DataTypeT>::Entity_T(Entity::ent_type const& type,
		Entity::ent_idx_type const& idx, ent_value_type const& value) :
	Entity(type, idx), m_value(value) {
}

template<typename DataTypeT>
inline typename Entity_T<DataTypeT>::ent_value_type const& Entity_T<DataTypeT>::get_value() const {
	return m_value;
}

template<typename DataValueT>
ostream& operator <<(ostream& oss, Entity_T<DataValueT> const& ent) {
	oss << ent.get_idx() << "," << ent.get_type() << "," << ent.get_value();
	return oss;
}

ostream& operator <<(ostream& oss, Entity_T<char> const& ent) {
	oss << ent.get_idx() << "," << ent.get_type() << ","
			<< static_cast<int> (ent.get_value());
	return oss;
}

ostream& operator <<(ostream& oss, Entity_T<NullValue> const& ent) {
	oss << ent.get_idx() << "," << ent.get_type();
	return oss;
}

/// members for DefaultComposeKey

string DefaultComposeKey::operator()(Entity::ent_type const& type,
		string const& name) const {
	// simply paste type and name by _
	stringstream ss;
	ss << type << "_" << name;
	return ss.str();
}

string DefaultComposeKey::operator()(Entity const& ent1, Entity const& ent2) const {
	stringstream ss;
	ss << ent1.get_idx() << "_" << ent2.get_idx();
	return ss.str();
}

/// define operator() for DefaultDecomposeKey
void DefaultDecomposeKey::operator()(string const& cKey,
		Entity::ent_type& oType, string& oName) const {
	vector < string > splits;
	boost::split(splits, cKey, boost::is_any_of("_"));
	stringstream ss;
	for (size_t i = 1; i < splits.size(); i++) {
		ss << (i == 1 ? "" : "_") << splits[i];
	}
	oName = ss.str();
	int type = boost::lexical_cast<int>(splits[0]);
	oType = static_cast<Entity::ent_type> (type);
}

}
}


#endif /* BASICIMPL_HPP_ */
