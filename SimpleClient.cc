#include "IPV4Socket.hh"
#include "Errcode.hh"
#include "Ex.hh"
#include "Request.hh"
using namespace std;

int main(int argc, char* argv[]) {
	const char* ip = argc > 1 ? argv[1] : "127.0.0.1";
	int port = argc > 2 ? atoi(argv[2]) : 8060;
	uint32_t req = argc > 3 ? atoi(argv[3]) : 1;

	Socket::classInit();
	try {
		Request r;
		IPV4Socket s(ip, port);
    s.sendAndAwait(req, r);
        } catch (const Ex& e) {
		cerr << e << '\n';
	}
	Socket::classCleanup();
	return 0;
}
