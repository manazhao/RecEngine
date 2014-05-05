/*
 * AmazonParser.h
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#ifndef MOVIELENSPARSER_H_
#define MOVIELENSPARSER_H_

#include "JSONDataLoader.h"
#include <boost/algorithm/string.hpp>
namespace recsys {
namespace movieLens {
class UserEntityParser: public recsys::EntityParser {
protected:
	virtual void _parse_helper(js::Object& jsObj, Entity& entity,
			vector<Entity>& entityFeatures) {
		js::String userId = jsObj["id"];
		entity = Entity(userId.Value(), Entity::ENT_USER);
		/// erase the id field
		jsObj.Erase(jsObj.Find("id"));
		js::Object::const_iterator beginIter(jsObj.Begin()), endIter(jsObj.End());
		for(; beginIter != endIter; ++beginIter){
			string featureName = beginIter->name;
			/// generate the feature
			js::String featureValue = beginIter->element;
			string compositeFeature = featureName + "_" + featureValue.Value();
			Entity tmpEntity(compositeFeature,Entity::ENT_FEATURE);
			entityFeatures.push_back(tmpEntity);
		}
	}
};

class ItemEntityParser: public recsys::EntityParser {
protected:
	virtual void _parse_helper(js::Object& jsObj, Entity& entity,
			vector<Entity>& entityFeatures) {
		/// extract the category fields and merchant fields
		js::String itemId = jsObj["id"];
		/// add item Entity
		js::Object objDesc;
		objDesc["name"] = jsObj["name"];
		entity = Entity(itemId.Value(), Entity::ENT_ITEM,objDesc);
		js::Object::const_iterator beginIter(jsObj.Begin()), endIter(jsObj.End());
		jsObj.Erase(jsObj.Find("name"));
		jsObj.Erase(jsObj.Find("id"));
		for(;beginIter != endIter; ++beginIter){
			string featureName = beginIter->name;
			if(featureName == "genres"){
				js::Array genres = beginIter->element;
				/// iterate through the genres
				for(js::Array::const_iterator iter = genres.Begin(); iter != genres.End(); ++iter){
					js::String tmpGenre = *iter;
					string featureName = "genre_" + tmpGenre.Value();
					Entity tmpEntity(featureName, Entity::ENT_FEATURE);
					entityFeatures.push_back(tmpEntity);
				}
			}
		}
	}
};

}
}

#endif /* AMAZONPARSER_H_ */
