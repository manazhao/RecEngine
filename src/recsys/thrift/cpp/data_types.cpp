/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "data_types.h"

#include <algorithm>

namespace recsys { namespace thrift {

const char* Interact::ascii_fingerprint = "2ED8AD9CE28DF04ABD9E0EB6395553D0";
const uint8_t Interact::binary_fingerprint[16] = {0x2E,0xD8,0xAD,0x9C,0xE2,0x8D,0xF0,0x4A,0xBD,0x9E,0x0E,0xB6,0x39,0x55,0x53,0xD0};

uint32_t Interact::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->ent_id);
          this->__isset.ent_id = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_DOUBLE) {
          xfer += iprot->readDouble(this->ent_val);
          this->__isset.ent_val = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t Interact::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("Interact");

  xfer += oprot->writeFieldBegin("ent_id", ::apache::thrift::protocol::T_I64, 1);
  xfer += oprot->writeI64(this->ent_id);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ent_val", ::apache::thrift::protocol::T_DOUBLE, 3);
  xfer += oprot->writeDouble(this->ent_val);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(Interact &a, Interact &b) {
  using ::std::swap;
  swap(a.ent_id, b.ent_id);
  swap(a.ent_val, b.ent_val);
  swap(a.__isset, b.__isset);
}

}} // namespace