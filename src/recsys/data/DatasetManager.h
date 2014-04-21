/*
 * DatasetManager.h
 *
 *  Created on: Apr 21, 2014
 *      Author: qzhao2
 */

#ifndef DATASETMANAGER_H_
#define DATASETMANAGER_H_

#include "DatasetExt.h"
#include "Entity.h"
#include "EntityInteraction.h"

namespace recsys {

class DatasetManager {
protected:
	vector<DatasetExt> m_datasets;
protected:
	void _get_entity_interacts(
			std::map<int8_t, std::vector<Interact> > & _return,
			const int64_t entId);
	void _split_by_rating(float splitRatio, DatasetExt & inputDataset, DatasetExt& subset1, DatasetExt& subset2);
	void _split_by_user(float splitRatio, DatasetExt & inputDataset, DatasetExt& subSet1, DatasetExt& subSet2);
public:
	DatasetManager();
	void generate_datasets();
	DatasetExt& get_dataset(rt::DSType::type dsType){
		return m_datasets[dsType];
	}
	virtual ~DatasetManager();
};

} /* namespace recsys */

#endif /* DATASETMANAGER_H_ */
