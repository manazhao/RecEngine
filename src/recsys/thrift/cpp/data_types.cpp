/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "data_types.h"

#include <algorithm>

namespace recsys { namespace thrift {

int _kDSTypeValues[] = {
  DSType::DS_ALL,
  DSType::DS_TRAIN,
  DSType::DS_TEST,
  DSType::DS_CS
};
const char* _kDSTypeNames[] = {
  "DS_ALL",
  "DS_TRAIN",
  "DS_TEST",
  "DS_CS"
};
const std::map<int, const char*> _DSType_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(4, _kDSTypeValues, _kDSTypeNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

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

const char* Dataset::ascii_fingerprint = "FB6671B65EB56CC7DB3CAA10D6B3EFB5";
const uint8_t Dataset::binary_fingerprint[16] = {0xFB,0x66,0x71,0xB6,0x5E,0xB5,0x6C,0xC7,0xDB,0x3C,0xAA,0x10,0xD6,0xB3,0xEF,0xB5};

uint32_t Dataset::read(::apache::thrift::protocol::TProtocol* iprot) {

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
        if (ftype == ::apache::thrift::protocol::T_MAP) {
          {
            this->type_ent_ids.clear();
            uint32_t _size0;
            ::apache::thrift::protocol::TType _ktype1;
            ::apache::thrift::protocol::TType _vtype2;
            xfer += iprot->readMapBegin(_ktype1, _vtype2, _size0);
            uint32_t _i4;
            for (_i4 = 0; _i4 < _size0; ++_i4)
            {
              int8_t _key5;
              xfer += iprot->readByte(_key5);
              std::set<int64_t> & _val6 = this->type_ent_ids[_key5];
              {
                _val6.clear();
                uint32_t _size7;
                ::apache::thrift::protocol::TType _etype10;
                xfer += iprot->readSetBegin(_etype10, _size7);
                uint32_t _i11;
                for (_i11 = 0; _i11 < _size7; ++_i11)
                {
                  int64_t _elem12;
                  xfer += iprot->readI64(_elem12);
                  _val6.insert(_elem12);
                }
                xfer += iprot->readSetEnd();
              }
            }
            xfer += iprot->readMapEnd();
          }
          this->__isset.type_ent_ids = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_SET) {
          {
            this->ent_ids.clear();
            uint32_t _size13;
            ::apache::thrift::protocol::TType _etype16;
            xfer += iprot->readSetBegin(_etype16, _size13);
            uint32_t _i17;
            for (_i17 = 0; _i17 < _size13; ++_i17)
            {
              int64_t _elem18;
              xfer += iprot->readI64(_elem18);
              this->ent_ids.insert(_elem18);
            }
            xfer += iprot->readSetEnd();
          }
          this->__isset.ent_ids = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->ent_type_interacts.clear();
            uint32_t _size19;
            ::apache::thrift::protocol::TType _etype22;
            xfer += iprot->readListBegin(_etype22, _size19);
            this->ent_type_interacts.resize(_size19);
            uint32_t _i23;
            for (_i23 = 0; _i23 < _size19; ++_i23)
            {
              {
                this->ent_type_interacts[_i23].clear();
                uint32_t _size24;
                ::apache::thrift::protocol::TType _ktype25;
                ::apache::thrift::protocol::TType _vtype26;
                xfer += iprot->readMapBegin(_ktype25, _vtype26, _size24);
                uint32_t _i28;
                for (_i28 = 0; _i28 < _size24; ++_i28)
                {
                  int8_t _key29;
                  xfer += iprot->readByte(_key29);
                  std::vector<Interact> & _val30 = this->ent_type_interacts[_i23][_key29];
                  {
                    _val30.clear();
                    uint32_t _size31;
                    ::apache::thrift::protocol::TType _etype34;
                    xfer += iprot->readListBegin(_etype34, _size31);
                    _val30.resize(_size31);
                    uint32_t _i35;
                    for (_i35 = 0; _i35 < _size31; ++_i35)
                    {
                      xfer += _val30[_i35].read(iprot);
                    }
                    xfer += iprot->readListEnd();
                  }
                }
                xfer += iprot->readMapEnd();
              }
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.ent_type_interacts = true;
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

uint32_t Dataset::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("Dataset");

  xfer += oprot->writeFieldBegin("type_ent_ids", ::apache::thrift::protocol::T_MAP, 1);
  {
    xfer += oprot->writeMapBegin(::apache::thrift::protocol::T_BYTE, ::apache::thrift::protocol::T_SET, static_cast<uint32_t>(this->type_ent_ids.size()));
    std::map<int8_t, std::set<int64_t> > ::const_iterator _iter36;
    for (_iter36 = this->type_ent_ids.begin(); _iter36 != this->type_ent_ids.end(); ++_iter36)
    {
      xfer += oprot->writeByte(_iter36->first);
      {
        xfer += oprot->writeSetBegin(::apache::thrift::protocol::T_I64, static_cast<uint32_t>(_iter36->second.size()));
        std::set<int64_t> ::const_iterator _iter37;
        for (_iter37 = _iter36->second.begin(); _iter37 != _iter36->second.end(); ++_iter37)
        {
          xfer += oprot->writeI64((*_iter37));
        }
        xfer += oprot->writeSetEnd();
      }
    }
    xfer += oprot->writeMapEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ent_ids", ::apache::thrift::protocol::T_SET, 2);
  {
    xfer += oprot->writeSetBegin(::apache::thrift::protocol::T_I64, static_cast<uint32_t>(this->ent_ids.size()));
    std::set<int64_t> ::const_iterator _iter38;
    for (_iter38 = this->ent_ids.begin(); _iter38 != this->ent_ids.end(); ++_iter38)
    {
      xfer += oprot->writeI64((*_iter38));
    }
    xfer += oprot->writeSetEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ent_type_interacts", ::apache::thrift::protocol::T_LIST, 3);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_MAP, static_cast<uint32_t>(this->ent_type_interacts.size()));
    std::vector<std::map<int8_t, std::vector<Interact> > > ::const_iterator _iter39;
    for (_iter39 = this->ent_type_interacts.begin(); _iter39 != this->ent_type_interacts.end(); ++_iter39)
    {
      {
        xfer += oprot->writeMapBegin(::apache::thrift::protocol::T_BYTE, ::apache::thrift::protocol::T_LIST, static_cast<uint32_t>((*_iter39).size()));
        std::map<int8_t, std::vector<Interact> > ::const_iterator _iter40;
        for (_iter40 = (*_iter39).begin(); _iter40 != (*_iter39).end(); ++_iter40)
        {
          xfer += oprot->writeByte(_iter40->first);
          {
            xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(_iter40->second.size()));
            std::vector<Interact> ::const_iterator _iter41;
            for (_iter41 = _iter40->second.begin(); _iter41 != _iter40->second.end(); ++_iter41)
            {
              xfer += (*_iter41).write(oprot);
            }
            xfer += oprot->writeListEnd();
          }
        }
        xfer += oprot->writeMapEnd();
      }
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(Dataset &a, Dataset &b) {
  using ::std::swap;
  swap(a.type_ent_ids, b.type_ent_ids);
  swap(a.ent_ids, b.ent_ids);
  swap(a.ent_type_interacts, b.ent_type_interacts);
  swap(a.__isset, b.__isset);
}

}} // namespace
