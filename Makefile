# Compiler and compiler flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall
LDFLAGS = -lSDL2

DEBUG_CXXFLAGS = -g

TARGET = 2dphysics

SRC_DIR = src
BUILD_DIR = build

# Recursively find all source files in SRC_DIR and its subdirectories
SRCS = $(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

VPATH = $(SRC_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

setup:
	# clangd LSP bindings
	bear -- make

debug: CXXFLAGS += $(DEBUG_CXXFLAGS)
debug: $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: clean
