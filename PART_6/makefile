CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage
INCLUDE = -I../Convex -I../PART_5
SOURCES = convex_server.cpp ../Convex/convex.cpp ../PART_5/reactor.cpp
TARGET = convex_server

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f $(TARGET) *.o *.gcno *.gcda *.gcov

.PHONY: all clean
