/*
 * JSONDataLoader.h
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#ifndef JSONDATALOADER_H_
#define JSONDATALOADER_H_
#include <string>
#include <set>
#include <boost/shared_ptr.hpp>
#include "DataLoader.h"
using namespace boost;
using namespace std;

namespace recsys {

class EntityParser{
	friend class JSONDataLoader;
protected:
	virtual void _parse_helper(js::Object& jsObj, Entity& entity, vector<Entity>& entityFeatures)  = 0;
public:
	EntityParser();
	void parse(string const& line);
};

class JSONDataLoader : public DataLoader{
protected:
	typedef set<string> str_set;
	typedef shared_ptr<str_set> str_set_ptr;
protected:
	void _load_entity_profile(string const& file, EntityParser& parser);
public:
	void load_user_profile(string const& file, EntityParser& parser);
	void load_item_profile(string const& file, EntityParser& parser);
	void load_user_item_rating(string const& file);
	void prepare_datasets();
public:
	JSONDataLoader();
	virtual ~JSONDataLoader();
};

} /* namespace recsys */

#endif /* JSONDATALOADER_H_ */
