/*
 * DataLoader.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#include "DataLoader.h"

namespace recsys {

DataLoader::DataLoader()
:m_dataset_manager(shared_ptr<DatasetManager>(new DatasetManager())){
	// TODO Auto-generated constructor stub

}

DataLoader::~DataLoader() {
	// TODO Auto-generated destructor stub
}

}
