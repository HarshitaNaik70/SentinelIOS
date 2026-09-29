CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC = src/AlertManager.cpp src/ResourceMonitor.cpp src/ProcessManager.cpp src/KernelMonitor.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = sentinel_prototype

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) sentinel_stage4.log sentinel.log

.PHONY: all clean
