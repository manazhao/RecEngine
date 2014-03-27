/*
 * Rating.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#include "Feedback.h"
#include <sstream>

Feedback::Feedback(unsigned int fromId, unsigned int toId, float val)
:m_fromId(fromId)
,m_toId(toId)
,m_val(val)
{
	// TODO Auto-generated constructor stub
	stringstream ss;
	ss << fromId << "_" << toId;
	m_id = ss.str();
}

Feedback::~Feedback() {
	// TODO Auto-generated destructor stub
}

