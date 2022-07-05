#include "Request.hh"

#include <errno.h>
#include <unistd.h>

#include <iostream>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdlib>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif
using namespace std;

/*
    receive request and send back socket to talk on?
 */
void Request::handleServer(int returnsckt) {
  cout << "Returning message on socket: " << returnsckt << '\n';
  char buf[256];

  // first read incoming message from client
  int bytesRead = recv(returnsckt, buf, 256, 0);
  if (bytesRead < 1) perror("Read error: ");
  cout << "bytes read: " << bytesRead << ' ';
  for (int i = 0; i < bytesRead; i++)  // dump out buffer as numbers
    cout << int(buf[i]) << ' ';
  cout << '\n';

  // now write back to client
  const char answer[] = "testing";

  size_t bytesWritten;
  if((bytesWritten = ::send(returnsckt, answer, sizeof(answer), 0)) < 0){
    perror("Write Error: ");
  }
  cout << "Bytes Written: " << bytesWritten << endl;
}

void Request::handle(int returnsckt) {
  cout << "Received socket: " << returnsckt << '\n';
  char buf[256];
  int bytesRead = recv(returnsckt, buf, 256, 0);
  if (bytesRead < 1) perror("Read error: ");
  cout << "bytes read: " << bytesRead << ' ';
  for (int i = 0; i < bytesRead; i++)  // dump out buffer as numbers
    cout << int(buf[i]) << ' ';
  cout << '\n';
  cout << buf << '\n';
}
