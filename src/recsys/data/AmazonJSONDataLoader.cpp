/*
 * JSONDataLoader.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: qzhao2
 */

#include <boost/algorithm/string.hpp>
#include "EntityInteraction.h"
#include <recsys/data/AmazonJSONDataLoader.h>
#include <fstream>
#include <set>
#include <boost/timer.hpp>
#include <assert.h>
#include "Entity.h"
#include "EntityInteraction.h"

namespace recsys {

AmazonJSONDataLoader::str_set_ptr AmazonJSONDataLoader::_get_item_cat_nodes(
		string const& catStr) {
	str_set_ptr resultSetPtr(new str_set());
	/// split the cats by | and then by /
	vector<string> catPaths;
	boost::split(catPaths, catStr, boost::is_any_of("|"));
	for (vector<string>::iterator iter = catPaths.begin();
			iter < catPaths.end(); ++iter) {
		string& tmpCatPath = *iter;
		/// further split by / to get the category nodes
		vector<string> catNodes;
		split(catNodes, tmpCatPath, boost::is_any_of("/"));
		/// only keep two nodes closest to the leaf node
		for (size_t i = 0; i < 2 && i < catNodes.size(); i++) {
			string tmpNode = catNodes[i];
//			vector<string> tmpNodeSplits;
//			split(tmpNodeSplits,tmpNode,boost::is_any_of("-"));
			/// add to the set
			resultSetPtr->insert(tmpNode);
		}
	}
	return resultSetPtr;
}

void AmazonJSONDataLoader::prepare_datasets() {
	// Your implementation goes here
	printf("############### prepare datasets ###############\n");
	boost::timer t;
	DatasetExt& allDataset = m_all_datasets[rt::DSType::DS_ALL];
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

	std::set<int64_t> userIdSet = allDataset.type_ent_ids[Entity::ENT_USER];
	vector<int64_t> userIds;
	userIds.assign(userIdSet.begin(), userIdSet.end());
	std::random_shuffle(userIds.begin(), userIds.end());
	const float csRatio = 0.2;
	const float trainRatio = 0.8;
	/// shuffle the array
	size_t csUserEndId = (size_t) (userIds.size() * csRatio);
	/// reserve space for training dataset
	DatasetExt& trainDataset = m_all_datasets[rt::DSType::DS_TRAIN];
	DatasetExt& testDataset = m_all_datasets[rt::DSType::DS_TEST];
	DatasetExt& csDataset = m_all_datasets[rt::DSType::DS_CS];
	trainDataset = DatasetExt(allDataset.ent_type_interacts.size());
	testDataset = DatasetExt(allDataset.ent_type_interacts.size());
	csDataset = DatasetExt(allDataset.ent_type_interacts.size());
	for (size_t userIdx = 0; userIdx < userIds.size(); userIdx++) {
		/// user -> item and user -> feature interactions
		int64_t userId = userIds[userIdx];
		map<int8_t, vector<rt::Interact> >& userInteracts =
				allDataset.ent_type_interacts[userId];
		if (userIdx < csUserEndId) {
			csDataset.add_entity(Entity::ENT_USER, userId);
		} else {
			trainDataset.add_entity(Entity::ENT_USER, userId);
			testDataset.add_entity(Entity::ENT_USER, userId);
		}
		for (map<int8_t, vector<rt::Interact> >::iterator iter =
				userInteracts.begin(); iter != userInteracts.end(); ++iter) {
			int8_t type = iter->first;
			vector<rt::Interact>& tmpInteracts = iter->second;
			vector<size_t> tmpIdxVec(tmpInteracts.size());
			for (size_t j = 0; j < tmpInteracts.size(); j++) {
				tmpIdxVec[j] = j;
			}
			if (type == EntityInteraction::RATE_ITEM
					&& userIdx >= csUserEndId) {
				random_shuffle(tmpIdxVec.begin(), tmpIdxVec.end());
			}
			size_t endTrainIdx = 0;
			if (type == EntityInteraction::RATE_ITEM) {
				endTrainIdx = (size_t) (tmpInteracts.size() * trainRatio);
			}
			for (size_t intIdx = 0; intIdx < tmpIdxVec.size(); intIdx++) {
				// add the destination entity id
				Interact& tmpInteract = tmpInteracts[tmpIdxVec[intIdx]];
				Entity::ENTITY_TYPE endEntType = Entity::ENT_DEFAULT;
				switch (type) {
				case EntityInteraction::RATE_ITEM:
					endEntType = Entity::ENT_ITEM;
					break;
				case EntityInteraction::ADD_FEATURE:
					endEntType = Entity::ENT_FEATURE;
					break;
				}
				if (userIdx < csUserEndId) {
					csDataset.add_entity(endEntType, tmpInteract.ent_id);
				} else {
					/// for coldstart dataset, add directly
					/// add user id and feature id to both training and testing dataset
					trainDataset.add_entity(Entity::ENT_USER, userId);
					testDataset.add_entity(Entity::ENT_USER, userId);
					if (type == EntityInteraction::ADD_FEATURE) {
						testDataset.add_entity(endEntType,
								tmpInteract.ent_id);
						trainDataset.add_entity(endEntType,
								tmpInteract.ent_id);
					}
					if (type == EntityInteraction::RATE_ITEM) {
						if (intIdx < endTrainIdx)
							trainDataset.add_entity(endEntType,
									tmpInteract.ent_id);
						else
							testDataset.add_entity(endEntType,
									tmpInteract.ent_id);
					}
				}
			}
		}
	}
	csDataset.filter_entity_interactions(allDataset.ent_type_interacts);
	trainDataset.filter_entity_interactions(allDataset.ent_type_interacts);
	testDataset.filter_entity_interactions(allDataset.ent_type_interacts);

	cout << ">>>>>>>>>>>> time elapsed:" << t.elapsed() << " >>>>>>>>>>>>" <<  endl;
}


void AmazonJSONDataLoader::_get_entity_interacts(
		std::map<int8_t, std::vector<Interact> > & _return,
		const int64_t entId) {
	// Your implementation goes here
//		printf("get_entity_interacts\n");
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
void AmazonJSONDataLoader::load_author_profile() {
/// check the file existence
	fstream fs(m_author_file.c_str());
	assert(fs.good());
/// open the file and read
	string line;
	cout << "start to load user profile..." << endl;
	timer t;
	while (std::getline(fs, line)) {
		stringstream ss;
		ss << line;
		js::Object authorObj;
		js::Reader::Read(authorObj, ss);
		js::String authorId = authorObj["id"];
		/// location is yet normalized, hold up using it.
//		js::String location = itemObj["l"];
		/// create user entity
		Entity authorEntity(authorId.Value(), Entity::ENT_USER);
		Entity::entity_ptr authorEntityPtr = authorEntity.index_if_not_exist();
		if (authorObj.Find("age") != authorObj.End()) {
			js::String age = authorObj["age"];
			float ageF = lexical_cast<float>(age.Value());
			int8_t ageI = (int8_t) ageF / 5;
			Entity tmpEntity("ag_" + lexical_cast<string>(ageI),
					Entity::ENT_FEATURE);
			Entity::entity_ptr tmpEntityPtr = tmpEntity.index_if_not_exist();
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE,
					JSObjectWrapper().add("v", 1));
			tmpEI.set_from_entity(authorEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
		if (authorObj.Find("gender") != authorObj.End()) {
			js::String gender = authorObj["gender"];
			Entity tmpEntity("gd_" + gender.Value(), Entity::ENT_FEATURE);
			Entity::entity_ptr tmpEntityPtr = tmpEntity.index_if_not_exist();
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE,
					JSObjectWrapper().add("v", 1));
			tmpEI.set_from_entity(authorEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
	}
	fs.close();
	cout << "time elapsed: " << t.elapsed() << endl;
	m_author_inited = true;
}

void AmazonJSONDataLoader::load_item_profile() {
/// check the file existence
	fstream fs(m_item_file.c_str());
	assert(fs.good());
/// open the file and read
	string line;
	cout << "start to load item profile..." << endl;

	timer t;
	while (std::getline(fs, line)) {
		stringstream ss;
		ss << line;
		js::Object itemObj;
		js::Reader::Read(itemObj, ss);
		/// extract the category fields and merchant fields
		js::String itemId = itemObj["id"];
		js::String itemCats = itemObj["c"];
		str_set_ptr itemCatNodes = _get_item_cat_nodes(itemCats);
		/// add item Entity
		Entity itemEntity(itemId.Value(), Entity::ENT_ITEM);
		Entity::entity_ptr itemEntityPtr = itemEntity.index_if_not_exist();
		if (itemObj.Find("m") != itemObj.End()) {
			js::String itemMerchant = itemObj["m"];
			Entity tmpFeatEntity("m_" + itemMerchant.Value(),
					Entity::ENT_FEATURE, JSObjectWrapper().add("t", "c"));
			Entity::entity_ptr tmpEntityPtr =
					tmpFeatEntity.index_if_not_exist();
			/// add an EntityInteraction between the item entity and the feature entity
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE);
			tmpEI.set_from_entity(itemEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
		/// create the item - category interactions
		for (str_set::iterator iter = itemCatNodes->begin();
				iter != itemCatNodes->end(); ++iter) {
			string catFeature = "c_" + *iter;
			Entity tmpEntity(catFeature, Entity::ENT_FEATURE);
			Entity::entity_ptr tmpEntityPtr = tmpEntity.index_if_not_exist();
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE);
			tmpEI.set_from_entity(itemEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
	}
	fs.close();
	cout << "time elapsed:" << t.elapsed() << endl;
	m_item_inited = true;
}

void AmazonJSONDataLoader::load_rating_file() {
/// make sure both author profile and item profile are well initialized prior to loading the ratings
#ifndef __DEBUG_LOADING__
	assert(m_author_inited && m_item_inited);
#endif
	fstream fs(m_rating_file.c_str());
	assert(fs.good());
/// open the file and read
	string line;
	cout << "start to loading user-item ratings..." << endl;
	timer t;
#ifdef __DEBUG_LOADING__
	size_t lineCnt = 0;
#endif
	while (std::getline(fs, line)) {
		stringstream ss;
		ss << line;
		js::Object ratingObj;
		js::Reader::Read(ratingObj, ss);
		js::String authorId = ratingObj["u"];
		js::String itemId = ratingObj["i"];
		js::String rating = ratingObj["r"];
		/// add the rating interaction entity directly
		EntityInteraction ei(EntityInteraction::RATE_ITEM,
				lexical_cast<float>(rating.Value()), js::Object());
		ei.add_from_entity(authorId, Entity::ENT_USER);
		ei.add_to_entity(itemId, Entity::ENT_ITEM);
		ei.index_if_not_exist();
//#ifdef __DEBUG_LOADING__
//		if(lineCnt++ > 100000)
//		break;
//#endif
	}
	fs.close();
	cout << "time elapsed:" << t.elapsed() << endl;
}

AmazonJSONDataLoader::AmazonJSONDataLoader(string const& authorFile,
		string const& itemFile, string const& ratingFile) :
		m_author_file(authorFile), m_item_file(itemFile), m_rating_file(
				ratingFile), m_author_inited(false), m_item_inited(false), m_all_datasets(
				4) {
// TODO Auto-generated constructor stub

}

AmazonJSONDataLoader::~AmazonJSONDataLoader() {
// TODO Auto-generated destructor stub
}

}

