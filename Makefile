CC = g++
PROG = simhttp simget

DEPS = httpSim.h
CPLUSFLAGS = -c -ggdb -O2 -Wall

SERVERSRC = httpServer.cpp ErrorFunctions.cpp
SERVEROBJS = $(SERVERSRC:.c=.o)

CLIENTSRC = httpClient.cpp ErrorFunctions.cpp
CLIENTOBJS = $(CLIENTSRC:.cpp=.o)


all: $(PROG)

simhttp: $(SERVERSRC)
	${CC} $(CPLUSFLAGS) -c -o httpServer.o httpServer.cpp
	${CC} $(CPLUSFLAGS) -c -o ErrorFunctions.o ErrorFunctions.cpp
	${CC} -o $@ httpServer.o ErrorFunctions.o

simget: $(CLIENTSRC)
	${CC} $(CPLUSFLAGS) -c -o httpClient.o httpClient.cpp
	${CC} $(CPLUSFLAGS) -c -o ErrorFunctions.o ErrorFunctions.cpp
	${CC} -o $@ httpClient.o ErrorFunctions.o

#test:
#	./dnsq -t 5 -r 5 @192.168.1.254 www.clemson.edu

backup:
	rm -f awbrenn_assignment3.tar.gz *.o *.out client *~
	tar -cf awbrenn_assignment3.tar *.cpp *.h readme.txt Makefile
	gzip -f awbrenn_assignment3.tar

clean:
	rm -f $(PROG) *.o *.out client *~
