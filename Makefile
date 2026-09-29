CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -Iclient

SERVER_SRC = src/Server.cpp src/Logger.cpp src/AlertManager.cpp src/ProcessManager.cpp src/KernelMonitor.cpp src/ResourceMonitor.cpp src/RecoveryManager.cpp src/main.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
SERVER_TARGET = sentinel_os

CLIENT_SRC = client/Client.cpp client/client_main.cpp src/Logger.cpp
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
CLIENT_TARGET = sentinel_client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -lpthread -o $@ $(SERVER_OBJ)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -lpthread -o $@ $(CLIENT_OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o client/*.o $(SERVER_TARGET) $(CLIENT_TARGET) logs/*.log *.log

.PHONY: all clean
