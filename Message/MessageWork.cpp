#include "MessageWork.h"

#include <iostream>

using namespace std;

MessageWork::MessageWork(Message::Type t) :
    Message(t, Message::Operation::WORK)
{}

void MessageWork::serializeContents(stringstream &ss) const {
    const size_t &numberChunks = chunkURLs.size();
    ss.write(reinterpret_cast<const char*>(&numberChunks), sizeof(numberChunks));
    for(size_t i = 0; i < numberChunks; ++i){
        const string &chunkURL = chunkURLs[i];
        const size_t &sizeURL = chunkURL.size();

        ss.write(reinterpret_cast<const char*>(&sizeURL), sizeof(sizeURL));
        ss.write(chunkURL.data(), sizeURL);
    }
    ss.write(reinterpret_cast<const char*>(&result), sizeof(result));
}

bool MessageWork::deserializeContents(stringstream &ss) {
    size_t numberChunks;

    ss.read(reinterpret_cast<char*>(&numberChunks), sizeof(numberChunks));
    chunkURLs.resize(numberChunks);

    for(size_t i = 0; i < numberChunks; ++i) {
        size_t sizeURL;
        ss.read(reinterpret_cast<char*>(&sizeURL), sizeof(sizeURL));
        char buf[Message::MAX_SIZE];
        ss.read(buf, sizeURL);
        buf[sizeURL] = '\0';
        chunkURLs[i] = string(buf);
    }
    ss.read(reinterpret_cast<char*>(&result), sizeof(result));

    return ss.eof();
}
