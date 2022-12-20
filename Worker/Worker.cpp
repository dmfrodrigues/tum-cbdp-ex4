#include "Worker.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <string_view>

#include <cassert>

#include "../Blob/AzureBlobClient.h"
#include "../Blob/FilesystemBlobClient.h"

using namespace std;
using namespace std::literals;

#ifndef AZURE_BLOB_STORAGE
   #define AZURE_BLOB_STORAGE 0
#endif

Worker::Worker(const std::string &coordName, const int coordPort) :
   curl(CurlEasyPtr::easyInit())
{
   socket.connect(coordName, coordPort);
   // MessageHeartbeat m(Message::Type::REQUEST);
   // socket.send(&m);

   if(AZURE_BLOB_STORAGE){
      const string AZURE_ACCOUNT_NAME = "TODO";
      const string AZURE_ACCESS_TOKEN = "TODO";
      const string containerName = "my-container";
      blobClient = new AzureBlobClient(
         AZURE_ACCOUNT_NAME,
         AZURE_ACCESS_TOKEN,
         containerName
      );
   } else {
      blobClient = new FilesystemBlobClient;
   }
}

void Worker::run() {
   while(true) {
      Message *m = socket.receive();
      if (m == nullptr) return;

      m->process(socket);

      delete m;

      sched_yield();
   }
}

Worker::~Worker(){
   delete blobClient;
}
