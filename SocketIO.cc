#include "SocketIO.hh"

#include <cstdint>
#include <cstdio>

#include "Errcode.hh"
#include "Ex.hh"

// TODO: Look into logging WSAGetLastError and strerror(errno)
// Using the logging object in csp.hh?
int SocketIO::send(socket_t sckt, const char *buf, int size, int flags) {
  uint32_t bytesSent;
  if ((bytesSent = ::send(sckt, (char *)buf, size, 0)) == err_code) {
    sprintf(err_buf, "send() failed. (%d)", GETSOCKETERRNO());
    throw Ex2(Errcode::SOCKET, err_buf);
  }
  return bytesSent;
}

int SocketIO::recv(socket_t sckt, const char *buf, int size, int flags) {
  uint32_t bytesRecv;
  if ((bytesRecv = ::recv(sckt, (char *)buf, size, 0)) == err_code) {
    sprintf(err_buf, "recv() failed. (%d)", GETSOCKETERRNO());
    throw Ex2(Errcode::SOCKET, err_buf);
  }
  return bytesRecv;
}