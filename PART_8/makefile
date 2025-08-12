CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

all: libreacotor.a

libreacotor.a: reactor.o
	ar rcs libreacotor.a reactor.o

reactor.o: reactor.cpp reactor.hpp
	g++ -c reactor.cpp -o reactor.o 

clean:
	rm -f *.o libreacotor.a
