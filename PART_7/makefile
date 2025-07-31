CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage
INCLUDE = -I../Convex
SOURCES = thread_for_client.cpp ../Convex/convex.cpp
TARGET = thread_for_client

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f $(TARGET) *.gcno *.gcda *.gcov

.PHONY: all clean
