/*
 * MemoryData.cpp
 *
 *  Created on: Mar 24, 2014
 *      Author: qzhao2
 */

#include "MemoryData.h"
#include <algorithm>
#include <string>
#include "../../csv.h"
#include <boost/lexical_cast.hpp>
#include "../data/json/elements.h"
#include "../data/json/reader.h"
#include "../data/json/writer.h"


using namespace boost;
using namespace std;
using namespace json;

namespace recsys {

MemoryData::MemoryData() {
	// TODO Auto-generated constructor stub

}


void MemoryData::load_item_profile(string const& profileFile){
	fstream fs;
	cout << "read from rating file:" << profileFile << endl;
	fs.open(profileFile.c_str(),fstream::in);
	if(!fs.good()){
		cerr << "failed to open rating file" << endl;
		exit(1);
	}
	string line;
	size_t lineCounter = 0;
	while(std::getline(fs,line)){
		/// decode
		Object obj;
		stringstream ss;
		ss << line;
		Reader::Read(obj,ss);
		String id = obj["id"];
		m_item_profile_map[id.Value()] = line;
		lineCounter++;
		if(lineCounter % 5000 == 0){
			cout << ".";
		}
	}
	cout << endl;
	fs.close();
}

void MemoryData::load_rating(string const& ratingFile){
	/// load the data
	fstream fs;
	cout << "read from rating file:" << ratingFile << endl;
	fs.open(ratingFile.c_str(),fstream::in);
	if(!fs.good()){
		cerr << "failed to open rating file" << endl;
		exit(1);
	}
	size_t lineCounter = 0;
	for(CSVIterator iter(fs,'\t'); iter != CSVIterator(); ++iter){
		CSVRow const& row = *iter;
		if(row.size() == 0)
			continue;
		string iid = row[1];
		unsigned short rating = lexical_cast<unsigned short>(row[2]);
		m_item_popularity_map[iid]++;
		m_item_avg_rating_map[iid] += rating;
		lineCounter++;
		if(lineCounter % 5000 == 0){
			cout << ".";
		}
	}
	cout << endl;
	fs.close();
	/// get the average rating
	for(map<string,size_t>::iterator iter = m_item_popularity_map.begin(); iter != m_item_popularity_map.end();++iter){
		m_pop_item_vec.push_back(ItemScore(iter->first,iter->second));
	}
	cout << "sort the item in descending popularity..." << endl;
	sort(m_pop_item_vec.begin(),m_pop_item_vec.end());
}

MemoryData::~MemoryData() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
