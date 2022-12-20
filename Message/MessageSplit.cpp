#include "MessageSplit.h"

#include <iostream>

#include <vector>
#include <map>

#include "../Socket/Socket.h"

using namespace std;

MessageSplit::MessageSplit(Message::Type t) :
    Message(t, Message::Operation::SPLIT)
{}

MessageSplit::MessageSplit(Message::Type t, const string &partitionURI) :
    Message(t, Message::Operation::SPLIT),
    partitionURI(partitionURI)
{}

void MessageSplit::serializeContents(stringstream &ss) const {
    const size_t &partitionURIsize = partitionURI.size();
    ss.write(reinterpret_cast<const char*>(&partitionURIsize), sizeof(partitionURIsize));
    ss.write(partitionURI.data(), partitionURIsize);

    const size_t &numberSubpartitionsURI = subpartitionsURI.size();
    ss.write(reinterpret_cast<const char*>(&numberSubpartitionsURI), sizeof(numberSubpartitionsURI));

    for(size_t i = 0; i < numberSubpartitionsURI; ++i){
        const string &subpartitionURI = subpartitionsURI[i];
        const size_t &sizeSubpartitionURI = subpartitionURI.size();

        ss.write(reinterpret_cast<const char*>(&sizeSubpartitionURI), sizeof(sizeSubpartitionURI));
        ss.write(subpartitionURI.data(), sizeSubpartitionURI);
    }
}

bool MessageSplit::deserializeContents(stringstream &ss) {
    char buf[Message::MAX_SIZE];

    size_t partitionURIsize;
    ss.read(reinterpret_cast<char*>(&partitionURIsize), sizeof(partitionURIsize));
    ss.read(buf, partitionURIsize);
    partitionURI = string(buf, partitionURIsize);

    size_t numberSubpartitionsURI;
    ss.read(reinterpret_cast<char*>(&numberSubpartitionsURI), sizeof(numberSubpartitionsURI));
    subpartitionsURI.clear();
    subpartitionsURI.reserve(numberSubpartitionsURI);
    for(size_t i = 0; i < numberSubpartitionsURI; ++i) {
        size_t sizeSubpartitionURI;
        ss.read(reinterpret_cast<char*>(&sizeSubpartitionURI), sizeof(sizeSubpartitionURI));
        ss.read(buf, sizeSubpartitionURI);
        subpartitionsURI.emplace_back(buf, sizeSubpartitionURI);
    }

    return ss.eof();
}

string MessageSplit::extractDomain(const string &s){
   size_t idx1 = s.find("://");
   size_t idx2 = s.find("/", idx1+3);
   if(idx1 == string::npos || idx2 == string::npos)
      return s;
   else return s.substr(idx1+3, idx2 - (idx1 + 3));
}

void MessageSplit::process(Socket &socket, BlobClient &blobClient) const {
   vector<map<string, size_t>> countings(NUMBER_SUBPARTITIONS);

   cerr << "[W] Processing partition " << partitionURI << endl;

   istream *in = blobClient.get(partitionURI);
   string url;
   hash<string> h;
   while(getline(*in, url)){
      url = url.substr(url.find_first_of("\t")+1); // Quick and dirty fix

      string domain = extractDomain(url);
      countings[h(domain) % NUMBER_SUBPARTITIONS][domain]++;
   }

   MessageSplit response(Message::Type::RESPONSE);
   response.partitionURI = partitionURI;

   for (int i = 0; i < NUMBER_SUBPARTITIONS; ++i) {
      string outFilename = partitionURI.substr(partitionURI.find_last_of("/")+1) + "." + to_string(i);
      cerr << "[W]     Printing subpartition " << outFilename << endl;
      stringstream out;
      for (const auto &p: countings.at(i)) {
         out << p.second << " " << p.first << "\n";
      }
      blobClient.put(outFilename, out);
      cerr << "[W]     Done printing subpartition to " << outFilename << endl;
      response.subpartitionsURI.push_back(outFilename);
   }

   delete in;
   cerr << "[W] Done processing partition " << partitionURI << endl;

   socket.send(&response);
}
