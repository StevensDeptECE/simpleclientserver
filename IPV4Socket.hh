#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Socket.hh"
#define MAX_REQUEST_SIZE 2047
struct client_info;



/*
        IPV4Socket represents a socket connection that is portable between all
   platforms. The code must all be encapsulated in IPV4Socket.cc and hides any
   implementation details This implementation uses TCP/IP sockets. Eventually a
   corresponding class UDP4Socket will be written for faster Datagrams without
   acknowledgement and a guaranteed delivery system will have to be written
   on top of that. Header files are being moved to .cc so external code does not
   have to deal with the large includes as well as the system specific nature of
   those includes

*/
class IPV4Socket : public Socket {
 public:
  IPV4Socket(const char* addr, uint16_t port);  // Client
  IPV4Socket(uint16_t port, const char* addr=NULL);  // Server
  ~IPV4Socket();
  void wait();
  void send(const char* command);  // For HTTP
  void send(uint32_t reqn) const;
  void sendAndAwait(uint32_t reqn, Request& r) const;

  std::vector<client_info> clients; // TODO: figure out if this is compiler magic or UB

 private:
  void dropClient(struct client_info* info);
  std::string getClientAddress(struct client_info* info);
  // client_info* getClient(socket_t sock); // Implemented in cc file
	void handleServer(client_info* info); // Handle requests on server-side and conditionally drop
  socket_t sckt;
};