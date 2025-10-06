CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -fPIC
LIBFLAGS := -shared

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CPP_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
HPP_FILES := $(shell find $(SRC_DIR) -name "*.hpp")

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))
MAIN_OBJ := $(OBJ_DIR)/main.o
LIB_OBJS := $(filter-out $(MAIN_OBJ),$(OBJ_FILES))

EXEC := $(BIN_DIR)/canno
LIB := $(BIN_DIR)/libcanno.so

.PHONY: all run clean

all: $(EXEC) $(LIB)

$(EXEC): $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(LIB): $(LIB_OBJS) | $(BIN_DIR)
	$(CXX) $(LIBFLAGS) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# ---------
# Formatting
# ---------

format:
	clang-format -i $(CPP_FILES) $(HPP_FILES)

tidy:
	clang-tidy -p $(CPP_FILES) $(HPP_FILES) -- $(CXXFLAGS)

# ---------
# Python frontend
# ---------
PYTHON := $(shell command -v python3 || command -v python)

.PHONY: py
py: $(LIB)
	$(PYTHON) py/front.py
