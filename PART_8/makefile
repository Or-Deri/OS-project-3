CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

all: reactor

reactor.o: reactor.cpp Reactor.hpp
	$(CXX) $(CXXFLAGS) -c reactor.cpp -o reactor.o

clean:
	rm -f *.o *.a main