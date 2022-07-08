ifeq ($(OS),Windows_NT)
	LIBS	:=	$(LIBSPEC) -lWs2_32
else
	#assuming if not windows, then linux. This will break for unsupported OS or OS that does not declare properly. This will break on OSX.
	LIBS	:=	$(LIBSPEC) 
	# @echo "LINUX DETECTED"
endif


COMP := clang++
DEBUG := -g
OPT := 
CXXFLAGS := --std=c++20
COMP += ${DEBUG} ${OPT} ${CXXFLAGS}

all: bin/simpleserver bin/simpleclient

bin/simpleserver: SimpleServer.cc IPV4SocketPlat.cc Socket.cc Request.cc ErrNames.cc | bin
	${COMP} $^ -o $@ $(LIBS)

bin/simpleclient: SimpleClient.cc IPV4SocketPlat.cc Socket.cc Request.cc ErrNames.cc | bin
	${COMP} $^ -o $@ $(LIBS)

bin:
	mkdir bin/

clean:
	rm bin/simpleclient bin/simpleserver
