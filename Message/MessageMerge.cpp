#include "MessageMerge.h"

#include <iostream>

#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <unistd.h>

#include "../Socket/Socket.h"
#include "MessageSplit.h"

using namespace std;

MessageMerge::MessageMerge(Message::Type t) : Message(t, Message::Operation::MERGE) {}

MessageMerge::MessageMerge(Message::Type t, const std::vector<std::string>& subpartitionsURI, const std::string& partialResultURI) :
   Message(t, Message::Operation::MERGE),
   subpartitionsURI(subpartitionsURI),
   partialResultURI(partialResultURI)
{}

void MessageMerge::serializeContents(stringstream& ss) const {
   const size_t& partialResultURIsize = partialResultURI.size();
   ss.write(reinterpret_cast<const char*>(&partialResultURIsize), sizeof(partialResultURIsize));
   ss.write(partialResultURI.data(), partialResultURIsize);

   const size_t& numberSubpartitionsURI = subpartitionsURI.size();
   ss.write(reinterpret_cast<const char*>(&numberSubpartitionsURI), sizeof(numberSubpartitionsURI));

   cerr << "[C] Serializing Merge, got " << numberSubpartitionsURI << " subpartitions to merge" << endl;

   for (size_t i = 0; i < numberSubpartitionsURI; ++i) {
      const string& subpartitionURI = subpartitionsURI[i];
      const size_t& sizeSubpartitionURI = subpartitionURI.size();

      ss.write(reinterpret_cast<const char*>(&sizeSubpartitionURI), sizeof(sizeSubpartitionURI));
      ss.write(subpartitionURI.data(), sizeSubpartitionURI);

      cerr << "[C]    Serializing Merge - Serialized subpartition " << subpartitionURI << " (size " << sizeSubpartitionURI << ")" << endl;
   }
}

bool MessageMerge::deserializeContents(stringstream& ss) {
   char buf[Message::MAX_SIZE];

   size_t partialResultURIsize;
   ss.read(reinterpret_cast<char*>(&partialResultURIsize), sizeof(partialResultURIsize));
   ss.read(buf, partialResultURIsize);
   partialResultURI = string(buf, partialResultURIsize);

   size_t numberSubpartitionsURI;
   ss.read(reinterpret_cast<char*>(&numberSubpartitionsURI), sizeof(numberSubpartitionsURI));
   cerr << "[W] Deserializing Merge, got " << numberSubpartitionsURI << " subpartitions to merge" << endl;
   subpartitionsURI.clear();
   subpartitionsURI.reserve(numberSubpartitionsURI);
   for (size_t i = 0; i < numberSubpartitionsURI; ++i) {
      size_t sizeSubpartitionURI;
      ss.read(reinterpret_cast<char*>(&sizeSubpartitionURI), sizeof(sizeSubpartitionURI));
      ss.read(buf, sizeSubpartitionURI); cerr << ss.good() << ss.eof() << ss.fail() << ss.bad() << endl;
      subpartitionsURI.emplace_back(buf, sizeSubpartitionURI); cerr << ss.good() << ss.eof() << ss.fail() << ss.bad() << endl;
      cerr << "[W] Deserializing Merge, was asked to merge subpartition " << *subpartitionsURI.rbegin() << " (size " << sizeSubpartitionURI << ")" << endl;
   }

   return ss.eof();
}

void MessageMerge::process(Socket& socket, BlobClient &blobClient) const {
   vector<map<string, size_t>> countings(MessageSplit::NUMBER_SUBPARTITIONS);

   cerr << "[W] !Merging " << partialResultURI << endl;

   map<string, size_t> partialCounts;

   for (const string& elem : subpartitionsURI) {
      cerr << "[W] Merge - Retrieving " << elem << endl;
      istream *in_ptr = nullptr;
      while (!in_ptr) {
         try
         {
            in_ptr = blobClient.get(elem);
         }
         catch(const std::exception& e)
         {
            std::cerr << "Could not retrieve " << elem << " : " << e.what() << '\n';
         }
         sleep(1);
      }
      
      istream &in = *in_ptr;

      size_t count;
      string domain;

      while (!in.eof()) {
         in >> count;
         getline(in, domain);

         partialCounts[domain] += count;
      }

      delete in_ptr;
   }

   vector<pair<size_t, string>> outTemp;
   for (const auto &elem: partialCounts) {
      outTemp.push_back(make_pair(elem.second, elem.first));
   }
   sort(outTemp.rbegin(), outTemp.rend());

   
   cerr << "[W] Merge - Printing partial result " << partialResultURI << endl;
   {
      stringstream out;

      for (size_t i = 0; i < min(size_t(NUMBER_RESULTS), outTemp.size()); ++i) {
         const auto& elem = outTemp[i];
         out << elem.first << elem.second << "\n";
      }
      blobClient.put(partialResultURI, out);
   }

   cerr << "[W] Merge - Done printing partial result to " << partialResultURI << endl;
   
   MessageMerge response(Message::Type::RESPONSE);
   response.partialResultURI = partialResultURI;

   cerr << "[W] Merge - Done processing partial result " << partialResultURI << endl;

   socket.send(&response);
}
