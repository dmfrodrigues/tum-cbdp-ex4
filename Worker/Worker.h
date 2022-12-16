#ifndef WORKER_H
#define WORKER_H

#include <sstream>

#include "../Socket/Socket.h"
#include "../CurlEasyPtr.h"

class Worker {
private:
   static const int NUMBER_RETRIES_CONNECT = 10;
   static const useconds_t SLEEP_MICROS = 50000;

   Socket socket;

   CurlEasyPtr curl;
public:
   Worker(const std::string& coordName, const int coordPort);
   void run();
   size_t processChunk(std::stringstream& chunkName);
};

#endif