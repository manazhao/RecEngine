/*
 * DatasetManager.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: qzhao2
 */

#include <recsys/data/DatasetManager.h>

namespace recsys {

void DatasetManager::generate_cv_datasets() {
	/// generate the cross-validation datasets
	/// put all rating interacts in a vector
	cout << ">>> generate cross validation folds:" << m_cv_folds << endl;
	vector<Interact*> allRatingInteracts;
	vector<size_t> allRatingUsers;
	DatasetExt& completeDataset = m_datasets[rt::DSType::DS_ALL];
	size_t numOfEntity = completeDataset.ent_type_interacts.size();
	/// allocate space for cv datasets
	m_cv_datasets = vector<TrainTestPair>(m_cv_folds,TrainTestPair(numOfEntity) );
	set<int64_t> userIds = completeDataset.type_ent_ids[Entity::ENT_USER];

	for (set<int64_t>::iterator iter = userIds.begin(); iter != userIds.end();
			++iter) {
		int64_t userId = *iter;
		vector<Interact>& ratingInteracts =
				completeDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		for (vector<Interact>::iterator iter1 = ratingInteracts.begin();
				iter1 < ratingInteracts.end(); ++iter1) {
			allRatingInteracts.push_back(&(*iter1));
			allRatingUsers.push_back(userId);
		}
	}
	size_t numOfRatings = allRatingInteracts.size();
	vector<int64_t> ratingIdxVec(numOfRatings);
	for (size_t i = 0; i < numOfRatings; i++) {
		ratingIdxVec[i] = i;
	}
	/// shuffle the index
	std::random_shuffle(ratingIdxVec.begin(), ratingIdxVec.end());
	/// now assign the rating to each fold
	for (size_t i = 0; i < numOfRatings; i++) {
		size_t ratingIdx = ratingIdxVec[i];
		size_t foldIdx = ratingIdx % m_cv_folds;
		Interact ratingInteract = *(allRatingInteracts[ratingIdx]);
		int64_t userIdx = allRatingUsers[ratingIdx];
		int64_t itemIdx = ratingInteract.ent_id;
		Interact ratingInteractRev = ratingInteract;
		ratingInteractRev.ent_id = userIdx;
		for (size_t j = 0; j < m_cv_folds; j++) {
			TrainTestPair& curPair = m_cv_datasets[j];
			DatasetExt* curDataset = (
					foldIdx == j ? &(curPair.m_test) : &(curPair.m_train));
			/// add to testing dataset
			curDataset->add_entity(Entity::ENT_USER, userIdx);
			curDataset->add_entity(Entity::ENT_ITEM, itemIdx);
			/// add the interaction
			curDataset->ent_type_interacts[userIdx][EntityInteraction::RATE_ITEM].push_back(
					ratingInteract);
			//// change the destination id to user
			curDataset->ent_type_interacts[itemIdx][EntityInteraction::RATE_ITEM].push_back(
					ratingInteractRev);
		}
	}
	/// now add the feature interactions back
	for(size_t i = 0; i < m_cv_datasets.size(); i++){
		_add_feature_interactions(m_cv_datasets[i].m_train,completeDataset);
		_add_feature_interactions(m_cv_datasets[i].m_test,completeDataset);
//		m_cv_datasets[i].m_train.dump_rating_interact();
//		m_cv_datasets[i].m_test.dump_rating_interact();
	}
	cout << ">>> done!" << endl;
}

void DatasetManager::init_complete_dataset() {
	cout << ">>> generate the complete dataset" << endl;
	DatasetExt& completeDataset = m_datasets[rt::DSType::DS_ALL];
	for (Entity::entity_ptr_map::iterator iter =
			Entity::m_entity_ptr_map.begin();
			iter != Entity::m_entity_ptr_map.end(); ++iter) {
		///  get the mapped id and type
		Entity::mapped_id_type id = iter->first;
		int8_t type = iter->second->m_type;
		completeDataset.add_entity(type, id);
	}
	completeDataset.ent_type_interacts.assign(Entity::m_entity_ptr_map.size(),
			map<int8_t, std::vector<Interact> >());
	for (size_t i = 0; i < completeDataset.ent_type_interacts.size(); i++) {
		map<int8_t, std::vector<Interact> > tmpInteracts;
		_get_entity_interacts(tmpInteracts, i);
		completeDataset.ent_type_interacts[i] = tmpInteracts;
	}
	//// done
}


void DatasetManager::_add_feature_interactions(DatasetExt& dataset, DatasetExt& inputDataset){
	vector<int64_t> userItemIds;
	userItemIds.insert(userItemIds.end(), dataset.ent_ids.begin(),
			dataset.ent_ids.end());
	for (size_t i = 0; i < userItemIds.size(); i++) {
		int64_t entityId = userItemIds[i];
		vector<rt::Interact> const& featureInteracts =
				inputDataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE];
		for (vector<rt::Interact>::const_iterator iter =
				featureInteracts.begin(); iter < featureInteracts.end();
				iter++) {
			rt::Interact interact = *iter;
			int64_t featId = interact.ent_id;
			dataset.add_entity(Entity::ENT_FEATURE, featId);
			dataset.ent_type_interacts[entityId][EntityInteraction::ADD_FEATURE].push_back(
					interact);
			interact.ent_id = entityId;
			dataset.ent_type_interacts[featId][EntityInteraction::ADD_FEATURE].push_back(
					interact);
			assert(interact.ent_val);
		}
	}

}

