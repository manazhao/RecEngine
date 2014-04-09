/*
 * AmazonJSONDataLoader_test.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: qzhao2
 */
#include "AmazonJSONDataLoader.h"

namespace recsys {
void test_amazon_data_loader() {
	string authorFile =
			"/home/qzhao2/data/amazon-yms/ratings/processed/author_profile.json";
	string itemFile =
			"/home/qzhao2/data/amazon-yms/ratings/processed/item_profile.json";
	string ratingFile =
			"/home/qzhao2/data/amazon-yms/ratings/processed/book_rating_filter.json";
	AmazonJSONDataLoader amazonDataLoader(authorFile, itemFile, ratingFile);
	amazonDataLoader.load_author_profile();
	amazonDataLoader.load_item_profile();
	amazonDataLoader.load_rating_file();
}
}

