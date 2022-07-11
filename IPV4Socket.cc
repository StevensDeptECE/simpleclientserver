#include "IPV4Socket.hh"

#include <algorithm>

#include "Errcode.hh"
#include "Ex.hh"
#include "Request.hh"
#include "SocketIO.hh"
/*
  All encapsulation for different operating systems networking code is done here
*/
#ifdef _WIN32
static WSADATA wsaData;
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
  socket_t socket;
  char request[MAX_REQUEST_SIZE + 1];
  int received;
};
fd_set waitOnClients(IPV4Socket *sckt, socket_t server);

IPV4Socket::~IPV4Socket() { CLOSESOCKET(sckt); }

// Constructor for server
IPV4Socket::IPV4Socket(uint16_t port, const char *host) : Socket(port) {
  auto bind_addr = this->result;
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

// Constructor for csp/http client
IPV4Socket::IPV4Socket(const char *addr, uint16_t port) : Socket(addr, port) {
  auto server_addr = this->result;
  struct addrinfo hints;

  char port_string[8];
  itoa(port, port_string, 10);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // TODO: Find out if port should stay as uint16_t or convert to const char *
  // Resolve the address and port to be used by the client
  testResult(getaddrinfo(addr, port_string, &hints, &server_addr),
             Errcode::SOCKET);

  char address_buffer[100];
  char service_buffer[100];

  getnameinfo(server_addr->ai_addr, server_addr->ai_addrlen, address_buffer,
              sizeof(address_buffer), service_buffer, sizeof(service_buffer),
              NI_NUMERICHOST);
  sckt = socket(server_addr->ai_family, server_addr->ai_socktype,
                server_addr->ai_protocol);
  testResult(!ISVALIDSOCKET(sckt), Errcode::SOCKET);

  testResult(connect(sckt, server_addr->ai_addr, server_addr->ai_addrlen),
             Errcode::SOCKET);

  freeaddrinfo(server_addr);
}

// Server side
void IPV4Socket::wait() {
  char err_buf[100];

  while (true) {
    cout << "WAITING CONNECTION." << endl;
    fd_set reads;
    reads = waitOnClients(this, sckt);

    if (FD_ISSET(sckt, &reads)) {
      auto client = &clients.emplace_back();
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
  auto status = req->handle(info->socket, true);
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

fd_set waitOnClients(IPV4Socket *sckt, socket_t server) {
  fd_set reads;
  FD_ZERO(&reads);

  FD_SET(server, &reads);
  socket_t max_socket = server;

  for (const auto &client : sckt->clients) {
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
inline void IPV4Socket::send(uint32_t reqn) const {
  SocketIO::send(sckt, (const char *)&reqn, sizeof(uint32_t), 0);
}

void IPV4Socket::sendAndAwait(uint32_t reqn, Request &r) const {
  send(reqn);
  r.handle(sckt, false);
}

auto getClient(IPV4Socket *sckt, socket_t sock) {
  auto isSock = [sock](client_info &c) { return c.socket == sock; };
  auto fr = ranges::find_if(sckt->clients, isSock);
  auto end = sckt->clients.end();
  if (fr != end) {
    return &*fr;
  }

  sckt->clients.emplace_back();
  return &(sckt->clients.back());
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
