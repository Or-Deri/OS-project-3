CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fprofile-arcs -ftest-coverage -pg
INCLUDE = -I../Convex

TARGET = convex_server
TARGET_PROFILE = convex_server_profile

all: $(TARGET)

profile: $(TARGET_PROFILE)
	./$(TARGET_PROFILE)
	mv gmon.out gmon_convex.out
	gprof $(TARGET_PROFILE) gmon_convex.out > profile_convex_server.txt

$(TARGET): convex_server.cpp ../Convex/convex.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

$(TARGET_PROFILE): convex_server.cpp ../Convex/convex.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f $(TARGET) $(TARGET_PROFILE) gmon_convex.out profile_convex_server.txt *.gcda *.gcno

.PHONY: all clean profile
