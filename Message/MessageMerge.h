#pragma once

#include "Message.h"

#include <vector>

class MessageMerge : public Message {
public:
    static const int NUMBER_RESULTS = 25;

    std::vector<std::string> subpartitionsURI;
    std::string partialResultURI;
private:
    virtual void serializeContents(std::stringstream &ss) const;
    virtual bool deserializeContents(std::stringstream &ss);
public:
    MessageMerge(Message::Type t);
    MessageMerge(Message::Type t, const std::vector<std::string> &subpartitionsURI, const std::string &partialResultURI);
    virtual void process(Socket &socket) const;
};
