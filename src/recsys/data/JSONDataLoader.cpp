/*
 * JSONDataLoader.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#include <recsys/data/JSONDataLoader.h>
#include <boost/timer.hpp>
#include <fstream>

namespace recsys {

EntityParser::EntityParser(){
}

void EntityParser::parse(string const& line) {
	stringstream ss;
	ss << line;
	js::Object entityObj;
	/// construct json object from line
	js::Reader::Read(entityObj, ss);
	/// parse the json object to get entity and its features
	Entity entity;
	vector<Entity> features;
	vector<float> featVals;
	_parse_helper(entityObj, entity, features,featVals);
	/// index the entity and the features
	Entity::entity_ptr entityPtr = entity.index_if_not_exist();
	/// index the features and link entity and feature
	for(size_t i = 0; i < features.size(); i++){
		Entity& feature = features[i];
		Entity::entity_ptr featurePtr = feature.index_if_not_exist();
		float featVal = featVals[i];
		EntityInteraction tmpEI(EntityInteraction::ADD_FEATURE, featVal);
//		cout << "entity interaction value:" << tmpEI.m_val << endl;
		tmpEI.set_from_entity(entityPtr);
		tmpEI.set_to_entity(featurePtr);
		tmpEI.index_if_not_exist();
	}
}

JSONDataLoader::JSONDataLoader(){

}

void JSONDataLoader::_load_entity_profile(string const& file,
		EntityParser& parser) {
	/// check the file existence
	fstream fs(file.c_str());
	assert(fs.good());
	string line;
	while (std::getline(fs, line)) {
		parser.parse(line);
	}
}

void JSONDataLoader::load_user_profile(string const& file, EntityParser& parser){
	boost::timer t;
	cout << "start to load user profile file" << endl;
	_load_entity_profile(file,parser);
	cout << "time elapsed: " << t.elapsed() << " seconds" << endl;
}

void JSONDataLoader::load_item_profile(string const& file, EntityParser& parser){
	boost::timer t;
	cout << "start to load item profile file" << endl;
	_load_entity_profile(file,parser);
	cout << "time elapsed: " << t.elapsed() << " seconds" << endl;
}

void JSONDataLoader::load_user_item_rating(string const& file) {
	boost::timer t;
	cout << "start to load rating file" << endl;
	fstream fs(file.c_str());
	assert(fs.good());
	string line;
	while (std::getline(fs, line)) {
		stringstream ss;
		ss << line;
		js::Object ratingObj;
		js::Reader::Read(ratingObj, ss);
		js::String userId = ratingObj["u"];
		js::String itemId = ratingObj["i"];
		js::String rating = ratingObj["r"];
		/// add the rating interaction entity directly
		EntityInteraction ei(EntityInteraction::RATE_ITEM,
				lexical_cast<float>(rating.Value()), js::Object());
		ei.add_from_entity(userId, Entity::ENT_USER);
		ei.add_to_entity(itemId, Entity::ENT_ITEM);
		ei.index_if_not_exist();
	}
	fs.close();
	cout << "time elapsed: " << t.elapsed() << " seconds" << endl;

}

void JSONDataLoader::prepare_datasets() {
	// Your implementation goes here
	m_dataset_manager->init_complete_dataset();
	m_dataset_manager->generate_datasets();
//	m_dataset_manager->generate_cv_datasets();
}

JSONDataLoader::~JSONDataLoader() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
