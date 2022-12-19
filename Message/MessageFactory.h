#pragma once

#include "Message.h"

class MessageFactory {
public:
    Message* factoryMethod(std::stringstream &ss) const;
};
