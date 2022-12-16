#pragma once

#include "Message.h"
#include "MessageHeartbeat.h"
#include "MessageWork.h"

class MessageFactory {
public:
    Message* factoryMethod(std::stringstream &ss) const;
};
