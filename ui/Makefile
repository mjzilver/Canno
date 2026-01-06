CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -ggdb
QT_PKGS := Qt6Widgets

SRC_DIR := src
OBJ_DIR := ../obj/ui
BIN := ../bin/canno-ui

CORE_INC := ../core/include
CORE_LIB := ../bin
CORE_LIBS := -lcanno

CXXFLAGS += -I$(CORE_INC) $(shell pkg-config --cflags $(QT_PKGS))
LDFLAGS  += -L$(CORE_LIB) $(CORE_LIBS) $(shell pkg-config --libs $(QT_PKGS))

MOC := moc

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
HPP_FILES := $(wildcard $(SRC_DIR)/*.hpp)

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))
MOC_SRC := $(patsubst $(SRC_DIR)/%.hpp,$(OBJ_DIR)/%.moc.cpp,$(HPP_FILES))
MOC_OBJ := $(patsubst %.moc.cpp,%.moc.o,$(MOC_SRC))

.PHONY: all run debug clean  moc format tidy

all: run

run: $(BIN)
	LD_LIBRARY_PATH=$(CORE_LIB) $(BIN)

debug: $(BIN)
	env LD_LIBRARY_PATH=$(CORE_LIB) gdb ./$(BIN)


$(BIN): $(OBJ_FILES) $(MOC_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------
# MOC
# ---------
$(OBJ_DIR)/%.moc.cpp: $(SRC_DIR)/%.hpp
	@mkdir -p $(OBJ_DIR)
	$(MOC) $< -o $@

$(OBJ_DIR)/%.moc.o: $(OBJ_DIR)/%.moc.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

moc: $(MOC_OBJ)

# ---------
# Formatting
# ---------
format:
	clang-format -i -style=file:../.clang-format $(CPP_FILES) $(HPP_FILES)

tidy:
	clang-tidy -p=../build $(CPP_FILES) $(HPP_FILES) --config-file=../.clang-tidy -- $(CXXFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN)
