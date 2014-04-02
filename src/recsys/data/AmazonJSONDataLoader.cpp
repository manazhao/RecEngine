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
#include <assert.h>
#include "Entity.h"
#include "EntityInteraction.h"

namespace recsys{

AmazonJSONDataLoader::str_set_ptr AmazonJSONDataLoader::_get_item_cat_nodes(string const& catStr){
	str_set_ptr resultSetPtr(new str_set());
	/// split the cats by | and then by /
	vector<string> catPaths;
	boost::split(catPaths,catStr,boost::is_any_of("|"));
	for(vector<string>::iterator iter = catPaths.begin(); iter < catPaths.end(); ++iter){
		string& tmpCatPath = *iter;
		/// further split by / to get the category nodes
		vector<string> catNodes;
		split(catNodes,tmpCatPath,boost::is_any_of("/"));
		/// only keep two nodes closest to the leaf node
		for(size_t i = 0; i < 2 && i < catNodes.size(); i++){
			string tmpNode = catNodes[i];
//			vector<string> tmpNodeSplits;
//			split(tmpNodeSplits,tmpNode,boost::is_any_of("-"));
			/// add to the set
			resultSetPtr->insert(tmpNode);
		}
	}
	return resultSetPtr;
}

void AmazonJSONDataLoader::load_author_profile(string const& fileName){
	/// check the file existence
	fstream fs(fileName.c_str());
	assert(fs.good());
	/// open the file and read
	string line;
	cout << "start to load user profile..." << endl;
	while(std::getline(fs,line)){
		stringstream ss;
		ss << line;
		js::Object authorObj;
		js::Reader::Read(authorObj,ss);
		js::String authorId = authorObj["id"];
		/// location is yet normalized, hold up using it.
//		js::String location = itemObj["l"];
		/// create user entity
		Entity authorEntity(authorId.Value(),Entity::ENT_USER);
		Entity::entity_ptr authorEntityPtr =  authorEntity.index_if_not_exist();
		if(authorObj.Find("age") != authorObj.End()){
			js::String age = authorObj["age"];
			float ageF = lexical_cast<float>(age.Value());
			ushort ageI = (ushort)ageF / 5;
			Entity tmpEntity("ag_" + lexical_cast<string>(ageI), Entity::ENT_FEATURE);
			Entity::entity_ptr tmpEntityPtr = tmpEntity.index_if_not_exist();
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",1));
			tmpEI.set_from_entity(authorEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
		if(authorObj.Find("gender") != authorObj.End()){
			js::String gender = authorObj["gender"];
			Entity tmpEntity("gd_" + gender.Value(), Entity::ENT_FEATURE);
			Entity::entity_ptr tmpEntityPtr = tmpEntity.index_if_not_exist();
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",1));
			tmpEI.set_from_entity(authorEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
//		size_t authorMappedId = authorEntityPtr->get_mapped_id();
//		EntityInteraction::entity_interact_vec_ptr authorFeatVec = EntityInteraction::query(authorMappedId,Entity::ENT_USER);
//		if(authorFeatVec){
//			for(EntityInteraction::entity_interact_vec::iterator iter = authorFeatVec->begin(); iter < authorFeatVec->end(); ++iter){
//				cout << **iter << endl;
//			}
//		}
	}
	fs.close();
	m_author_inited = true;
}


void AmazonJSONDataLoader::load_item_profile(string const& fileName){
	/// check the file existence
	fstream fs(fileName.c_str());
	assert(fs.good());
	/// open the file and read
	string line;
	cout << "start to load item profile..." << endl;
	while(std::getline(fs,line)){
		stringstream ss;
		ss << line;
		js::Object itemObj;
		js::Reader::Read(itemObj,ss);
		/// extract the category fields and merchant fields
		js::String itemId = itemObj["id"];
		js::String itemCats = itemObj["c"];
		str_set_ptr itemCatNodes = _get_item_cat_nodes(itemCats);
		/// add item Entity
		Entity itemEntity(itemId.Value(),Entity::ENT_ITEM);
		Entity::entity_ptr itemEntityPtr = itemEntity.index_if_not_exist();
		if(itemObj.Find("m") != itemObj.End()){
			js::String itemMerchant = itemObj["m"];
			Entity tmpFeatEntity("m_" + itemMerchant.Value(),Entity::ENT_FEATURE,JSObjectWrapper().add("t","c"));
			Entity::entity_ptr tmpEntityPtr = tmpFeatEntity.index_if_not_exist();
			/// add an EntityInteraction between the item entity and the feature entity
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",1));
			tmpEI.set_from_entity(itemEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
		/// create the item - category interactions
		for(str_set::iterator iter = itemCatNodes->begin(); iter != itemCatNodes->end(); ++iter){
			string catFeature = "c_" + *iter;
			Entity tmpEntity(catFeature, Entity::ENT_FEATURE);
			Entity::entity_ptr tmpEntityPtr = tmpEntity.index_if_not_exist();
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE,JSObjectWrapper().add("v",1));
			tmpEI.set_from_entity(itemEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
	}
	fs.close();
	m_item_inited = true;
}

void AmazonJSONDataLoader::load_rating_file(string const& fileName){
	/// make sure both author profile and item profile are well initialized prior to loading the ratings
	assert(m_author_inited && m_item_inited);
	fstream fs(fileName.c_str());
	assert(fs.good());
	/// open the file and read
	string line;
	cout << "start to loading user-item ratings..." << endl;
	while(std::getline(fs,line)){
		stringstream ss;
		ss << line;
		js::Object ratingObj;
		js::Reader::Read(ratingObj,ss);
		js::String authorId = ratingObj["u"];
		js::String itemId = ratingObj["i"];
		js::String rating = ratingObj["r"];
		/// add the rating interaction entity directly
		EntityInteraction ei(EntityInteraction::RATE_ITEM,JSObjectWrapper().add("v",rating.Value()));
		ei.add_from_entity(authorId,Entity::ENT_USER);
		ei.add_to_entity(itemId,Entity::ENT_ITEM);
		ei.index_if_not_exist();
	}
	fs.close();
}

AmazonJSONDataLoader::AmazonJSONDataLoader():m_author_inited(false),m_item_inited(false){
	// TODO Auto-generated constructor stub

}

AmazonJSONDataLoader::~AmazonJSONDataLoader() {
	// TODO Auto-generated destructor stub
}

}

