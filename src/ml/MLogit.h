/*

 * MLogit.h
 *
 * Multinomial logit regression
 *
 *  Created on: Sep 30, 2014
 *      Author: manazhao
 */

#ifndef ML_MLOGIT_H_
#define ML_MLOGIT_H_

#include <string>
using namespace std;

class MLogit {
protected:
	/// user feature file
	 string m_ufeat_file;
	/// item feature file
	 string m_ifeat_file;
	/// feature dictionary
	 string m_dict_file;
protected:
	 void _LoadFeature();
public:
	MLogit(const string& ufeat_file, const string& ifeat_file, const string& dict_file):
		m_ufeat_file(ufeat_file),m_ifeat_file(ifeat_file),m_dict_file(dict_file)
	{

	}
	virtual ~MLogit();
};

#endif /* ML_MLOGIT_H_ */
