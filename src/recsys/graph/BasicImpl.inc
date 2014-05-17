/*
 * BasicImpl.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#include "boost/lexical_cast.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace recsys {
namespace graph {

ostream& operator<<(ostream& oss, NullValue const& val) {
	return oss;
}


/// members for DefaultComposeKey

string DefaultComposeKey::operator()(ENTITY_TYPE const& type,
		string const& name) const {
	// simply paste type and name by _
	stringstream ss;
	ss << type << "_" << name;
	return ss.str();
}

template<typename T1, typename T2>
string DefaultComposeKey::operator()(T1 const& t1, T2 const& t2) const {
	// simply paste type and name by _
	stringstream ss;
	ss << t1 << "_" << t2;
	return ss.str();
}


/// define operator() for DefaultDecomposeKey
void DefaultDecomposeKey::operator()(string const& cKey,
		ENTITY_TYPE& oType, string& oName) const {
	vector < string > splits;
	boost::split(splits, cKey, boost::is_any_of("_"));
	stringstream ss;
	for (size_t i = 1; i < splits.size(); i++) {
		ss << (i == 1 ? "" : "_") << splits[i];
	}
	oName = ss.str();
	int type = boost::lexical_cast<int>(splits[0]);
	oType = static_cast<ENTITY_TYPE> (type);
}

}
}
