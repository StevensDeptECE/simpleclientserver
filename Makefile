all: simpleserver simpleclient

simpleserver: SimpleServer.cc IPV4Socket.cc Socket.cc Request.cc ErrNames.cc
	g++ -g $^ -o $@

simpleclient: SimpleClient.cc IPV4Socket.cc Socket.cc Request.cc ErrNames.cc
	g++ -g $^ -o $@
