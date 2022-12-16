#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include <set>
#include <queue>

#include <sys/poll.h>


#include "../Socket/Socket.h"


struct workerDetails {
   std::list<std::string> work;
   Socket socket;
};
typedef struct workerDetails workerDetails;

class Coordinator {
private:
   const static int POLL_TIMEOUT = 10000;

   Socket socket;
   std::map<int, workerDetails> workers;
   std::vector<struct pollfd> pollSockets;
   
   std::queue<std::string> remainingChunks;

   size_t totalResults;
   int unfinishedChunks;

   bool processWorkerResult(int sd);
   void sendWork(int sd);
   void acceptConnection();
   void loop();
   void cleanup();
   void cleanupDeadWorker(int dw);


public:
   Coordinator(const std::string& name, const int p);
   size_t processFile(const std::string listUrl);
};

#endif