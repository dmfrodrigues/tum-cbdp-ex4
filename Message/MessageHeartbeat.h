#pragma once

#include "Message.h"

class MessageHeartbeat: public Message {
private:
    virtual void serializeContents(std::stringstream &ss) const;
    virtual bool deserializeContents(std::stringstream &ss);
public:
    MessageHeartbeat(Message::Type t);
};
