#ifdef _WIN32
// socket_t should be SOCKET, but that requires importing winsock
#define socket_t unsigned long long
#else
#define socket_t int
#endif
