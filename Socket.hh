#pragma once
/**
         Represent a client/server socket pair
         and call a handler.
         @author: Dov Kruger
 */

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <cstdint>
using namespace std;

class Request; // forward reference, all code is included in .cc
class Socket {
 protected:
  const char* address;
  uint16_t port;
  struct addrinfo* result;
  #ifdef __linux__
       char sockaddress[16]; // placeholder big enough to hold sockaddr_in structure
  #elif _WIN32
      static WSADATA wsaData;
  #endif
  Request* req;    // to be called when a request is INCOMING (req->handle() )

 public:
  Socket(const char* addr, uint16_t port);

  // TODO: simplify
  // below two constructors could be merged depends on the location of variable
  // "req" Constructor for CSP server
  Socket(uint16_t port, Request* req);

  Socket(uint16_t port);  // Constructor for server (addres not specified)

  static void classCleanup();
  static void classInit();
  void attach(Request* r) { req = r; }

  virtual void wait() = 0;
};
