#pragma once

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // Need to be at least Vista
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib?
#pragma comment(lib, "Ws2_32.lib")  // Ignored by MinGW

#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) (closesocket(s))
#define GETSOCKETERRNO() (WSAGetLastError())
#else  // linux
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) (close(s))
#define GETSOCKETERRNO() (errno)
#define INVALID_SOCKET -1
#endif

#ifndef socket_t
using socket_t = decltype(socket(0, 0, 0));
#endif

namespace SocketIO {
namespace {
#ifdef __linux__
const static int err_code = -1;
#elif _WIN32
const static int err_code = SOCKET_ERROR;
#endif
 char err_buf[100] = {'\0'};
}  // namespace

 int send(socket_t sckt, const char *buf, int size, int flags);
 int recv(socket_t sckt, const char *buf, int size, int flags);
};  // namespace SocketIO