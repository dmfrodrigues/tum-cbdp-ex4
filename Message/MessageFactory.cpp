#include "MessageFactory.h"

#include <iostream>

#include "MessageHeartbeat.h"
#include "MessageSplit.h"
#include "MessageMerge.h"

using namespace std;

Message* MessageFactory::factoryMethod(stringstream &ss) const {
    Message::Type type;
    ss.read(reinterpret_cast<char*>(&type), sizeof(type));

    Message::Operation operation;
    ss.read(reinterpret_cast<char*>(&operation), sizeof(operation));

    Message *m = nullptr;

    switch(operation){
        case Message::Operation::HEARTBEAT: m = new MessageHeartbeat(type); break;
        case Message::Operation::SPLIT    : m = new MessageSplit(type); break;
        case Message::Operation::MERGE    : m = new MessageMerge(type); break;
        default: throw logic_error("Unknown operation: " + to_string(static_cast<uint8_t>(operation)));
    }

    m->deserializeContents(ss);
    return m;
}
