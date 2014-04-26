/*
 * ThriftDataLoader.h
 *
 *  Created on: Apr 25, 2014
 *      Author: qzhao2
 */

#ifndef THRIFTDATALOADER_H_
#define THRIFTDATALOADER_H_
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "recsys/thrift/cpp/HandleData.h"
#include "recsys/data/DatasetExt.h"
#include "DatasetManager.h"
#include "DataLoader.h"

using namespace ::recsys::thrift;
using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
namespace rt = recsys::thrift;

namespace recsys {

class ThriftDataLoader : public DataLoader{
protected:
	string m_host;
	int m_port;
	/// used by thrift client
	boost::shared_ptr<TTransport> m_socket;
	boost::shared_ptr<TTransport> m_transport;
	boost::shared_ptr<TProtocol> m_protocol;
	rt::HandleDataClient m_client;
public:
	ThriftDataLoader(string const& host = "localhost", int port = 9090);
	virtual ~ThriftDataLoader();
};

} /* namespace recsys */

#endif /* THRIFTDATALOADER_H_ */
