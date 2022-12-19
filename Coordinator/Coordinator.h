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


struct WorkerDetails {
   std::list<std::string> work;
   Socket socket;
};

class Coordinator {
private:
   const static int POLL_TIMEOUT = 120000;

   Socket socket;
   std::map<int, WorkerDetails> workers;
   std::vector<struct pollfd> pollSockets;
   
   /// @brief Partitions that were not yet subpartitioned
   std::queue<std::string> remainingPartitions;

   /// @brief For each ID of a range, store subpartitions that are done
   std::map<int, std::list<std::string>> doneSubpartitions;

   /// @brief Set of finalized partial results. When this structure is full,
   /// the coordinator can merge all partial results.
   std::set<std::string> processedPartialResults;

   // bool processWorkerResult(int sd);
   void sendWork(int sd);
   void acceptConnection();
   void loop();
   void cleanup();


public:
   Coordinator(const std::string& name, const int p);
   std::vector<std::pair<int, std::string>> processFile(const std::string listUrl);
};

#endif