#pragma once

struct transportStatus {
  int bytesTransferred;
  bool isWriting;
};

class Request {
 protected:
  bool shouldClose;

 public:
  Request() : shouldClose(false) {}
  virtual transportStatus handleServer(int returnsckt);
  virtual void handle(int returnsckt);
  const bool getShouldClose() const { return shouldClose; }
};
