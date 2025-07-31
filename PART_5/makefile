CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

all: libreacotor.a

reactor.o: reactor.cpp reactor.hpp
	$(CXX) $(CXXFLAGS) -c reactor.cpp -o reactor.o

libreacotor.a: reactor.o
	ar rcs libreacotor.a reactor.o

clean:
	rm -f *.o libreacotor.a
