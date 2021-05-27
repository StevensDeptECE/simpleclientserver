#include "Errcode.hh"
#include "Ex.hh"
#include "IPV4Socket.hh"
#include <cstdlib>
#include "Request.hh"
using namespace std;

int main(int argc, char* argv[]) {
  int port = argc > 1 ? atoi(argv[1]) : 8060;
	Socket::classInit(); // this is done so Winsock crap initializes
	try {
		Request r;
		IPV4Socket s(port);
		s.attach(&r);
		s.wait(); // main server wait loop
	} catch (const Ex& e) {
		cerr << e << '\n';
	}
	Socket::classCleanup(); // for ugly winsock compatibility
	return 0;
}
