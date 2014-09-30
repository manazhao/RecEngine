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
namespace rt = recsys::thrift;

namespace recsys {

class DatasetManager {
public:
	struct TrainTestPair{
		DatasetExt m_train;
		DatasetExt m_test;
		TrainTestPair(size_t numOfEntity  = 0)
		:m_train(numOfEntity),m_test(numOfEntity){
		}
	};
protected:
	vector<DatasetExt> m_datasets;
	vector<TrainTestPair> m_cv_datasets;
	size_t m_cv_folds;
protected:
	void _get_entity_interacts(
			std::map<int8_t, std::vector<Interact> > & _return,
			const int64_t entId);
	void _split_by_rating(float splitRatio, DatasetExt & inputDataset, DatasetExt& subset1, DatasetExt& subset2);
	void _split_by_user(float splitRatio, DatasetExt & inputDataset, DatasetExt& subSet1, DatasetExt& subSet2);
	void _add_feature_interactions(DatasetExt& dataset, DatasetExt& inputDataset);
public:
	DatasetManager(size_t const& numCvFolds = 5);
	DatasetExt& dataset(rt::DSType::type type){
		return m_datasets[type];
	}
	TrainTestPair& cv_dataset(size_t foldIdx = 0){
		return m_cv_datasets[foldIdx];
	}
	inline size_t get_cv_folds(){
		return m_cv_folds;
	}
	void init_complete_dataset();
	void generate_datasets();
	void generate_cv_datasets();
	virtual ~DatasetManager();
};

} /* namespace recsys */

#endif /* DATASETMANAGER_H_ */
