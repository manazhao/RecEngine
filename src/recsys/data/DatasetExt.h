/*
 * Dataset.h
 *
 *  Created on: Apr 8, 2014
 *      Author: manazhao
 */

#ifndef DATASET_H_
#define DATASET_H_

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <boost/serialization/access.hpp>
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>
#include "recsys/thrift/cpp/DataHost.h"
using namespace ::recsys::thrift;
namespace rt = ::recsys::thrift;

using namespace std;

namespace boost {
namespace serialization {
template<class Archive>
void serialize(Archive& ar, rt::Interact & v, unsigned int version) {
	ar & v.ent_id & v.ent_val;
}
}
}

namespace recsys {
class DatasetExt: public rt::Dataset {
	friend ostream& operator <<(ostream& oss, DatasetExt const&);
public:
	map<int64_t, int8_t> m_id_type_map;
protected:
	void _filter_entity_interaction_helper(int8_t const& type,
			int64_t const& from_ent_id, Interact const& interact);
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		/// serialize every data member except for the dataset
		ar & ent_ids & ent_type_interacts & type_ent_ids;
	}

public:
	DatasetExt(int64_t const& numEntities = 0);
	void add_entity(int8_t const& type, int64_t const& id);
	inline bool entity_exist(int64_t const& id) {
		return ent_ids.find(id) != ent_ids.end();
	}
	inline bool empty() {
		return m_id_type_map.empty();
	}
	void dump_rating_interact();
	void prepare_id_type_map();
	void verify_interaction();
	virtual ~DatasetExt() throw () {
	}
};
ostream& operator <<(ostream& oss, DatasetExt const& rhs);
}

#endif /* DATASET_H_ */
