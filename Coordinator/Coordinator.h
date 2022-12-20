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
   const static int POLL_TIMEOUT = 10 * 60000; // 10 mins

   Socket socket;
   std::map<int, WorkerDetails> workers;
   std::vector<struct pollfd> pollSockets;

   std::set<int> nonBusyWorkers;

   size_t NUMBER_PARTITIONS;
   
   /// @brief Partitions that were not yet subpartitioned
   std::queue<std::string> remainingPartitions;

   /// @brief For each ID of a range, store subpartitions that are done
   std::map<size_t, std::vector<std::string>> doneSubpartitions;

   /// @brief Set of finalized partial results. When this structure is full,
   /// the coordinator can merge all partial results.
   std::set<std::string> processedPartialResults;

   // bool processWorkerResult(int sd);
   bool sendWork(int sd);
   bool processWorkerResult(int sd);
   void acceptConnection();
   std::vector<std::pair<int, std::string>> aggregatePartialResults();
   void loop();
   void cleanup();

   BlobClient *blobClient = nullptr;
public:
   Coordinator(const int p);
   std::vector<std::pair<int, std::string>> processFile(const std::string listUrl);
   ~Coordinator();
};

#endif