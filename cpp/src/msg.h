#pragma once

#include <cstdint>
#include <string>
#include <memory>

using namespace std;

using MsgId = uint32_t;

struct MsgBase
{
    const MsgId msg_id;
    const string msg_name;

    MsgBase(MsgId id, string name): msg_id(id), msg_name(name) {}
    virtual ~MsgBase() {}
};
using MsgPtr = shared_ptr<MsgBase>;

#define DEFINE_MSG(TypeName, MSG_ID, data_member)                                   \
struct TypeName##Msg : public MsgBase {                                             \
    TypeName data_member;                                                           \
    TypeName##Msg(): MsgBase(MSG_ID, #MSG_ID) {}                                    \
    TypeName##Msg(TypeName data): MsgBase(MSG_ID, #MSG_ID), data_member(data) {}    \
};                                                                                  \
using TypeName##MsgPtr = shared_ptr<TypeName##Msg>;   

