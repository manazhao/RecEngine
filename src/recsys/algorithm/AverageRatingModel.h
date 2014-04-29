/*
 * AverageRatingModel.h
 *
 *  Created on: Apr 29, 2014
 *      Author: qzhao2
 */

#ifndef AVERAGERATINGMODEL_H_
#define AVERAGERATINGMODEL_H_

#include <recsys/algorithm/RecModel.h>
#include "recsys/thrift/cpp/DataHost.h"

namespace rt = recsys::thrift;

namespace recsys {

class AverageRatingModel: public recsys::RecModel {
protected:
	float m_avg_rating;
protected:
	virtual void _init_training();
	virtual TrainIterLog _train_update();
	virtual float _pred_error(int64_t const& entityId, map<int8_t, vector<Interact> >& entityInteractMap);
	virtual void _add_new_entity(int64_t const& entityId, int8_t const& entityType);
public:
	AverageRatingModel();
	virtual vector<rt::Recommendation> recommend(int64_t const& userId, map<int8_t, vector<rt::Interact> >& userInteracts);
	virtual string model_summary();
	virtual ~AverageRatingModel();
};

} /* namespace recsys */

#endif /* AVERAGERATINGMODEL_H_ */
