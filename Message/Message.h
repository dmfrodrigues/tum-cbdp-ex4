#pragma once

#include <sstream>
#include <string>

class Socket;

class MessageFactory;

class Message {
    friend MessageFactory;

    virtual void serializeContents(std::stringstream &ss) const = 0;
    virtual bool deserializeContents(std::stringstream &ss) = 0;
public:
    static const size_t MAX_SIZE = 8192;

    enum Type : uint8_t {
        REQUEST = 1,
        RESPONSE = 2
    } type;
    enum Operation : uint8_t {
        HEARTBEAT = 1,
        SPLIT = 2,
        MERGE = 3
    } operation;

    Message(Type t, Operation op);

    std::string serialize() const;

    virtual ~Message();

    virtual void process(Socket &socket) const = 0;
};
