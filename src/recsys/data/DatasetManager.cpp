/*
 * DatasetManager.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: qzhao2
 */

#include <recsys/data/DatasetManager.h>

namespace recsys {

void DatasetManager::generate_datasets(){
	cout << "generate the whole dataset" << endl;
	DatasetExt& allDataset = m_datasets[rt::DSType::DS_ALL];
	for (Entity::entity_ptr_map::iterator iter =
			Entity::m_entity_ptr_map.begin();
			iter != Entity::m_entity_ptr_map.end(); ++iter) {
		///  get the mapped id and type
		Entity::mapped_id_type id = iter->first;
		int8_t type = iter->second->m_type;
		allDataset.add_entity(type,id);
	}
	allDataset.ent_type_interacts.assign(Entity::m_entity_ptr_map.size(),
			map<int8_t, std::vector<Interact> >());
	for (size_t i = 0; i < allDataset.ent_type_interacts.size(); i++) {
		map<int8_t, std::vector<Interact> > tmpInteracts;
		_get_entity_interacts(tmpInteracts, i);
		allDataset.ent_type_interacts[i] = tmpInteracts;
	}
	/// allocate space for subsets
	for(size_t i = 1; i < m_datasets.size(); i++){
		m_datasets[i] = DatasetExt(allDataset.ent_type_interacts.size());
	}
	DatasetExt tmpDataset(allDataset.ent_type_interacts.size());
	cout << "generate coldstart dataset" << endl;
	/// generate coldstart dataset
	_split_by_user(0.75,allDataset,tmpDataset,m_datasets[rt::DSType::DS_CS]);
	/// generate training and testing dataset
	cout << "generate model training and evaluation datasets" << endl;
	_split_by_rating(0.75,tmpDataset,m_datasets[rt::DSType::DS_TRAIN], m_datasets[rt::DSType::DS_TEST]);
}

void DatasetManager::_get_entity_interacts(
		std::map<int8_t, std::vector<Interact> > & _return,
		const int64_t entId) {
	EntityInteraction::type_interact_map& entIntMap =
			EntityInteraction::m_entity_type_interact_map[entId];
	for (EntityInteraction::type_interact_map::iterator iter =
			entIntMap.begin(); iter != entIntMap.end(); ++iter) {
		/// extract the interaction value
		int8_t intType = iter->first;
		EntityInteraction::entity_interact_vec_ptr& interactVecPtr =
				iter->second;
		if (interactVecPtr) {
			for (EntityInteraction::entity_interact_vec::iterator iter1 =
					interactVecPtr->begin(); iter1 < interactVecPtr->end();
					++iter1) {
				EntityInteraction& tmpInteract = **iter1;
				Interact resultInteract;
				Entity::mapped_id_type fromId =
						tmpInteract.m_from_entity->m_mapped_id;
				Entity::mapped_id_type toId =
						tmpInteract.m_to_entity->m_mapped_id;
				resultInteract.ent_id = (fromId == entId ? toId : fromId);
				resultInteract.ent_val = tmpInteract.m_val;
				_return[intType].push_back(resultInteract);
			}
		}
	}
}

void DatasetManager::_split_by_rating(float splitRatio, DatasetExt & inputDataset, DatasetExt& subSet1, DatasetExt& subSet2){
	std::set<int64_t> const& userIdSet = inputDataset.type_ent_ids[Entity::ENT_USER];
	for(set<int64_t>::const_iterator iter = userIdSet.begin(); iter != userIdSet.end(); ++iter){
		int64_t userId = *iter;
		subSet1.add_entity(Entity::ENT_USER,userId);
		subSet2.add_entity(Entity::ENT_USER,userId);
		vector<rt::Interact> const& ratings = inputDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		vector<size_t> idxVec;
		for(size_t i = 0; i < ratings.size(); i++){
			idxVec.push_back(i);
		}
		std::random_shuffle(idxVec.begin(),idxVec.end());
		size_t split1Size = (size_t)(ratings.size() * splitRatio);
		for(size_t i = 0; i < idxVec.size(); i++){
			size_t intIdx = idxVec[i];
			DatasetExt& curDataset = (i < split1Size ? subSet1 : subSet2);
			Interact interact = ratings[intIdx];
			int64_t itemId = interact.ent_id;
			curDataset.add_entity(Entity::ENT_ITEM,itemId);
			curDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM].push_back(interact);
			interact.ent_id = userId;
			curDataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM].push_back(interact);
		}
	}
	/// add back features
	/// now add user/item <-> feature interactions
	DatasetExt* splits[] = {&subSet1,&subSet2};
	for(size_t i =0 ; i < 2; i++){
		vector<int64_t> userItemIds;
		userItemIds.insert(userItemIds.end(),splits[i]->ent_ids.begin(),splits[i]->ent_ids.end());
		for(size_t j = 0; j < userItemIds.size(); j++){
			int64_t entityId = userItemIds[j];
			vector<rt::Interact> const& featureInteracts = inputDataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
			for(vector<rt::Interact>::const_iterator iter = featureInteracts.begin(); iter < featureInteracts.end(); iter++){
				rt::Interact  interact = *iter;
				int64_t featId = interact.ent_id;
				splits[i]->add_entity(Entity::ENT_FEATURE,featId);
				splits[i]->ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE].push_back(interact);
				interact.ent_id = entityId;
				splits[i]->ent_type_interacts[featId][EntityInteraction::ADD_FEATURE].push_back(interact);
			}
		}
	}
}

