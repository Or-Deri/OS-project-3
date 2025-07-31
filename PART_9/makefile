CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage
LIBS = ../PART_8/libreacotor.a
SOURCES = thread_for_client.cpp ../Convex/convex.cpp
TARGET = thread_for_client

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lpthread

clean:
	rm -f $(TARGET) *.o *.gcno *.gcda *.gcov

.PHONY: all clean
