#include "FilesystemBlobClient.h"

#include <fstream>

using namespace std;

void FilesystemBlobClient::put(const std::string &name, std::istream &stream){
    ofstream out(name);
    out << stream.rdbuf();
}

std::istream* FilesystemBlobClient::get(const std::string &name){
    ifstream *in = new ifstream(name);
    return in;
}
