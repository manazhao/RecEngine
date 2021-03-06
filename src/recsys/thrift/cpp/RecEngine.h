/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef RecEngine_H
#define RecEngine_H

#include <thrift/TDispatchProcessor.h>
#include "data_types.h"

namespace recsys { namespace thrift {

class RecEngineIf {
 public:
  virtual ~RecEngineIf() {}
  virtual void get_recommendation(std::vector<Recommendation> & _return, const std::string& userId) = 0;
};

class RecEngineIfFactory {
 public:
  typedef RecEngineIf Handler;

  virtual ~RecEngineIfFactory() {}

  virtual RecEngineIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(RecEngineIf* /* handler */) = 0;
};

class RecEngineIfSingletonFactory : virtual public RecEngineIfFactory {
 public:
  RecEngineIfSingletonFactory(const boost::shared_ptr<RecEngineIf>& iface) : iface_(iface) {}
  virtual ~RecEngineIfSingletonFactory() {}

  virtual RecEngineIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(RecEngineIf* /* handler */) {}

 protected:
  boost::shared_ptr<RecEngineIf> iface_;
};

class RecEngineNull : virtual public RecEngineIf {
 public:
  virtual ~RecEngineNull() {}
  void get_recommendation(std::vector<Recommendation> & /* _return */, const std::string& /* userId */) {
    return;
  }
};

typedef struct _RecEngine_get_recommendation_args__isset {
  _RecEngine_get_recommendation_args__isset() : userId(false) {}
  bool userId;
} _RecEngine_get_recommendation_args__isset;

class RecEngine_get_recommendation_args {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  RecEngine_get_recommendation_args() : userId() {
  }

  virtual ~RecEngine_get_recommendation_args() throw() {}

  std::string userId;

  _RecEngine_get_recommendation_args__isset __isset;

  void __set_userId(const std::string& val) {
    userId = val;
  }

  bool operator == (const RecEngine_get_recommendation_args & rhs) const
  {
    if (!(userId == rhs.userId))
      return false;
    return true;
  }
  bool operator != (const RecEngine_get_recommendation_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RecEngine_get_recommendation_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class RecEngine_get_recommendation_pargs {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};


  virtual ~RecEngine_get_recommendation_pargs() throw() {}

  const std::string* userId;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _RecEngine_get_recommendation_result__isset {
  _RecEngine_get_recommendation_result__isset() : success(false) {}
  bool success;
} _RecEngine_get_recommendation_result__isset;

class RecEngine_get_recommendation_result {
 public:

  static const char* ascii_fingerprint; // = "7BF4A3FE2531F7AC02E2435499B1825C";
  static const uint8_t binary_fingerprint[16]; // = {0x7B,0xF4,0xA3,0xFE,0x25,0x31,0xF7,0xAC,0x02,0xE2,0x43,0x54,0x99,0xB1,0x82,0x5C};

  RecEngine_get_recommendation_result() {
  }

  virtual ~RecEngine_get_recommendation_result() throw() {}

  std::vector<Recommendation>  success;

  _RecEngine_get_recommendation_result__isset __isset;

  void __set_success(const std::vector<Recommendation> & val) {
    success = val;
  }

  bool operator == (const RecEngine_get_recommendation_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const RecEngine_get_recommendation_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RecEngine_get_recommendation_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _RecEngine_get_recommendation_presult__isset {
  _RecEngine_get_recommendation_presult__isset() : success(false) {}
  bool success;
} _RecEngine_get_recommendation_presult__isset;

class RecEngine_get_recommendation_presult {
 public:

  static const char* ascii_fingerprint; // = "7BF4A3FE2531F7AC02E2435499B1825C";
  static const uint8_t binary_fingerprint[16]; // = {0x7B,0xF4,0xA3,0xFE,0x25,0x31,0xF7,0xAC,0x02,0xE2,0x43,0x54,0x99,0xB1,0x82,0x5C};


  virtual ~RecEngine_get_recommendation_presult() throw() {}

  std::vector<Recommendation> * success;

  _RecEngine_get_recommendation_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class RecEngineClient : virtual public RecEngineIf {
 public:
  RecEngineClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  RecEngineClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void get_recommendation(std::vector<Recommendation> & _return, const std::string& userId);
  void send_get_recommendation(const std::string& userId);
  void recv_get_recommendation(std::vector<Recommendation> & _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class RecEngineProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<RecEngineIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (RecEngineProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_get_recommendation(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  RecEngineProcessor(boost::shared_ptr<RecEngineIf> iface) :
    iface_(iface) {
    processMap_["get_recommendation"] = &RecEngineProcessor::process_get_recommendation;
  }

  virtual ~RecEngineProcessor() {}
};

class RecEngineProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  RecEngineProcessorFactory(const ::boost::shared_ptr< RecEngineIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< RecEngineIfFactory > handlerFactory_;
};

class RecEngineMultiface : virtual public RecEngineIf {
 public:
  RecEngineMultiface(std::vector<boost::shared_ptr<RecEngineIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~RecEngineMultiface() {}
 protected:
  std::vector<boost::shared_ptr<RecEngineIf> > ifaces_;
  RecEngineMultiface() {}
  void add(boost::shared_ptr<RecEngineIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void get_recommendation(std::vector<Recommendation> & _return, const std::string& userId) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->get_recommendation(_return, userId);
    }
    ifaces_[i]->get_recommendation(_return, userId);
    return;
  }

};

}} // namespace

#endif
