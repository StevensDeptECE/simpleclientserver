ifeq ($(OS),Windows_NT)
	LIBS	:=	$(LIBSPEC) -lWs2_32
else
	#assuming if not windows, then linux. This will break for unsupported OS or OS that does not declare properly. This will break on OSX.
	LIBS	:=	$(LIBSPEC) -L /usr/lib/x86_64-linux-gnu/
	# @echo "LINUX DETECTED"
endif




all: simpleserver simpleclient

simpleserver: SimpleServer.cc IPV4Socket.cc Socket.cc Request.cc ErrNames.cc
	g++ -g $^ -o $@ $(LIBS)

simpleclient: SimpleClient.cc IPV4Socket.cc Socket.cc Request.cc ErrNames.cc
	g++ -g $^ -o $@ $(LIBS)

clean:
	rm simpleclient simpleserver