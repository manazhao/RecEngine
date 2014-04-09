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
	timer t;
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
			int8_t ageI = (int8_t)ageF / 5;
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
	}
	fs.close();
	cout << "time elapsed: " << t.elapsed() << endl;
	m_author_inited = true;
}


void AmazonJSONDataLoader::load_item_profile(string const& fileName){
	/// check the file existence
	fstream fs(fileName.c_str());
	assert(fs.good());
	/// open the file and read
	string line;
	cout << "start to load item profile..." << endl;

	timer t;
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
			EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE);
			tmpEI.set_from_entity(itemEntityPtr);
			tmpEI.set_to_entity(tmpEntityPtr);
			tmpEI.index_if_not_exist();
		}
		/// create the item - category interactions
		for(str_set::iterator iter = itemCatNodes->begin(); iter != itemCatNodes->end(); ++iter){
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

void AmazonJSONDataLoader::load_rating_file(string const& fileName){
	/// make sure both author profile and item profile are well initialized prior to loading the ratings
#ifndef __DEBUG_LOADING__
	assert(m_author_inited && m_item_inited);
#endif
	fstream fs(fileName.c_str());
	assert(fs.good());
	/// open the file and read
	string line;
	cout << "start to loading user-item ratings..." << endl;
	timer t;
#ifdef __DEBUG_LOADING__
	size_t lineCnt = 0;
#endif
	while(std::getline(fs,line)){
		stringstream ss;
		ss << line;
		js::Object ratingObj;
		js::Reader::Read(ratingObj,ss);
		js::String authorId = ratingObj["u"];
		js::String itemId = ratingObj["i"];
		js::String rating = ratingObj["r"];
		/// add the rating interaction entity directly
		EntityInteraction ei(EntityInteraction::RATE_ITEM,lexical_cast<float>(rating.Value()),js::Object());
		ei.add_from_entity(authorId,Entity::ENT_USER);
		ei.add_to_entity(itemId,Entity::ENT_ITEM);
		ei.index_if_not_exist();
#ifdef __DEBUG_LOADING__
		if(lineCnt++ > 10000)
			break;
#endif
	}
	fs.close();
	cout << "time elapsed:" << t.elapsed() << endl;
}

AmazonJSONDataLoader::AmazonJSONDataLoader():m_author_inited(false),m_item_inited(false){
	// TODO Auto-generated constructor stub

}

AmazonJSONDataLoader::~AmazonJSONDataLoader() {
	// TODO Auto-generated destructor stub
}

}

