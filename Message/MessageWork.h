#include "Message.h"

#include <vector>

class MessageWork : public Message {
public:
    std::vector<std::string> chunkURLs;
    size_t result = 0;
private:
    virtual void serializeContents(std::stringstream &ss) const;
    virtual bool deserializeContents(std::stringstream &ss);
public:
    MessageWork(Message::Type t);
};
