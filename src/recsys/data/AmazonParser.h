/*
 * AmazonParser.h
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#ifndef AMAZONPARSER_H_
#define AMAZONPARSER_H_

#include "JSONDataLoader.h"
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
//#define __ONE_DIM_GENDER__

namespace recsys {
namespace amazon {
class UserEntityParser: public recsys::EntityParser {
protected:
	virtual void _parse_helper(js::Object& jsObj, Entity& entity,
			vector<Entity>& entityFeatures,vector<float>& featValVec) {
		js::String userId = jsObj["id"];
		entity = Entity(userId.Value(), Entity::ENT_USER);
		/// location is yet normalized, hold up using it.
//		js::String location = jsObj["l"];
		/// create user entity

		/// age feature will be dropped for model diagnosis purpose
		if (jsObj.Find("age") != jsObj.End()) {
			js::String age = jsObj["age"];
			float ageF = lexical_cast<float>(age.Value());
			int ageI = (int) (ageF/5);
			string featureName = "ag_" + lexical_cast<string,int>(ageI);
			Entity tmpEntity(featureName,
					Entity::ENT_FEATURE);
			entityFeatures.push_back(tmpEntity);
			featValVec.push_back(1);
		}

		if (jsObj.Find("gender") != jsObj.End()) {
			js::String gender = jsObj["gender"];
			string genderStr = gender.Value();
			boost::algorithm::to_lower(genderStr);
			/// use one feature for gender and female and male take +1 and -1 values
#ifdef __ONE_DIM_GENDER__
			string featureKey = "gender_";
			float featVal = (genderStr == "female" ? 1 : -1);
#else
			string featureKey = "gender_" + genderStr;
			float featVal = 1;
#endif
			Entity tmpEntity(featureKey, Entity::ENT_FEATURE);
			featValVec.push_back(featVal);
			entityFeatures.push_back(tmpEntity);
		}
	}
};

class ItemEntityParser: public recsys::EntityParser {
protected:
	typedef set<string> str_set;
	typedef boost::shared_ptr<str_set> str_set_ptr;
protected:
	str_set_ptr _get_item_cat_nodes(string const& catStr) {
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

	virtual void _parse_helper(js::Object& jsObj, Entity& entity,
			vector<Entity>& entityFeatures,vector<float>& featValVec) {
		/// extract the category fields and merchant fields
		js::String itemId = jsObj["id"];
		js::String itemCats = jsObj["c"];
		str_set_ptr itemCatNodes = _get_item_cat_nodes(itemCats);
		/// add item Entity
		entity = Entity(itemId.Value(), Entity::ENT_ITEM);
		if (jsObj.Find("m") != jsObj.End()) {
			js::String itemMerchant = jsObj["m"];
			Entity tmpFeatEntity("m_" + itemMerchant.Value(),
					Entity::ENT_FEATURE, JSObjectWrapper().add("t", "c"));
			entityFeatures.push_back(tmpFeatEntity);
			featValVec.push_back(1);
		}
		/// create the item - category interactions
		for (str_set::iterator iter = itemCatNodes->begin();
				iter != itemCatNodes->end(); ++iter) {
			string catFeature = "c_" + *iter;
			Entity tmpFeatEntity(catFeature, Entity::ENT_FEATURE);
			entityFeatures.push_back(tmpFeatEntity);
			featValVec.push_back(1);
		}
	}
};

}
}

#endif /* AMAZONPARSER_H_ */
