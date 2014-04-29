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

namespace recsys {

class RecModel {
	friend class ModelDriver;
public:
	struct ModelParams {
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
		ModelParams(size_t const& latDim = 10, size_t const& maxIter = 10,
				bool diagCov = true, bool useFeature = true);
		ModelParams(int argc, char** argv);
		operator string() {
			stringstream ss;
			ss << "d_" << m_lat_dim << "-f" << "_" << m_use_feature;
			return ss.str();
		}
	};

	struct TrainIterLog{
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
		TrainIterLog():m_iter(0),m_train_rmse(0),m_test_rmse(0),m_cs_rmse(0),m_iter_time(0){

		}
	};

protected:
	size_t m_num_users;
	size_t m_num_items;
	size_t m_num_features;
	ModelParams m_model_param;
	DatasetExt m_active_dataset;
	shared_ptr<DatasetManager> m_dataset_manager;
	bool m_model_selection;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		/// serialize every data member except for the dataset
		ar & m_num_users & m_num_items & m_num_features & m_model_param & m_active_dataset;
	}

protected:
	virtual void _init_training() = 0;
	virtual TrainIterLog _train_update() = 0;
	virtual float _pred_error(int64_t const& entityId, map<int8_t, vector<Interact> >& entityInteractMap) = 0;
	virtual void _add_new_entity(int64_t const& entityId, int8_t const& entityType) = 0;
	float _dataset_rmse(DatasetExt& dataset);
public:
	RecModel();
	void setup_train(ModelParams const& modelParam, shared_ptr<
			DatasetManager> datasetManager);
	virtual vector<rt::Recommendation> recommend(int64_t const& userId, map<int8_t, vector<rt::Interact> >& userInteracts) = 0;
	virtual string model_summary() = 0;
	void train(DatasetExt& trainSet, DatasetExt& testSet, DatasetExt& csSet);
	DatasetExt& get_active_ds(){
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
	virtual ~RecModel();
};

struct RecommendationComparator{
	bool operator()(rt::Recommendation const& rt1, rt::Recommendation const& rt2) const{
		return rt1.score > rt2.score;
	}
};

ostream& operator <<(ostream& oss, RecModel::ModelParams const& param);
ostream& operator << (ostream& oss, RecModel::TrainIterLog const& rhs);

} /* namespace recsys */

#endif /* MODEL_H_ */
