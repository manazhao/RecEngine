/*
 * UserObject.h
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#ifndef USEROBJECT_H_
#define USEROBJECT_H_
#include "LatentObject.h"

class UserObject : public LatentObject{
public:
	UserObject(unsigned int id=0, unsigned int dim=0, bool isDiagCov = true);
	virtual ~UserObject();
};

#endif /* USEROBJECT_H_ */
