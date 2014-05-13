/*
 * Model.h
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#ifndef MODEL_H_
#define MODEL_H_
#include "recsys/data/DatasetExt.h"
#include "recsys/thrift/cpp/data_types.h"
#include <sstream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include "recsys/data/DatasetManager.h"

namespace rt = recsys::thrift;

using namespace std;

namespace boost {
namespace serialization {
template<class Archive>
void serialize(Archive & ar, Recommendation& rec, unsigned version) {
	ar & rec.id & rec.score & rec.type;
}
}
}

namespace recsys {

class RecModel {
	friend class ModelDriver;
public:
	struct ModelParam {
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & m_lat_dim & m_max_iter & m_diag_cov & m_use_feature;
		}
	public:
		/// the dimensionality of the latent vector
		size_t m_lat_dim;
		/// maximum number of iterations
		size_t m_max_iter;
		/// whether use diagonal multivariate Gaussian
		bool m_diag_cov;
		// whether use feature
		bool m_use_feature;
		ModelParam(size_t const& latDim = 10, size_t const& maxIter = 10,
				bool diagCov = true, bool useFeature = false);
		ModelParam(int argc, char** argv);
		operator string() const {
			stringstream ss;
			ss << "d_" << m_lat_dim << "-f" << "_" << m_use_feature;
			return ss.str();
		}
	};

	struct TrainIterLog {
		/// iteration index
		size_t m_iter;
		/// training rmse
		float m_train_rmse;
		/// testing rmse
		float m_test_rmse;
		/// coldstart dataset rmse
		float m_cs_rmse;
		/// iteration time
		float m_iter_time;
		TrainIterLog() :
				m_iter(0), m_train_rmse(0), m_test_rmse(0), m_cs_rmse(0), m_iter_time(
						0) {

		}
	};

protected:
	ModelParam m_model_param;
	DatasetExt m_active_dataset;
	std::shared_ptr<DatasetManager> m_dataset_manager;
	bool m_model_selection;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		/// serialize every data member except for the dataset
		ar & m_model_param & m_active_dataset;
	}

protected:
	virtual void _init_training() = 0;
	virtual TrainIterLog _train_update() = 0;
	virtual float _pred_error(int64_t const& userId, DatasetExt& dataset) = 0;
//	virtual float _pred_error(int64_t const& entityId,
//			map<int8_t, vector<Interact> >& entityInteractMap) = 0;
	virtual void _add_new_entity(int64_t const& entityId,
			int8_t const& entityType) = 0;
	float _dataset_rmse(DatasetExt& dataset);
public:
	RecModel();
	void setup_train(ModelParam const& modelParam,
			std::shared_ptr<DatasetManager> datasetManager);
	virtual vector<rt::Recommendation> recommend(int64_t const& userId,
			map<int8_t, vector<rt::Interact> >& userInteracts) = 0;
	virtual string model_summary() = 0;
	virtual void dump_model_text(string const& filePrefix = "");
	TrainIterLog train(DatasetExt& trainSet, DatasetExt& testSet, DatasetExt& csSet);
	DatasetExt& get_active_ds() {
		return m_active_dataset;
	}
	DatasetExt& get_train_ds() {
		return m_dataset_manager->dataset(rt::DSType::DS_TRAIN);
	}
	DatasetExt& get_test_ds() {
		return m_dataset_manager->dataset(rt::DSType::DS_TEST);
	}
	DatasetExt& get_cs_ds() {
		return m_dataset_manager->dataset(rt::DSType::DS_CS);
	}
	DatasetExt& get_ds() {
		return m_dataset_manager->dataset(rt::DSType::DS_ALL);
	}
	ModelParam const& get_model_param() const{
		return m_model_param;
	}
	virtual ~RecModel();
};

struct RecommendationComparator {
	bool operator()(rt::Recommendation const& rt1,
			rt::Recommendation const& rt2) const {
		return rt1.score > rt2.score;
	}
};

ostream& operator <<(ostream& oss, RecModel::ModelParam const& param);
ostream& operator <<(ostream& oss, RecModel::TrainIterLog const& rhs);

/**
 * @brief evaluate the first and second moments for the sum/subtraction of two independent random variables
 *
 * @arguments:
 * x11	first moment of first random variable
 * x12 	second moment of the first random variable
 * x21	first moment of the second random variable
 * x22	second moment of the second random variable
 *
 * r1	first moment of the  sum/subtraction
 * r2	second moment of the  sum/subtraction
 *
 */
void sum_moments(float  x11, float x12, float x21, float x22, float& r1, float& r2);
void sub_moments(float  x11, float  x12, float x21, float x22, float& r1, float& r2);

} /* namespace recsys */

#endif /* MODEL_H_ */
