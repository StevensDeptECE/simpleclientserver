#include "Request.hh"
#include <unistd.h>
#include <iostream>
using namespace std;

/*
	receive request and send back socket to talk on?
 */
void Request::handleServer(int returnsckt) {
	cout << "Returning message on socket: " << returnsckt << '\n';
	char buf[256];

	// first read incoming message from client
	int bytesRead = read(returnsckt, buf, 256);
	cout << "bytes read: " << bytesRead << ' ';
	for (int i = 0; i < bytesRead; i++) // dump out buffer as numbers
		cout << int(buf[i]) << ' ';
	cout << '\n';

	//now write back to client
	const char answer[] = "testing";
	write(returnsckt, answer, sizeof(answer));
}

void Request::handle(int returnsckt) {
	cout << "Received socket: " << returnsckt << '\n';
	char buf[256];
	int bytesRead = read(returnsckt, buf, 256);
	cout << "bytes read: " << bytesRead << ' ';
	for (int i = 0; i < bytesRead; i++) // dump out buffer as numbers
		cout << int(buf[i]) << ' ';
	cout << '\n';
	cout << buf << '\n';
}
