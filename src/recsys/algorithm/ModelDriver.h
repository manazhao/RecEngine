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
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace recsys {

class ModelDriver {
protected:
	string m_model_name;
	string m_model_file;
	shared_ptr<RecModel> m_model_ptr;
	ModelDriver(){

	}
	ModelDriver& operator=(ModelDriver const& rhs);
	ModelDriver(ModelDriver const& rhs);
	void _save_model();
	void _load_model();
private:
	friend class boost::serialization::access;
	template <class Archive>
	void load(Archive& ar, const unsigned int version ){
		ar & m_model_name;
		/// generate the Model based on the model name
		if(m_model_name == "HHMF"){
			m_model_ptr = shared_ptr<RecModel>(new HierarchicalHybridMF());
			HierarchicalHybridMF& modelRef = dynamic_cast<HierarchicalHybridMF&>(*m_model_ptr);
			ar & modelRef;
		}else{
			cerr << "unknown model" << endl;
			exit(1);
		}
	}

	template <class Archive>
	void save(Archive& ar, const unsigned int version ) const{
		ar & m_model_name;
		if(m_model_name == "HHMF"){
			HierarchicalHybridMF& modelRef = dynamic_cast<HierarchicalHybridMF&>(*m_model_ptr);
			ar & modelRef;
		}else{
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
		return modelName == "HHMF";
	}
	/// get model reference
	RecModel& get_model_ref(){
		return *m_model_ptr;
	}
	void run_from_cmd(int argc, char** argv);
	virtual ~ModelDriver();
};

}

#endif /* MODELDRIVER_H_ */
