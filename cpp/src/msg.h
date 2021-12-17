#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "datatype/ids.h"
#include "datatype/types.h"

using namespace std;

struct MsgBase
{
    const DataId data_id;
    const string msg_name;

    MsgBase(DataId id, string name): data_id(id), msg_name(name) {}
    virtual ~MsgBase() {}
};
using MsgPtr = shared_ptr<MsgBase>;

#define DEFINE_MSG(Type, TYPE_ID)                                               \
struct Type##Msg : public MsgBase {                                             \
    Type data;                                                                  \
    Type##Msg(): MsgBase(DataType::TYPE_ID, #TYPE_ID) {}                        \
    Type##Msg(Type d): MsgBase(DataType::TYPE_ID, #TYPE_ID), data(d) {}     \
};                                                                                  \
using Type##MsgPtr = shared_ptr<Type##Msg>;
// TODO: Type##Msg(...): MsgBase(DataType::TYPE_ID, #TYPE_ID), data(...) {} 

DEFINE_MSG(SysInfo, SYS_INFO)

