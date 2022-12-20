#pragma once

#include <iostream>

class BlobClient {
public:
    virtual void put(const std::string &name, std::istream &stream) = 0;
    virtual std::istream* get(const std::string &name) = 0;
    virtual ~BlobClient(){};
};
