/*
 * ItemObject.h
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#ifndef ITEMOBJECT_H_
#define ITEMOBJECT_H_
#include "LatentObject.h"

class ItemObject : public LatentObject{
public:
	ItemObject(unsigned int id=0, unsigned int dim=0, bool isDiagCov = true);
	virtual ~ItemObject();
};

#endif /* ITEMOBJECT_H_ */
