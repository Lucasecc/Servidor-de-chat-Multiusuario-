CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude -pthread
LDFLAGS = -pthread

LIB_SRC = src/libtslog.cpp
LIB_OBJ = $(LIB_SRC:.cpp=.o)

SERVER_SRC = src/server.cpp
CLIENT_SRC = examples/client.cpp
TEST_CLIENTS_SRC = examples/test_clients.cpp

SERVER_EXE = server
CLIENT_EXE = client
TEST_CLIENTS_EXE = test_clients

.PHONY: all clean

all: $(SERVER_EXE) $(CLIENT_EXE) $(TEST_CLIENTS_EXE)

$(LIB_OBJ): $(LIB_SRC)
	$(CXX) $(CXXFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

$(SERVER_EXE): $(SERVER_SRC) $(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) $(LIB_OBJ) -o $(SERVER_EXE) $(LDFLAGS)

$(CLIENT_EXE): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o $(CLIENT_EXE) $(LDFLAGS)

$(TEST_CLIENTS_EXE): $(TEST_CLIENTS_SRC)
	$(CXX) $(CXXFLAGS) $(TEST_CLIENTS_SRC) -o $(TEST_CLIENTS_EXE) $(LDFLAGS)

clean:
	rm -f $(SERVER_EXE) $(CLIENT_EXE) $(TEST_CLIENTS_EXE) src/*.o *.log
