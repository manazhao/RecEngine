/*
 * DataLoaderSwitcher.h
 *
 *  Created on: Apr 25, 2014
 *      Author: manazhao
 */

#ifndef DATALOADERSWITCHER_H_
#define DATALOADERSWITCHER_H_
#include "recsys/data/JSONDataLoader.h"
#include "recsys/data/AmazonParser.h"
#include "recsys/data/MovieLensParser.h"
#include "recsys/data/ThriftDataLoader.h"

namespace recsys {

/**
 * @brief choose the data loader based on the input arguments
 */
class DataLoaderSwitcher {
protected:
	boost::shared_ptr<DataLoader> m_data_loader_ptr;
	map<string, map<string, boost::shared_ptr<EntityParser> > > m_dataset_parser_map;
protected:
	DataLoaderSwitcher();
	bool _is_dataset_supported(string const& datasetName);
	void _register_entity_parsers();
	boost::shared_ptr<DataLoader> _create_remote_loader(string const& host, int& port);
	boost::shared_ptr<DataLoader> _create_local_loader(string const& datasetName, string const& userFile,
			string const& itemFile, string const& ratingFile);
public:
	/**
	 * @brief get a data loader based on the arguments
	 */
	boost::shared_ptr<DataLoader> get_loader(int argc, char** argv);
	boost::shared_ptr<DataLoader> get_remote_loader(int argc, char** argv);
	boost::shared_ptr<DataLoader> get_local_loader(int argc, char** argv);
	boost::shared_ptr<DataLoader> get_remote_loader(string const& host, int port);
	boost::shared_ptr<DataLoader> get_local_loader(string const& datasetName, string const& userFile, string const& itemFile, string const& ratingFile);
	static DataLoaderSwitcher& ref(){
		static DataLoaderSwitcher switcher;
		return switcher;
	}
	virtual ~DataLoaderSwitcher();
};

}

#endif /* DATALOADERSWITCHER_H_ */
