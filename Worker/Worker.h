#ifndef WORKER_H
#define WORKER_H

#include <sstream>

#include "../Socket/Socket.h"
#include "../CurlEasyPtr.h"

#include "../Blob/BlobClient.h"

class Worker {
private:
   static const int NUMBER_RETRIES_CONNECT = 10;
   static const useconds_t SLEEP_MICROS = 50000;

   Socket socket;

   CurlEasyPtr curl;

   BlobClient *blobClient = nullptr;   
public:
   Worker(const std::string& coordName, const int coordPort);
   void run();

   ~Worker();
};

#endif