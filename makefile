CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude
SRC = src/libtslog.cpp
EXAMPLE = examples/test_logging.cpp

all: test_logging

test_logging: $(SRC) $(EXAMPLE)
	$(CXX) $(CXXFLAGS) $(SRC) $(EXAMPLE) -o test_logging

clean:
	rm -f test_logging *.o *.log

.PHONY: all clean
