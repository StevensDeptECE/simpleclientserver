#include "IPV4SocketPlat.hh"

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>

#include "Errcode.hh"
#include "Ex.hh"
#include "Request.hh"
/*
  All encapsulation for different operating systems networking code is done here
*/
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // Need to be at least Vista
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdlib>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")  // Ignored by MinGW

WSADATA Socket::wsaData;
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#else  // linux
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET close(s)
#define GETSOCKETERRNO() (errno)
#endif

using namespace std;

#ifdef __linux__
#define testResult(result, err) (result < 0 ? throw Ex(err) : result)
#elif _WIN32
#define testResult(result, err) (result != 0 ? throw Ex(err) : result)
#endif

// Initializes Winsock
void Socket::classInit() {
  if constexpr (_WIN32) {
    testResult(WSAStartup(MAKEWORD(2, 2), &wsaData), Errcode::SOCKET);
  }
}

// Takes care of allocations made by Winsock
void Socket::classCleanup() {
  if constexpr (_WIN32) {
    WSACleanup();
  }
}

struct client_info {
  socklen_t addr_len = sizeof(addr);
  struct sockaddr_storage addr;
  IPV4Socket::socket_t socket;
  char request[MAX_REQUEST_SIZE + 1];
  int received;
};

IPV4Socket::~IPV4Socket() { CLOSESOCKET(sckt); }
#ifdef __linux__
// Constructor for client
IPV4Socket::IPV4Socket(const char *addr, uint16_t port) : Socket(addr, port) {
  struct hostent *server;
  testResult(sckt = socket(AF_INET, SOCK_STREAM, 0), Errcode::SOCKET);
  server = gethostbyname(address);

  if (server == nullptr) {
    throw Ex2(Errcode::SERVER_INVALID);
  }

  sockaddr_in *sockAddr = (sockaddr_in *)sockaddress;
  sockAddr->sin_family = AF_INET;
  //    bcopy((char *)server->h_addr, (char *)&sockaddress.sin_addr.s_addr,
  //    server->h_length);
  sockAddr->sin_addr.s_addr = inet_addr(address);
  sockAddr->sin_port = htons(port);

  if (connect(sckt, (struct sockaddr *)sockaddress, sizeof(sockaddr_in)) < 0) {
    throw Ex2(Errcode::CONNECTION_FAILURE);
  }
}

// Server side
void IPV4Socket::wait() {
  struct sockaddr_in client_addrconfig;
  socklen_t client_length = sizeof(client_addrconfig);
  while (true) {
    cout << "WAITING CONNECTION." << endl;
    int returnsckt =
        accept(sckt, (struct sockaddr *)&client_addrconfig, &client_length);
    //		int senderSock = accept(listenSock, (struct sockaddr *)
    //&sockaddress, &senderNameLen);
    if (returnsckt >= 0) {
      cout << "CONNECT SUCCESSFULLY"
           << "\n";
      req->handleServer(returnsckt);
      close(returnsckt);
      // if you are not familiar with socket, try below code
      //			read(senderSock,testin, sizeof(testin)-1);
      //			cout<<testin<<endl;
      //			strcpy(testout,"hello,this is server");
      //			write(senderSock,testout,sizeof(testout));
      //			cout<<listenSock;
    } else {
      throw Ex2(Errcode::CONNECTION_FAILURE);
    }
  }
}
#endif

// Constructor for server
IPV4Socket::IPV4Socket(uint16_t port, const char *host) : Socket(port) {
  struct addrinfo *bind_addr = NULL;
  struct addrinfo hints;

  char err_buf[256];
  char port_string[8];
  itoa(port, port_string, 10);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the local address and port to be used by the server
  testResult(getaddrinfo(host, port_string, &hints, &bind_addr),
             Errcode::SOCKET);
  this->result = bind_addr;

  sckt = INVALID_SOCKET;

  // Create a SOCKET for the server to listen for client connections
  sckt = socket(bind_addr->ai_family, bind_addr->ai_socktype,
                bind_addr->ai_protocol);

  if (!ISVALIDSOCKET(sckt)) {
    if constexpr (_WIN32) {
      freeaddrinfo(result);
    }
    sprintf(err_buf, "socket() failed. %d", GETSOCKETERRNO());
    throw Ex2(Errcode::SOCKET, err_buf);
  }

  // Setup the TCP listening socket
  if (bind(sckt, result->ai_addr, (int)result->ai_addrlen)) {
    if constexpr (_WIN32) {
      freeaddrinfo(result);
    }
    sprintf(err_buf, "bind() failed. %d", GETSOCKETERRNO());
    throw Ex2(Errcode::SOCKET_BIND, err_buf);
  }

  freeaddrinfo(result);

  if (listen(sckt, SOMAXCONN) < 0) {
    sprintf(err_buf, "listen() failed. %d", GETSOCKETERRNO());
    throw Ex2(Errcode::LISTEN, err_buf);
  }

  // TODO: Check if send/receive needs to be implemented
  // TODO: Check how disconnect/shutdown should be handled
}

