/*
 * Model.h
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#ifndef MODEL_H_
#define MODEL_H_
#include "recsys/data/DatasetExt.h"
#include <boost/serialization/access.hpp>
#include "recsys/data/DatasetManager.h"
namespace rt = recsys::thrift;

namespace recsys {

class Model {
public:
	struct ModelParams{
	private:
		friend class boost::serialization::access;
		template <class Archive>
		void serialize(Archive& ar, const unsigned int version ){
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
		ModelParams(size_t const& latDim = 10, size_t const& maxIter = 10, bool diagCov = true, bool useFeature = true);
		ModelParams(int argc, char** argv);
	};
protected:
	size_t m_num_users;
	size_t m_num_items;
	size_t m_num_features;
	ModelParams m_model_param;
	DatasetManager& m_dataset_manager;
protected:
	virtual void _init() = 0;
public:
	Model(ModelParams const& modelParam, DatasetManager& datasetManager);
	DatasetExt& get_train_ds(){
		return m_dataset_manager.dataset(rt::DSType::DS_TRAIN);
	}
	DatasetExt& get_test_ds(){
		return m_dataset_manager.dataset(rt::DSType::DS_TEST);
	}
	DatasetExt& get_cs_ds(){
		return m_dataset_manager.dataset(rt::DSType::DS_CS);
	}
	DatasetExt& get_ds(){
		return m_dataset_manager.dataset(rt::DSType::DS_ALL);
	}
	virtual void train() = 0;
	virtual ~Model();
};

} /* namespace recsys */

#endif /* MODEL_H_ */
