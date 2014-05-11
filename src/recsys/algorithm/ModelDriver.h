/*
 * ModelDriver.h
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#ifndef MODELDRIVER_H_
#define MODELDRIVER_H_
#include <recsys/data/EntityInteraction.h>
#include <recsys/data/AppConfig.h>
#include <recsys/data/ThriftDataLoader.h>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <recsys/algorithm/HierarchicalHybridMF.h>
#include <recsys/algorithm/AverageRatingModel.h>
#include <recsys/algorithm/PopularityModel.h>
#include <recsys/algorithm/HHMFBias.h>
#include <recsys/algorithm/BayesianBiasModel.h>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace recsys {

class ModelDriver {
public:
	typedef shared_ptr<RecModel> rec_model_ptr;
protected:
	string m_dataset_name;
	string m_model_name;
	string m_model_file;
	map<string,rec_model_ptr > m_models;
	ModelDriver(){
		m_models["HHMF"] = rec_model_ptr(new HierarchicalHybridMF());
		m_models["AVG"] = rec_model_ptr(new AverageRatingModel());
		m_models["POP"] = rec_model_ptr(new PopularityModel());
		m_models["HHMFB"] = rec_model_ptr(new HHMFBias());
		m_models["BB"] = rec_model_ptr(new BayesianBiasModel());
	}
	ModelDriver& operator=(ModelDriver const& rhs);
	ModelDriver(ModelDriver const& rhs);
	void _save_model();
	void _load_model();

private:
	friend class boost::serialization::access;
	template <class Archive>
	void load(Archive& ar, const unsigned int version ){
		ar & m_dataset_name & m_model_name;
		/// generate the Model based on the model name
		if(m_model_name == "HHMF"){
			HierarchicalHybridMF& modelRef = dynamic_cast<HierarchicalHybridMF&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "AVG"){
			AverageRatingModel& modelRef = dynamic_cast<AverageRatingModel&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "POP"){
			PopularityModel& modelRef = dynamic_cast<PopularityModel&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "HHMFB"){
			HHMFBias& modelRef = dynamic_cast<HHMFBias&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "BB"){
			BayesianBiasModel& modelRef = dynamic_cast<BayesianBiasModel&>(get_model_ref());
			ar & modelRef;
		}
		else{
			cerr << "unknown model" << endl;
			exit(1);
		}
	}

	template <class Archive>
	void save(Archive& ar, const unsigned int version ) const{
		ar & m_dataset_name & m_model_name;
		if(m_model_name == "HHMF"){
			HierarchicalHybridMF const& modelRef = dynamic_cast<HierarchicalHybridMF const&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "AVG"){
			AverageRatingModel const& modelRef = dynamic_cast<AverageRatingModel const&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "POP"){
			PopularityModel const& modelRef = dynamic_cast<PopularityModel const&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "HHMFB"){
			HHMFBias const& modelRef = dynamic_cast<HHMFBias const&>(get_model_ref());
			ar & modelRef;
		}else if(m_model_name == "BB"){
			BayesianBiasModel const& modelRef = dynamic_cast<BayesianBiasModel const&>(get_model_ref());
			ar & modelRef;
		}
		else{
			cerr << "unknown model" << endl;
			exit(1);
		}
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER();
public:
	static ModelDriver& ref(){
		static ModelDriver MODEL_DRIVER;
		return MODEL_DRIVER;
	}
	bool is_model_supported(string const& modelName){
		return m_models.find(modelName) != m_models.end();
	}
	/// get model reference
	RecModel & get_model_ref() {
		return *(m_models[m_model_name]);
	}

	string const& get_model_name() const{
		return m_model_name;
	}

	string const& get_dataset_name() const{
		return m_dataset_name;
	}
	/// get model reference
	RecModel const& get_model_ref() const{
		return *(m_models.at(m_model_name));
	}

	void run_from_cmd(int argc, char** argv);
	virtual ~ModelDriver();
};

}

#endif /* MODELDRIVER_H_ */
