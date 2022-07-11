#include "Request.hh"

#include <errno.h>
#include <unistd.h>

#include <iostream>

#include "Socket.hh"
#include "SocketIO.hh"
using namespace std;

/*
    receive request and send back socket to talk on?
 */
// BUG: This should absolutely be socket_t not int
transportStatus Request::handle(socket_t returnsckt, bool shouldRespond) {
  cout << "Returning message on socket: " << returnsckt << '\n';
  char buf[256];
  char err_buf[100];

  // first read incoming message from client
  int bytesRead = recv(returnsckt, buf, 256, 0);
  if (bytesRead < 0) {
    fprintf(stderr, "recv() failed.");
    return {bytesRead, false};
  }
  cout << "bytes read: " << bytesRead << ' ';
  for (int i = 0; i < bytesRead; i++)  // dump out buffer as numbers
    cout << int(buf[i]) << ' ';
  cout << '\n';

  if (!shouldRespond) {
    return {bytesRead, false};
  }

  // now write back to client
  const char answer[] = "testing";

  int bytesWritten;
  if ((bytesWritten = ::send(returnsckt, answer, sizeof(answer), 0)) < 0) {
    perror("Write Error: ");
  }
  cout << "Bytes Written: " << bytesWritten << endl;
  this->shouldClose = true;
  return {bytesWritten, true};
}