void DatasetManager::generate_datasets() {
	/// allocate space for subsets
	DatasetExt& completeDataset = m_datasets[rt::DSType::DS_ALL];
	for (size_t i = 1; i < m_datasets.size(); i++) {
		m_datasets[i] = DatasetExt(completeDataset.ent_type_interacts.size());
	}
	DatasetExt tmpDataset(completeDataset.ent_type_interacts.size());
	cout << ">>> generate coldstart dataset" << endl;
	/// generate coldstart dataset
	_split_by_user(0.75, completeDataset, tmpDataset, m_datasets[rt::DSType::DS_CS]);
	/// generate training and testing dataset
	cout << ">>> generate training and testing datasets" << endl;
	_split_by_rating(0.75, tmpDataset, m_datasets[rt::DSType::DS_TRAIN],
			m_datasets[rt::DSType::DS_TEST]);
//	completeDataset.verify_interaction();
//	m_datasets[rt::DSType::DS_TRAIN].verify_interaction();
//	m_datasets[rt::DSType::DS_TEST].verify_interaction();
//	m_datasets[rt::DSType::DS_CS].verify_interaction();
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
				assert(resultInteract.ent_val);
				_return[intType].push_back(resultInteract);
			}
		}
	}
}

void DatasetManager::_split_by_rating(float splitRatio,
		DatasetExt & inputDataset, DatasetExt& subSet1, DatasetExt& subSet2) {
	std::set<int64_t> const& userIdSet =
			inputDataset.type_ent_ids[Entity::ENT_USER];
	for (set<int64_t>::const_iterator iter = userIdSet.begin();
			iter != userIdSet.end(); ++iter) {
		int64_t userId = *iter;
		subSet1.add_entity(Entity::ENT_USER, userId);
		subSet2.add_entity(Entity::ENT_USER, userId);
		vector<rt::Interact> const& ratings =
				inputDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM];
		vector<size_t> idxVec;
		for (size_t i = 0; i < ratings.size(); i++) {
			idxVec.push_back(i);
		}
		std::random_shuffle(idxVec.begin(), idxVec.end());
		size_t split1Size = (size_t) (ratings.size() * splitRatio);
		for (size_t i = 0; i < idxVec.size(); i++) {
			size_t intIdx = idxVec[i];
			DatasetExt& curDataset = (i < split1Size ? subSet1 : subSet2);
			Interact interact = ratings[intIdx];
			int64_t itemId = interact.ent_id;
			curDataset.add_entity(Entity::ENT_ITEM, itemId);
			curDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM].push_back(
					interact);
			interact.ent_id = userId;
			curDataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM].push_back(
					interact);
		}
	}
	/// add back features
	/// now add user/item <-> feature interactions
	DatasetExt* splits[] = { &subSet1, &subSet2 };
	for (size_t i = 0; i < 2; i++) {
		_add_feature_interactions(*(splits[i]), inputDataset);
	}
}

void DatasetManager::_split_by_user(float splitRatio, DatasetExt & inputDataset,
		DatasetExt& subSet1, DatasetExt& subSet2) {
	assert(splitRatio > 0 && splitRatio < 1);
	std::set<int64_t> const& userIdSet =
			inputDataset.type_ent_ids[Entity::ENT_USER];
	vector<int64_t> userIds;
	userIds.assign(userIdSet.begin(), userIdSet.end());
	/// shuffle the user ids
	std::random_shuffle(userIds.begin(), userIds.end());
	size_t split1Size = (size_t) (userIds.size() * splitRatio);
	/// first add the user and item entities and their rating interactions
	for (size_t i = 0; i < userIds.size(); i++) {
		int64_t userId = userIds[i];
		map<int8_t, vector<rt::Interact> >& userInteracts =
				inputDataset.ent_type_interacts[userId];
		vector<rt::Interact>& ratings =
				userInteracts[EntityInteraction::RATE_ITEM];
		DatasetExt& curDataset = (i < split1Size ? subSet1 : subSet2);
		curDataset.add_entity(Entity::ENT_USER, userId);
		for (vector<rt::Interact>::const_iterator iter = ratings.begin();
				iter < ratings.end(); ++iter) {
			Interact interact = *iter;
			int64_t itemId = interact.ent_id;
			curDataset.add_entity(Entity::ENT_ITEM, itemId);
			/// also add the entity interaction
			curDataset.ent_type_interacts[userId][EntityInteraction::RATE_ITEM].push_back(
					interact);
			interact.ent_id = userId;
			curDataset.ent_type_interacts[itemId][EntityInteraction::RATE_ITEM].push_back(
					interact);
		}
	}
	/// now add user/item <-> feature interactions
	DatasetExt* splits[] = { &subSet1, &subSet2 };
	for (size_t i = 0; i < 2; i++) {
		_add_feature_interactions(*(splits[i]),inputDataset);
	}
}

DatasetManager::DatasetManager(size_t const& numCvFolds) :
		m_datasets(4),m_cv_datasets(numCvFolds),m_cv_folds(numCvFolds) {
}

DatasetManager::~DatasetManager() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
