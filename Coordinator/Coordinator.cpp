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
   workerDetails wd = {};
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
   if (remainingChunks.empty()) return;
   
   string nextChunk = remainingChunks.front();
   remainingChunks.pop();   
   
   unfinishedChunks += 1;

   workerDetails &wd = workers.at(sd);
   wd.work.push_back(nextChunk);

   MessageWork m(Message::Type::REQUEST);
   m.chunkURLs = vector<string>({nextChunk});
   wd.socket.send(&m);

   #ifdef LOG
   cout << "[C] Dispatched chunk '" << nextChunk << "' to worker " << sd << endl;
   #endif
}

bool Coordinator::processWorkerResult(int sd) {
   workerDetails &wd = workers.at(sd);

   Message *m = wd.socket.receive();
   if (m == nullptr) {
      #ifdef LOG
      cout << "[C] Worker " << sd << " has died" << endl;
      #endif
      return false;
   }

   MessageWork *mw = dynamic_cast<MessageWork*>(m); 

   auto it = find(wd.work.begin(), wd.work.end(), mw->chunkURLs.at(0));
   if (it == wd.work.end()) {
      cerr << "[C] Worker " << sd << " returned unexpected chunk '" << mw->chunkURLs.at(0) << "'" << endl;
      delete mw;
      return false;
   }

   wd.work.erase(it);
   totalResults += mw->result;
   unfinishedChunks -= 1;

   #ifdef LOG
   cout << "[C] Worker " << sd << " processed chunk '" << mw->chunkURLs[0] << "' with result " << mw->result << endl;
   #endif

   delete mw;

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
            if (!processWorkerResult(element.fd)) {
               deadWorkers.push_back(element.fd);
            }
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
   for (int dw : deadWorkers) cleanupDeadWorker(dw);   
}

void Coordinator::cleanupDeadWorker(int worker) {

   #ifdef LOG
   cout << "[C] Cleaning dead worker " << worker << endl;
   #endif

   // Remove from polled sockets
   for (auto it = pollSockets.begin(); it != pollSockets.end(); ++it) {
      if (it->fd == worker) {
         pollSockets.erase(it);
         break;
      }
   }

   workerDetails wd = workers.at(worker);
   wd.socket.close();

   // Redistribute pending work
   for (const auto &elem : wd.work) {
      remainingChunks.push(elem);
      unfinishedChunks -= 1;
   }

   workers.erase(worker);
}

void Coordinator::cleanup() {
   // TODO:
}

size_t Coordinator::processFile(std::string listUrl) {
   //    1. Allow workers to connect
   //       socket(), bind(), listen(), accept(), see: https://beej.us/guide/bgnet/html/#system-calls-or-bust
   //    2. Distribute the following work among workers
   //       send() them some work
   //    3. Collect all results
   //       recv() the results
   // Hint: Think about how you track which worker got what work

   totalResults = 0;
   unfinishedChunks = 0;
   
   // Download the file list
   auto curl = CurlEasyPtr::easyInit();
   curl.setUrl(listUrl);
   stringstream chunks = curl.performToStringStream();


   string nextChunk;
   do {
      getline(chunks, nextChunk, '\n');
      if (nextChunk.empty()) continue;

      remainingChunks.push(nextChunk);
   } while(!chunks.eof());


   do { // Until result
      loop();
   } while (unfinishedChunks > 0 || !remainingChunks.empty());

   // Cleanup
   cleanup();

   #ifdef LOG
   cout << "[C] Finished processing file, found " << totalResults << " matches" << endl;
   #endif

   return totalResults;
}
