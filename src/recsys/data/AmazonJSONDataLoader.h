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
#include "DatasetManager.h"
using namespace boost;
using namespace std;

//#define __DEBUG_LOADING__

namespace recsys {

class AmazonJSONDataLoader {
protected:
	typedef set<string> str_set;
	typedef shared_ptr<str_set> str_set_ptr;
protected:
	string m_author_file;
	string m_item_file;
	string m_rating_file;
	bool m_author_inited;
	bool m_item_inited;
	DatasetManager m_dataset_manager;
protected:
	str_set_ptr _get_item_cat_nodes(string const& catStr);
public:
	AmazonJSONDataLoader(string const& authorFile, string const& itemFile, string const& ratingFile);
	void prepare_datasets();
	void load_author_profile();
	void load_item_profile();
	void load_rating_file();
	rt::Dataset& get_data_set(rt::DSType::type dsType){
		return m_dataset_manager.get_dataset(dsType);
	}
	virtual ~AmazonJSONDataLoader();
};

void test_amazon_data_loader();
}

#endif /* JSONDATALOADER_H_ */
