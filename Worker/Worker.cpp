#include "Worker.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <string_view>

#include <cassert>

#include "../Blob/AzureBlobClient.h"

using namespace std;
using namespace std::literals;

Worker::Worker(const std::string &coordName, const int coordPort)
{
   socket.connect(coordName, coordPort);
   // MessageHeartbeat m(Message::Type::REQUEST);
   // socket.send(&m);

   const string AZURE_ACCOUNT_NAME = "cbdpg43";
   const string AZURE_ACCESS_TOKEN = "HCGMVgxsvZe6BqYW+nEEFiu+9k/Jxe+hf0GkZbNr96jcl+EtzPwgm5RqdcocAr6kkasnWM5yffnx+AStY7u0+Q==";
   const string containerName = "mycontainer1";
   blobClient = new AzureBlobClient(
      AZURE_ACCOUNT_NAME,
      AZURE_ACCESS_TOKEN,
      containerName,
      false
   );
}

void Worker::run() {
   while(true) {
      Message *m = socket.receive();
      if (m == nullptr) return;

      MessageWork *mw = dynamic_cast<MessageWork*>(m); 

      const string chunkURL = mw->chunkURLs.at(0);
      delete mw;

      #ifdef LOG
      cout << "[W] Received chunk '" << chunkURL << "'" << endl;
      #endif

      istream *ss_ptr = blobClient->get(chunkURL);
      istream &ss = *ss_ptr;

      size_t result = processChunk(ss);

      delete ss_ptr;

      MessageWork response(Message::Type::RESPONSE);
      response.result = result;
      response.chunkURLs = vector<string>({chunkURL});
      socket.send(&response);

      #ifdef LOG
      cout << "[W] Sent response of chunk '" << chunkURL << "'" << endl;
      #endif

      sched_yield();
   }
}

size_t Worker::processChunk(std::istream &chunkName) {
   size_t result = 0;
   for (std::string row; std::getline(chunkName, row, '\n');) {
      std::stringstream rowStream = std::stringstream(std::move(row));

      // Check the URL in the second column
      unsigned columnIndex = 0;
      for (std::string column; std::getline(rowStream, column, '\t'); ++columnIndex) {
         // column 0 is id, 1 is URL
         if (columnIndex == 1) {
            // Check if URL is "google.ru"
            size_t pos = column.find("://"sv);
            if (pos != std::string::npos) {
               std::string_view afterProtocol = std::string_view(column).substr(pos + 3);
               if (afterProtocol.starts_with("google.ru/"))
                  ++result;
            }
            break;
         }
      }
   }

   return result;
}

Worker::~Worker(){
   delete blobClient;
}
