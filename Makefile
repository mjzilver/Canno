CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CPP_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
HPP_FILES := $(shell find $(SRC_DIR) -name "*.hpp")

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))

EXEC := $(BIN_DIR)/canno

.PHONY: all run clean

all: $(EXEC)

$(EXEC): $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: $(EXEC)
	./$(EXEC)

format:
	clang-format -i $(CPP_FILES) $(HPP_FILES)

tidy:
	clang-tidy -p $(CPP_FILES) $(HPP_FILES) -- $(CXXFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
