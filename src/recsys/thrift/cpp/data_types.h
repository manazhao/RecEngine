/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef data_TYPES_H
#define data_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace recsys { namespace thrift {

struct DSType {
  enum type {
    DS_ALL = 0,
    DS_TRAIN = 1,
    DS_TEST = 2,
    DS_CS = 3
  };
};

extern const std::map<int, const char*> _DSType_VALUES_TO_NAMES;

struct StatusCode {
  enum type {
    SC_FAIL = 0,
    SC_SUCCESS = 1
  };
};

extern const std::map<int, const char*> _StatusCode_VALUES_TO_NAMES;

typedef struct _Interact__isset {
  _Interact__isset() : ent_id(false), ent_val(false) {}
  bool ent_id;
  bool ent_val;
} _Interact__isset;

class Interact {
 public:

  static const char* ascii_fingerprint; // = "056BD45B5249CAA453D3C7B115F349DB";
  static const uint8_t binary_fingerprint[16]; // = {0x05,0x6B,0xD4,0x5B,0x52,0x49,0xCA,0xA4,0x53,0xD3,0xC7,0xB1,0x15,0xF3,0x49,0xDB};

  Interact() : ent_id(0), ent_val(0) {
  }

  virtual ~Interact() throw() {}

  int64_t ent_id;
  double ent_val;

  _Interact__isset __isset;

  void __set_ent_id(const int64_t val) {
    ent_id = val;
  }

  void __set_ent_val(const double val) {
    ent_val = val;
  }

  bool operator == (const Interact & rhs) const
  {
    if (!(ent_id == rhs.ent_id))
      return false;
    if (!(ent_val == rhs.ent_val))
      return false;
    return true;
  }
  bool operator != (const Interact &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Interact & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Interact &a, Interact &b);

typedef struct _Dataset__isset {
  _Dataset__isset() : type_ent_ids(false), ent_ids(false), ent_type_interacts(false) {}
  bool type_ent_ids;
  bool ent_ids;
  bool ent_type_interacts;
} _Dataset__isset;

class Dataset {
 public:

  static const char* ascii_fingerprint; // = "24BB30CDDB71AC126E5BAD553681285A";
  static const uint8_t binary_fingerprint[16]; // = {0x24,0xBB,0x30,0xCD,0xDB,0x71,0xAC,0x12,0x6E,0x5B,0xAD,0x55,0x36,0x81,0x28,0x5A};

  Dataset() {
  }

  virtual ~Dataset() throw() {}

  std::map<int8_t, std::set<int64_t> >  type_ent_ids;
  std::set<int64_t>  ent_ids;
  std::vector<std::map<int8_t, std::vector<Interact> > >  ent_type_interacts;

  _Dataset__isset __isset;

  void __set_type_ent_ids(const std::map<int8_t, std::set<int64_t> > & val) {
    type_ent_ids = val;
  }

  void __set_ent_ids(const std::set<int64_t> & val) {
    ent_ids = val;
  }

  void __set_ent_type_interacts(const std::vector<std::map<int8_t, std::vector<Interact> > > & val) {
    ent_type_interacts = val;
  }

  bool operator == (const Dataset & rhs) const
  {
    if (!(type_ent_ids == rhs.type_ent_ids))
      return false;
    if (!(ent_ids == rhs.ent_ids))
      return false;
    if (!(ent_type_interacts == rhs.ent_type_interacts))
      return false;
    return true;
  }
  bool operator != (const Dataset &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Dataset & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Dataset &a, Dataset &b);

typedef struct _Recommendation__isset {
  _Recommendation__isset() : id(false), type(false), score(false) {}
  bool id;
  bool type;
  bool score;
} _Recommendation__isset;

class Recommendation {
 public:

  static const char* ascii_fingerprint; // = "957FDF3757DFE4CA0D964CCD421BF703";
  static const uint8_t binary_fingerprint[16]; // = {0x95,0x7F,0xDF,0x37,0x57,0xDF,0xE4,0xCA,0x0D,0x96,0x4C,0xCD,0x42,0x1B,0xF7,0x03};

  Recommendation() : id(), type(0), score(0) {
  }

  virtual ~Recommendation() throw() {}

  std::string id;
  int8_t type;
  double score;

  _Recommendation__isset __isset;

  void __set_id(const std::string& val) {
    id = val;
  }

  void __set_type(const int8_t val) {
    type = val;
  }

  void __set_score(const double val) {
    score = val;
  }

  bool operator == (const Recommendation & rhs) const
  {
    if (!(id == rhs.id))
      return false;
    if (!(type == rhs.type))
      return false;
    if (!(score == rhs.score))
      return false;
    return true;
  }
  bool operator != (const Recommendation &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Recommendation & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Recommendation &a, Recommendation &b);

typedef struct _Response__isset {
  _Response__isset() : status(false), message(false) {}
  bool status;
  bool message;
} _Response__isset;

class Response {
 public:

  static const char* ascii_fingerprint; // = "19B5240589E680301A7E32DF3971EFBE";
  static const uint8_t binary_fingerprint[16]; // = {0x19,0xB5,0x24,0x05,0x89,0xE6,0x80,0x30,0x1A,0x7E,0x32,0xDF,0x39,0x71,0xEF,0xBE};

  Response() : status((StatusCode::type)0), message() {
  }

  virtual ~Response() throw() {}

  StatusCode::type status;
  std::string message;

  _Response__isset __isset;

  void __set_status(const StatusCode::type val) {
    status = val;
  }

  void __set_message(const std::string& val) {
    message = val;
  }

  bool operator == (const Response & rhs) const
  {
    if (!(status == rhs.status))
      return false;
    if (!(message == rhs.message))
      return false;
    return true;
  }
  bool operator != (const Response &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Response & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Response &a, Response &b);

}} // namespace

#endif
