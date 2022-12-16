#include "MessageHeartbeat.h"

using namespace std;

MessageHeartbeat::MessageHeartbeat(Message::Type t) :
    Message(t, Message::Operation::HEARTBEAT)
{}

void MessageHeartbeat::serializeContents(stringstream &) const {
}

bool MessageHeartbeat::deserializeContents(stringstream &ss){
    return ss.eof();
}
