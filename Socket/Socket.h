#pragma once

#include "../Message/Message.h"
#include "../Message/MessageFactory.h"

#include <string>
#include <netdb.h>

class Socket {
private:
    static const int BACKLOG = 10;
    static const int NUMBER_RETRIES_CONNECT = 20000;
    static const useconds_t SLEEP_MICROS = 200000;

    static MessageFactory messageFactory;

    void init(const char *name, int port, bool is_listening);

    addrinfo *req = nullptr;
    int sd;

public:
    Socket();
    Socket(int sd);

    int getSd() const;

    void bind(int port);
    void connect(const std::string &name, int port);

    Socket accept();
    void close();

    void send(const Message *m);
    Message* receive();

    bool operator<(const Socket &s) const;

    ~Socket();
};
