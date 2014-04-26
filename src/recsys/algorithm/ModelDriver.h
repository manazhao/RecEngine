/*
 * ModelDriver.h
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#ifndef MODELDRIVER_H_
#define MODELDRIVER_H_
#include <recsys/data/EntityInteraction.h>
#include <recsys/data/AppConfig.h>
#include <recsys/data/ThriftDataLoader.h>
#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace recsys {

class ModelDriver {
public:
	ModelDriver(int argc, char** argv);
	virtual ~ModelDriver();
};

}

#endif /* MODELDRIVER_H_ */
