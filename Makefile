ifeq ($(OS),Windows_NT)
	LIBS	:=	$(LIBSPEC) -lWs2_32
else
	#assuming if not windows, then linux. This will break for unsupported OS or OS that does not declare properly. This will break on OSX.
	LIBS	:=	$(LIBSPEC) 
	# @echo "LINUX DETECTED"
endif


COMP := clang++
DEBUG := -g
OPT := -O1
CXXFLAGS := --std=c++20
ADDRSAN := -fsanitize=address -fno-omit-frame-pointer
MEMSAN := -fsanitize=memory -fPIE -pie
UBSAN := -fsanitize=undefined
COMP += ${DEBUG} ${OPT} ${CXXFLAGS} ${ADDRSAN}

all: bin/simpleserver bin/simpleclient bin/logging_demo

bin/simpleserver: SimpleServer.cc SocketIO.cc IPV4Socket.cc Socket.cc Request.cc ErrNames.cc  | bin
	${COMP} $^ -o $@ $(LIBS)

bin/simpleclient: SimpleClient.cc SocketIO.cc IPV4Socket.cc Socket.cc Request.cc ErrNames.cc  | bin
	${COMP} $^ -o $@ $(LIBS)

bin/logging_demo: logging_demo.cc | bin
	${COMP} $^ -o $@ $(LIBS)

bin:
	mkdir bin/

clean:
	rm bin/simpleclient bin/simpleserver