void DatasetManager::_split_by_user(float splitRatio, DatasetExt & inputDataset, DatasetExt& subSet1, DatasetExt& subSet2){
	assert(splitRatio > 0 && splitRatio < 1);
	std::set<int64_t> const& userIdSet = inputDataset.type_ent_ids[Entity::ENT_USER];
	vector<int64_t> userIds;
	userIds.assign(userIdSet.begin(), userIdSet.end());
	/// shuffle the user ids
	std::random_shuffle(userIds.begin(), userIds.end());
	size_t split1Size = (size_t)(userIds.size() * splitRatio);
	/// first add the user and item entities and their rating interactions
	for(size_t i = 0; i < userIds.size(); i++){
		int64_t userId = userIds[i];
		map<int8_t, vector<rt::Interact> >& userInteracts =
				inputDataset.ent_type_interacts[userId];
		vector<rt::Interact>& ratings = userInteracts[EntityInteraction::RATE_ITEM];
		DatasetExt& curDataset = (i < split1Size? subSet1 : subSet2);
		curDataset.add_entity(Entity::ENT_USER,userId);
		for(vector<rt::Interact>::const_iterator iter = ratings.begin(); iter < ratings.end(); ++iter){
			Interact  interact = *iter;
			int64_t itemId = interact.ent_id;
			curDataset.add_entity(Entity::ENT_ITEM, itemId);
			/// also add the entity interaction
			curDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM].push_back(interact);
			interact.ent_id = userId;
			curDataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM].push_back(interact);
		}
	}
	/// now add user/item <-> feature interactions
	DatasetExt* splits[] = {&subSet1,&subSet2};
	for(size_t i =0 ; i < 2; i++){
		vector<int64_t> userItemIds;
		userItemIds.insert(userItemIds.end(),splits[i]->ent_ids.begin(),splits[i]->ent_ids.end());
		for(size_t j = 0; j < userItemIds.size(); j++){
			int64_t entityId = userItemIds[j];
			vector<rt::Interact> const& featureInteracts = inputDataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
			for(vector<rt::Interact>::const_iterator iter = featureInteracts.begin(); iter < featureInteracts.end(); iter++){
				Interact  interact = *iter;
				int64_t featId = interact.ent_id;
				splits[i]->add_entity(Entity::ENT_FEATURE,featId);
				splits[i]->ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE].push_back(interact);
				interact.ent_id = entityId;
				splits[i]->ent_type_interacts[featId][EntityInteraction::ADD_FEATURE].push_back(interact);
			}
		}
	}
}

DatasetManager::DatasetManager():m_datasets(4) {
}

DatasetManager::~DatasetManager() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
