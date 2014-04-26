/*
 * Model.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/Model.h>

namespace recsys {

Model::ModelParams::ModelParams(size_t const& latDim,
		size_t const& maxIter, bool diagCov, bool useFeature) :
		m_lat_dim(latDim), m_max_iter(maxIter), m_diag_cov(diagCov), m_use_feature(
				useFeature) {

}

Model::ModelParams::ModelParams(int argc, char** argv) :
		m_lat_dim(5), m_max_iter(10), m_diag_cov(true), m_use_feature(true) {
	/// parse model parameters from the command line
	try {
		po::options_description desc(
				"Run Hierarchical Bayesian Hybrid Matrix Factorization Model");
		desc.add_options()("help,h", "help message on use this application")(
				"lat-dim", po::value<size_t>(&m_lat_dim), "latent dimension")(
				"diag-cov", po::value<bool>(&m_diag_cov),
				"diagonal multivariate Gaussian")("max-iter",
				po::value<size_t>(&m_max_iter), "maximum number of iterations")(
				"use-feature", po::value<bool>(&m_use_feature),
				"integrate content feature as prior");
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			cout << desc << "\n";
			exit(1);
		}
		/// check all required options are provided
		vm.notify();
	} catch (std::exception& e) {
		cerr << "Error:" << e.what() << endl;
		exit(1);
	}
}

ostream& operator <<(ostream& oss,
		Model::ModelParams const& rhs) {
	oss << "------------ Model Parameters ------------" << endl;
	oss << "latent dimension:" << rhs.m_lat_dim << ", number of iterations:"
			<< rhs.m_max_iter << ",is diagonal:" << rhs.m_diag_cov
			<< ",use feature:" << rhs.m_use_feature << endl;
	return oss;
}


Model::Model(ModelParams const& modelParam,DatasetManager& datasetManager)
:m_model_param(modelParam),m_dataset_manager(datasetManager){
	// TODO Auto-generated constructor stub

}

Model::~Model() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
