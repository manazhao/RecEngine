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
public:
	/// entity id list for each type of entity
	map<int8_t, set<int64_t> > m_type_ent_ids;
	///
	vector<map<int8_t, vector<Interact> > > m_ent_type_interacts;
	vector<set<int64_t> > m_id_id_map;
public:
	Dataset(int64_t const& maxId = 0);
	void add_entity(ushort const& type, int64_t const& id);
	void add_entity_interaction(ushort const& type, int64_t const& from_ent_id,
			Interact const& interact);
	void build_interaction_lookup();
	~Dataset(){

	}
};

}

#endif /* DATASET_H_ */
