#pragma once

#include "Message.h"

#include <vector>

class MessageSplit : public Message {
public:
    static const int NUMBER_SUBPARTITIONS = 3;

    std::string partitionURI;
    std::vector<std::string> subpartitionsURI;
private:
    virtual void serializeContents(std::stringstream &ss) const;
    virtual bool deserializeContents(std::stringstream &ss);

    static std::string extractDomain(const std::string &s);
public:
    MessageSplit(Message::Type t);
    MessageSplit(Message::Type t, const std::string &partitionURI);
    virtual void process(Socket &socket) const;
};