#ifdef _WIN32
// Constructor for csp/http client
IPV4Socket::IPV4Socket(const char *addr, uint16_t port) : Socket(addr, port) {
  sckt = INVALID_SOCKET;
  struct addrinfo *result = NULL;
  struct addrinfo *ptr = NULL;
  struct addrinfo hints;

  char port_string[8];
  itoa(port, port_string, 10);
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // TODO: Find out if port should stay as uint16_t or convert to const char *
  // Resolve the address and port to be used by the client
  testResult(getaddrinfo(addr, port_string, &hints, &result), Errcode::SOCKET);

  for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    // Create a SOCKET for connecting to server
    sckt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    testResult(sckt == INVALID_SOCKET, Errcode::SOCKET);

    // TODO: Check if connect should be passed to test_result and
    // closesocket
    //       reimplemented in destructor
    if (connect(sckt, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
      closesocket(sckt);
      sckt = INVALID_SOCKET;
      continue;
    }
    break;
  }

  testResult(sckt == INVALID_SOCKET, Errcode::SOCKET);

  // TODO: Get some more info on this
  // Probably should remove and leave to the destructor???
  freeaddrinfo(result);
}
#endif

// Server side
void IPV4Socket::wait() {
  char err_buf[100];

  while (true) {
    cout << "WAITING CONNECTION." << endl;
    fd_set reads;
    reads = waitOnClients(sckt);

    if (FD_ISSET(sckt, &reads)) {
      auto client = getClient(-1);
      client->socket =
          accept(sckt, (struct sockaddr *)&(client->addr), &client->addr_len);

      if (!ISVALIDSOCKET(client->socket)) {
        sprintf(err_buf, "accept failed. (%d)", GETSOCKETERRNO());
        continue;
      }

      cout << "New connection from " << getClientAddress(client);
      handleServer(client);
    }

    for (auto &client : clients) {
      if (FD_ISSET(client.socket, &reads)) {
        handleServer(&client);
      }
    }
  }
}

void IPV4Socket::handleServer(client_info *info) {
  auto status = req->handleServer(info->socket);
  if (req->getShouldClose()) {
    printf("Request for client %s has been closed.\n",
           getClientAddress(info).c_str());
    dropClient(info);
  }
  if (!status.isWriting && status.bytesTransferred < 0) {
    printf("Unexpected disconnect from %s.\n", getClientAddress(info).c_str());
    dropClient(info);
  }
}

fd_set IPV4Socket::waitOnClients(socket_t server) {
  fd_set reads;
  FD_ZERO(&reads);

  FD_SET(server, &reads);
  socket_t max_socket = server;

  for (const auto &client : clients) {
    FD_SET(client.socket, &reads);
    if (client.socket > max_socket) {
      max_socket = client.socket;
    }
  }

  if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
    char err_buf[100];
    sprintf(err_buf, "select() failed. (%d)\n", GETSOCKETERRNO());
    throw Ex2(Errcode::SERVER_INVALID, err_buf);
  }

  return reads;
}

// this code is outside of windows ifdef???
void IPV4Socket::send(uint32_t reqn) {
  size_t bytesWritten;
  if ((bytesWritten = ::send(sckt, (const char *)&reqn, sizeof(uint32_t), 0)) <
      0) {
    perror("Write Error: ");
  }
  cout << "Bytes Written: " << bytesWritten << endl;
}

void IPV4Socket::sendAndAwait(uint32_t reqn, Request &r) {
  send(reqn);
  r.handle(sckt);
}

client_info *IPV4Socket::getClient(socket_t sock) {
  auto isSock = [sock](client_info &c) { return c.socket == sock; };
  auto fr = ranges::find_if(clients, isSock);
  if (fr != clients.end()) {
    return &*fr;
  }

  clients.emplace_back();
  return &clients.back();
}

void IPV4Socket::dropClient(struct client_info *info) {
  CLOSESOCKET(info->socket);

  auto isSock = [info](client_info &c) { return c.socket == info->socket; };

  auto numErased = std::erase_if(clients, isSock);
  if (numErased == 0) {
    printf("Client with address %s not found\n",
           getClientAddress(info).c_str());
  }
}

string IPV4Socket::getClientAddress(struct client_info *info) {
  std::string address;
  address.reserve(100);
  char *address_buffer = address.data();
  getnameinfo((struct sockaddr *)&info->addr, info->addr_len, address_buffer,
              sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

  return address;
}
