CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC = src/Server.cpp src/Logger.cpp src/AlertManager.cpp src/ProcessManager.cpp src/KernelMonitor.cpp src/ResourceMonitor.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = sentinel_os

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -lpthread -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) logs/*.log *.log

.PHONY: all clean
