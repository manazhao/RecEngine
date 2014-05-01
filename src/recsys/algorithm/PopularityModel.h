/*
 * PopularityModel.h
 *
 *  Created on: Apr 30, 2014
 *      Author: qzhao2
 */

#ifndef POPULARITYMODEL_H_
#define POPULARITYMODEL_H_

#include <recsys/algorithm/RecModel.h>
#include "recsys/thrift/cpp/DataHost.h"
#include <boost/serialization/base_object.hpp>

using namespace recsys::thrift;

namespace recsys {

class PopularityModel: public recsys::RecModel {
protected:
	vector<Recommendation> m_pop_sort_items;
protected:
protected:
	virtual void _init_training();
	virtual TrainIterLog _train_update();
	virtual float _pred_error(int64_t const& entityId, map<int8_t, vector<Interact> >& entityInteractMap);
	virtual void _add_new_entity(int64_t const& entityId, int8_t const& entityType);
public:
	PopularityModel();
	virtual vector<rt::Recommendation> recommend(int64_t const& userId, map<int8_t, vector<rt::Interact> >& userInteracts);
	virtual string model_summary();
	virtual ~PopularityModel();
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & boost::serialization::base_object<RecModel>(*this);
		ar & m_pop_sort_items;
	}
};

} /* namespace recsys */

#endif /* POPULARITYMODEL_H_ */
