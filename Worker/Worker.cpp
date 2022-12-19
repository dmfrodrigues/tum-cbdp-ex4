#include "Worker.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <string_view>

#include <cassert>

using namespace std;
using namespace std::literals;

Worker::Worker(const std::string &coordName, const int coordPort) :
   curl(CurlEasyPtr::easyInit())
{
   socket.connect(coordName, coordPort);
   // MessageHeartbeat m(Message::Type::REQUEST);
   // socket.send(&m);
}

void Worker::run() {
   while(true) {
      Message *m = socket.receive();
      if (m == nullptr) return;

      m->process(socket);

      delete m;

      sched_yield();
   }
}
