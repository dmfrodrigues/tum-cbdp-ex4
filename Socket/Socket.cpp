#include "Socket.h"

#include <iostream>
#include <netdb.h>
#include <sys/un.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

MessageFactory Socket::messageFactory = MessageFactory();

Socket::Socket() {}

Socket::Socket(int fd) : sd(fd) { }

int Socket::getSd() const {
    return sd;
}

void Socket::init(const char *name, int port, bool is_listening) {
   addrinfo hints{};
   memset(&hints, 0, sizeof(addrinfo));

   hints.ai_addrlen = sizeof(struct sockaddr_in);


   if (is_listening) {
      hints.ai_flags = AI_PASSIVE;
      hints.ai_family = AF_INET6;
      hints.ai_socktype = SOCK_STREAM;
   } else {
      hints.ai_flags = IPPROTO_TCP;
      hints.ai_socktype = SOCK_STREAM;
   }

   if (getaddrinfo(name, to_string(port).c_str(), &hints, &req) != 0) {
      throw runtime_error("getaddrinfo() failed");
   }

   sd = socket(req->ai_family, req->ai_socktype, req->ai_protocol);
   if (sd == -1) {
      throw runtime_error("socket() failed");
   }

   // allow kernel to rebind address even when in TIME_WAIT state
   int yes = 1;
   if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
      throw runtime_error("setsockopt() failed");
   }
}

void Socket::bind(int port) {
   init(NULL, port, true);

   if (::bind(sd, req->ai_addr, req->ai_addrlen) == -1) {
      throw runtime_error("perform_bind() failed");
   }

   if (listen(sd, BACKLOG) == -1) {
      throw runtime_error("perform_listen() failed");
   }
}

void Socket::close() {
   ::close(sd);
}

Socket Socket::accept() {
   int new_fd = ::accept(sd, nullptr, nullptr);
   return Socket(new_fd);
}

void Socket::connect(const string &name, int port) {
   init(name.c_str(), port, false);

   bool connected = false;
   int i = 0;
   while (!connected && i < NUMBER_RETRIES_CONNECT) {
      if (::connect(sd, req->ai_addr, req->ai_addrlen) == -1) {
         cerr << "[W] Failed to connect to " << name << " " << port << endl;
         usleep(SLEEP_MICROS);

         ++i;
      } else {
         connected = true;
      }
   }

   if (!connected)
      throw runtime_error("connect() failed");
}

void Socket::send(const Message *m) {
   string msg = m->serialize();
   const size_t &sz = msg.size();
   write(sd, &sz, sizeof(sz));
   write(sd, msg.data(), sz);
}

Message* Socket::receive() {
   size_t sz;
   size_t n = read(sd, &sz, sizeof(sz));
   if (n == 0) return nullptr;
   
   char buf[Message::MAX_SIZE];
   memset(buf, 0, Message::MAX_SIZE);
   
   char *current_buf = buf;
   ssize_t left_to_read = sz;
   while(left_to_read > 0){
      n = read(sd, current_buf, left_to_read);
      if (n == 0) return nullptr;
      current_buf += n;
      left_to_read -= n;
   }

   stringstream ss;
   ss.write(buf, sz);

   return messageFactory.factoryMethod(ss);
}

bool Socket::operator<(const Socket&s) const {
    return sd < s.sd;
};

Socket::~Socket(){
   freeaddrinfo(req);
}

