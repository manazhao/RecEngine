/*
 * JSONDataLoader.h
 *
 *  Created on: Apr 1, 2014
 *      Author: qzhao2
 */

#ifndef JSONDATALOADER_H_
#define JSONDATALOADER_H_
#include <string>
#include <set>
#include <boost/shared_ptr.hpp>
using namespace boost;
using namespace std;

//#define __DEBUG_LOADING__

namespace recsys {

class AmazonJSONDataLoader {
protected:
	typedef set<string> str_set;
	typedef shared_ptr<str_set> str_set_ptr;
protected:
	bool m_author_inited;
	bool m_item_inited;
protected:
	str_set_ptr _get_item_cat_nodes(string const& catStr);
public:
	AmazonJSONDataLoader();
	void load_author_profile(string const& fileName);
	void load_item_profile(string const& fileName);
	void load_rating_file(string const& file);
	virtual ~AmazonJSONDataLoader();
};

void test_amazon_data_loader();
}

#endif /* JSONDATALOADER_H_ */
