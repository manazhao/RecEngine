/*
 * Rating.h
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#ifndef RATING_H_
#define RATING_H_
#include <string>
using namespace std;

class Feedback {
	friend class MFImplicitModel;
	friend class VLImplicitModel;
protected:
	string m_id;
	unsigned int m_fromId;
	unsigned int m_toId;
	float m_val;
public:
	Feedback(unsigned int fromId, unsigned int toId, float val);
	operator string() const{
		return m_id;
	}
	virtual ~Feedback();
};


#endif /* RATING_H_ */
