/*
 * MemoryData.h
 *
 *  Created on: Mar 24, 2014
 *      Author: qzhao2
 */

#ifndef MEMORYDATA_H_
#define MEMORYDATA_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

namespace recsys {

typedef map<string,size_t> name_id_map;
typedef map<size_t,string> id_name_map;
class MemoryData {
public:
	struct ItemScore{
		string m_product_id;
		float m_score;
		ItemScore(string const& productId = "", float const& score = 0.0):
			m_product_id(productId),m_score(score){

		}
		bool operator<(ItemScore const& rhs) const{
			return m_score > rhs.m_score;
		}
	};
protected:
	map<string,size_t> m_item_popularity_map;
	map<string,float> m_item_avg_rating_map;
	vector<ItemScore> m_pop_item_vec;
	map<string,string> m_item_profile_map;
public:
	MemoryData();
	void load_rating(string const& ratingFile);
	vector<string> const& top_popular_items(size_t const& n = 10){
		static vector<string> topItems;
		static bool isGenerated = false;
		if(!isGenerated){
			for(size_t i = 0; i < n ; i++){
				string itemId = m_pop_item_vec[i].m_product_id;
				if(m_item_profile_map.find(itemId) != m_item_profile_map.end()){
					topItems.push_back(m_item_profile_map[itemId]);
					cout << "item id:" << itemId << endl;
				}
			}
			isGenerated = true;
		}
		return topItems;
	}
	void load_item_profile(string const& profileFile);
	virtual ~MemoryData();
};

} /* namespace recsys */

#endif /* MEMORYDATA_H_ */
