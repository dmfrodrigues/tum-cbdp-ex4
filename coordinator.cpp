#include "Coordinator/Coordinator.h"
#include <chrono>
#include <iostream>

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

/// Leader process that coordinates workers. Workers connect on the specified port
/// and the coordinator distributes the work of the CSV file list.
/// Example:
///    ./coordinator http://example.org/filelist.csv 4242

int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <URL to csv list> <listen port>" << std::endl;
      return 1;
   }

   #ifdef WAIT_FIFO
   mkfifo("fifo", 0666);
   std::ifstream fifo("fifo");
   char a[1];
   fifo.read(a, 1);
   #endif

   auto start = std::chrono::steady_clock::now();

   Coordinator coordinator = Coordinator(atoi(argv[2]));
   auto results = coordinator.processFile(argv[1]);
   int i = 1;
   for(const auto &p: results){
      std::cout << i++ << ". \t" << p.second << "\t" << p.first << std::endl;
   }

   auto end = std::chrono::steady_clock::now();
   std::cout << "[C] Finished job in: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
      << " ms" << std::endl;

   return 0;
}
