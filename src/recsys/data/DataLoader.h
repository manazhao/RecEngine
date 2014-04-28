/*
 * DataLoader.h
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#ifndef DATALOADER_H_
#define DATALOADER_H_
#include "DatasetManager.h"
namespace recsys {

class DataLoader {
protected:
	shared_ptr<DatasetManager> m_dataset_manager;
public:
	DataLoader();
	DatasetExt& dataset(rt::DSType::type dsType){
		return m_dataset_manager->dataset(dsType);
	}
	shared_ptr<DatasetManager> get_dataset_manager(){
		return m_dataset_manager;
	}
	virtual ~DataLoader();
};



}

#endif /* DATALOADER_H_ */
