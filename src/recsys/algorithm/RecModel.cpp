/*
 * Model.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#include <recsys/algorithm/RecModel.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace recsys {

ostream& operator <<(ostream& oss, RecModel::TrainIterLog const& rhs) {
	oss << "iteration:" << rhs.m_iter << ",time:" << rhs.m_iter_time
			<< ",train rmse:" << rhs.m_train_rmse << ",test rmse:"
			<< rhs.m_test_rmse << ",cs rmse:" << rhs.m_cs_rmse << endl;
	return oss;
}


RecModel::ModelParams::ModelParams(size_t const& latDim, size_t const& maxIter,
		bool diagCov, bool useFeature) :
	m_lat_dim(latDim), m_max_iter(maxIter), m_diag_cov(diagCov), m_use_feature(
			useFeature) {

}

void RecModel::setup_training(ModelParams const& modelParam, shared_ptr<
		DatasetManager> datasetManager){
	m_model_param = modelParam;
	m_dataset_manager = datasetManager;
	/// initialize training envrionment
	_init_training();
}

void RecModel::model_selection(){
	/// train the model on the training dataset and evaluate on the testing and coldstart dataset
	for(size_t iterNum = 1; iterNum <= m_model_param.m_max_iter; iterNum++){
		timer timer;
		TrainIterLog iterLog = _training_single_iteration(m_dataset_manager->dataset(rt::DSType::DS_TRAIN));
		iterLog.m_train_rmse = _dataset_rmse(m_dataset_manager->dataset(rt::DSType::DS_TRAIN));
		iterLog.m_test_rmse = _dataset_rmse(m_dataset_manager->dataset(rt::DSType::DS_TEST));
		iterLog.m_cs_rmse = _dataset_rmse(m_dataset_manager->dataset(rt::DSType::DS_CS));
		iterLog.m_iter_time = timer.elapsed();
		cout << iterLog << endl;
	}
}

void RecModel::train(){
	for(size_t iterNum = 1; iterNum <= m_model_param.m_max_iter; iterNum++){
		TrainIterLog iterLog = _training_single_iteration(m_dataset_manager->dataset(rt::DSType::DS_ALL));
	}
}

float RecModel::_dataset_rmse(DatasetExt& dataset){
	/// evaluate the rmse of the training dataset
	vector<int64_t>& userIds = dataset.m_id_type_map[Entity::ENT_USER];
	size_t numRating = 0;
	float rmse = 0;
	for(vector<int64_t>::iterator iter = userIds.begin(); iter < userIds.end(); ++iter){
		int64_t userId = *iter;
		map<int8_t,vector<Interact> >& userInteracts = dataset.type_ent_ids[userId];
		size_t userNumRating =  userInteracts[EntityInteraction::RATE_ITEM];
		/// evaluate the prediction error
		float userError = _pred_error(userId, userInteracts);
		rmse += userError;
		numRating += userNumRating;
	}
	return sqrt(rmse/numRating);
}

RecModel::ModelParams::ModelParams(int argc, char** argv) :
	m_lat_dim(5), m_max_iter(10), m_diag_cov(true), m_use_feature(true) {
	/// parse model parameters from the command line
	po::options_description desc(
			"Run Hierarchical Bayesian Hybrid Matrix Factorization Model");
	desc.add_options()("help,h", "help message on use this application")(
			"lat-dim", po::value<size_t>(&m_lat_dim), "latent dimension")(
			"diag-cov", po::value<bool>(&m_diag_cov),
			"diagonal multivariate Gaussian")("max-iter", po::value<size_t>(
			&m_max_iter), "maximum number of iterations")("use-feature",
			po::value<bool>(&m_use_feature),
			"integrate content feature as prior");
	po::variables_map vm;
	try {
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

ostream& operator <<(ostream& oss, RecModel::ModelParams const& rhs) {
	oss << "------------ Model Parameters ------------" << endl;
	oss << "latent dimension:" << rhs.m_lat_dim << ", number of iterations:"
			<< rhs.m_max_iter << ",is diagonal:" << rhs.m_diag_cov
			<< ",use feature:" << rhs.m_use_feature << endl;
	return oss;
}

RecModel::RecModel() {
	// TODO Auto-generated constructor stub

}


RecModel::~RecModel() {
	// TODO Auto-generated destructor stub
}

} /* namespace recsys */
