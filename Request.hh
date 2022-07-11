#pragma once

#include "SCSDefs.hh"

struct transportStatus {
  int bytesTransferred;
  bool isWriting;
};

class Request {
 protected:
  bool shouldClose;

 public:
  Request() : shouldClose(false) {}
  virtual transportStatus handle(socket_t returnsckt, bool shouldRespond);
  const bool getShouldClose() const { return shouldClose; }
};
