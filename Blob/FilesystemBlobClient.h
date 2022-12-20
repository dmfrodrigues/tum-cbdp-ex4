#pragma once

#include "BlobClient.h"

class FilesystemBlobClient: public BlobClient {
    virtual void put(const std::string &name, std::istream &stream);
    virtual std::istream* get(const std::string &name);
    ~FilesystemBlobClient();
};
