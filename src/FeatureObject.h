/*
 * FeatureObject.h
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#ifndef FEATUREOBJECT_H_
#define FEATUREOBJECT_H_
#include "LatentObject.h"

class FeatureObject : public LatentObject{
public:
	FeatureObject(unsigned int id=0, unsigned int dim=10, bool isDiagCov = true);
	virtual ~FeatureObject();
};

#endif /* FEATUREOBJECT_H_ */
