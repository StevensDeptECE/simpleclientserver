#pragma once
/**
         Represent a client/server socket pair
         and call a handler.
         @author: Dov Kruger
 */

#include <cstdint>
#include "SCSDefs.hh"

class Request; // forward reference, all code is included in .cc
class Socket {
 protected:
  const char* address;
  uint16_t port;
  struct addrinfo* result;
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
