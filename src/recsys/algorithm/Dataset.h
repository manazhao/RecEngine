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

using namespace std;
namespace recsys {

class Dataset {
protected:
	void _filter_entity_interaction_helper(int8_t const& type,
			int64_t const& from_ent_id, Interact const& interact);
public:
	Dataset(int64_t const& maxId = 0);
	void add_entity(int8_t const& type, int64_t const& id);
	inline bool entity_exist(int64_t const& id) {
		return m_ent_ids.find(id) != m_ent_ids.end();
	}
	void filter_interaction(
			vector<map<int8_t, vector<Interact> > > const& entTypeInteractions);
	~Dataset() {

	}
public:
	/// entity id list for each type of entity
	map<int8_t, set<int64_t> > m_type_ent_ids;
	set<int64_t> m_ent_ids;
	vector<map<int8_t, vector<Interact> > > m_ent_type_interacts;
};

}

#endif /* DATASET_H_ */
