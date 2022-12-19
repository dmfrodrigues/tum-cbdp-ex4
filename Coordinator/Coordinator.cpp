#include "Coordinator.h"
#include "../CurlEasyPtr.h"
#include "../Worker/Worker.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/un.h>
#include <cassert>
#include <algorithm>

#include "../Message/MessageHeartbeat.h"
#include "../Message/MessageSplit.h"
#include "../Message/MessageMerge.h"

using namespace std;

Coordinator::Coordinator(const std::string &name, const int port) {
   socket.bind(name, port);
   #ifdef LOG
   cout << "[C] " << name << " " << port << endl;
   #endif

   // pollSockets.reserve(20);

   // Build first pollfd to the listening connection
   struct pollfd pfd = {};
   pfd.fd = socket.getSd();
   pfd.events = POLLIN;
   pollSockets.push_back(pfd);
}

void Coordinator::acceptConnection() {
   Socket worker = socket.accept();
   
   // Add entry to the workers map
   WorkerDetails wd = {};
   wd.socket = worker;
   wd.work = {list<string>(),};
   workers.insert({worker.getSd(), wd});

   #ifdef LOG
   cout << "[C] " << "Accepted worker with sd " << worker.getSd() << endl;
   #endif

   // Add worker pollfd
   struct pollfd pfd = {};
   pfd.fd = worker.getSd();
   pfd.events = POLLIN;
   pollSockets.push_back(pfd);

   sendWork(worker.getSd());
}

void Coordinator::sendWork(int sd) {
   if (!remainingPartitions.empty()) {
      string partitionURI = remainingPartitions.front();
      remainingPartitions.pop();

      MessageSplit m(Message::Type::REQUEST, partitionURI);
      WorkerDetails &wd = workers.at(sd);
      wd.socket.send(&m);

      #ifdef LOG
      cout << "[C] Dispatched partition '" << partitionURI << "' to worker " << sd << endl;
      #endif
   } else {
      // Find a partial result that already has all of the necessary subpartitions
   }
 }

bool Coordinator::processWorkerResult(int sd) {
   WorkerDetails &wd = workers.at(sd);

   Message *mptr = wd.socket.receive();
   if (mptr == nullptr) {
      #ifdef LOG
      cout << "[C] Worker " << sd << " has died" << endl;
      #endif
      return false;
   }

   switch(mptr->operation){
      case Message::Operation::SPLIT: {
         MessageSplit &m = *dynamic_cast<MessageSplit*>(mptr);
         for(size_t i = 0; i < m.subpartitionsURI.size(); ++i){
            doneSubpartitions[i].emplace_back(m.subpartitionsURI[i]);
         }
         break;
      }
      case Message::Operation::MERGE: {
         MessageMerge &m = *dynamic_cast<MessageMerge*>(mptr);
         processedPartialResults.insert(m.partialResultURI);
         break;
      }
      default:
         throw new logic_error("Strange operation " + to_string(static_cast<int>(mptr->operation)));
   }

   sendWork(sd);
   return true;
}

void Coordinator::loop() {

   int rc = poll(pollSockets.data(), pollSockets.size(), POLL_TIMEOUT);
   if (rc == 0) {
      cerr << "[C] Aborting due to lack of network activity" << endl;
      exit(1);
   }
   if (rc < 0) {
      perror("  poll() failed");
      return;
   }

   bool workersAwaitingConnection = false;
   list<int> deadWorkers = list<int>();
   for (const auto &element : pollSockets) {
      if (element.revents == 0) continue;

      if (element.revents & POLLIN) {
         if (element.fd == socket.getSd()) {
            // Worker attempting to connect
            workersAwaitingConnection = true;
         } else {
            // Worker socket input
            processWorkerResult(element.fd);
         }
      } else if (element.revents & (POLLHUP | POLLNVAL | POLLERR)) {
         // Closed socket
         if (element.fd == socket.getSd()) {
            // OH NO
            cerr << "Not this Socket!" << endl;
         } else {
            deadWorkers.push_back(element.fd);
         }
      } else {
         // Do Nothing
      }
   }

   if (workersAwaitingConnection) acceptConnection();
}

void Coordinator::cleanup() {
   // TODO:
}

vector<pair<int, string>> Coordinator::processFile(std::string listUrl) {
   //    1. Allow workers to connect
   //       socket(), bind(), listen(), accept(), see: https://beej.us/guide/bgnet/html/#system-calls-or-bust
   //    2. Distribute the following work among workers
   //       send() them some work
   //    3. Collect all results
   //       recv() the results
   // Hint: Think about how you track which worker got what work
   
   // Download the file list
   auto curl = CurlEasyPtr::easyInit();
   curl.setUrl(listUrl);
   stringstream ss = curl.performToStringStream();



   string nextPartition;
   do {
      getline(ss, nextPartition, '\n');
      if (nextPartition.empty()) continue;

      remainingPartitions.push(nextPartition);
   } while(!ss.eof());

   // size_t numberPartitions = remainingPartitions.size();

   do { // Until result
      loop();
   } while (true);

   // Cleanup
   cleanup();

   // #ifdef LOG
   // cout << "[C] Finished processing file, found " << totalResults << " matches" << endl;
   // #endif

   vector<pair<int, string>> results;
   return results;
}
