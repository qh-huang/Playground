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

#define DEFINE_MSG(TypeName, TYPE_ID)                                               \
struct TypeName##Msg : public MsgBase {                                             \
    TypeName data;                                                                  \
    TypeName##Msg(): MsgBase(DataType::TYPE_ID, #TYPE_ID) {}                        \
    TypeName##Msg(TypeName d): MsgBase(DataType::TYPE_ID, #TYPE_ID), data(d) {}     \
};                                                                                  \
using TypeName##MsgPtr = shared_ptr<TypeName##Msg>;
// TODO: TypeName##Msg(...): MsgBase(DataType::TYPE_ID, #TYPE_ID), data(...) {} 

DEFINE_MSG(SysInfo, SYS_INFO)

