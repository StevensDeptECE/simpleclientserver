#include "Socket.hh"
#include "Request.hh"

Socket::Socket(uint16_t port, Request *req)
    : port(port), req(req){}

Socket::Socket(const char *addr, uint16_t port)
    : address(addr),
      port(port),
      req(nullptr) {}

Socket::Socket(uint16_t port)
    : address(nullptr),
      port(port),
      req(nullptr) {}

Socket::~Socket() {
	if (req != nullptr)
		delete req;
}
