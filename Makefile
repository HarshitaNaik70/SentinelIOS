CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -Iclient -Idriver -D_WIN32_WINNT=0x0600

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
else
    LDFLAGS = -lpthread
endif

SERVER_SRC = src/Server.cpp src/ProcessManager.cpp src/KernelMonitor.cpp src/ResourceMonitor.cpp src/main.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
SERVER_TARGET = sentinel_os

CLIENT_SRC = client/Client.cpp client/client_main.cpp
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
CLIENT_TARGET = sentinel_client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJ) $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o client/*.o $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_TARGET).exe $(CLIENT_TARGET).exe logs/*.log *.log

.PHONY: all clean
