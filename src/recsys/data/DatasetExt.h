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
#include <recsys/data/Entity.h>
#include <recsys/data/EntityInteraction.h>
#include "recsys/thrift/cpp/HandleData.h"
using namespace ::recsys::thrift;
namespace rt=::recsys::thrift;

using namespace std;
namespace recsys {

class DatasetExt : public rt::Dataset{
public:
	map<int64_t,int8_t> m_id_type_map;
protected:
	void _filter_entity_interaction_helper(int8_t const& type,
			int64_t const& from_ent_id, Interact const& interact);
public:
	DatasetExt(int64_t const& numEntities = 0);
	void add_entity(int8_t const& type, int64_t const& id);
	inline bool entity_exist(int64_t const& id) {
		return ent_ids.find(id) != ent_ids.end();
	}
	void prepare_id_type_map();
	void verify_interaction();
	void filter_entity_interactions(
			vector<map<int8_t, vector<Interact> > > const& entTypeInteractions);
	virtual ~DatasetExt() throw() {}
};

}

#endif /* DATASET_H_ */
