CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage
INCLUDE = -I../Convex

TARGET = convex_server

all: $(TARGET)

$(TARGET): convex_server.cpp ../Convex/convex.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f $(TARGET) 

.PHONY: all clean

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage
INCLUDE = -I../Convex
SOURCES = convex_server.cpp ../Convex/convex.cpp
TARGET = convex_server

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f $(TARGET) *.gcno *.gcda *.gcov

.PHONY: all clean
