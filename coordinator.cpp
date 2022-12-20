#include "Coordinator/Coordinator.h"
#include "CurlEasyPtr.h"
#include <iostream>

/// Leader process that coordinates workers. Workers connect on the specified port
/// and the coordinator distributes the work of the CSV file list.
/// Example:
///    ./coordinator http://example.org/filelist.csv 4242
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <URL to csv list> <listen port>" << std::endl;
      return 1;
   }

   CurlGlobalSetup curlSetup;

   Coordinator coordinator = Coordinator("127.0.0.1", atoi(argv[2]));
   auto results = coordinator.processFile(argv[1]);
   int i = 1;
   for(const auto &p: results){
      std::cout << i++ << ". \t" << p.second << "\t" << p.first << std::endl;
   }

   return 0;
}
