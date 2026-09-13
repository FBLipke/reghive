# BCDTool Makefile - Pure C++ Registry Hive / BCD Parser
# =========================================================

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2

# Directories
SRC_DIR = src
INC_DIR = include
REGHIV_DIR = RegHive
BCD_DIR = BCD
BIN_DIR = ../../../bin

# All include paths
INCLUDES = -I$(INC_DIR) -I$(REGHIV_DIR) -I$(BCD_DIR)

# Sources
SOURCES = $(SRC_DIR)/main.cpp \
          $(REGHIV_DIR)/RegHive.cpp \
          $(BCD_DIR)/BCD.cpp

# Objects
OBJECTS = $(SOURCES:.cpp=.o)

# Target
TARGET = $(BIN_DIR)/bcdtool

# Default target
all: $(TARGET)

# Create bin directory if needed
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -f $(SRC_DIR)/*.o $(REGHIV_DIR)/*.o $(BCD_DIR)/*.o $(TARGET)

# Run - Read BCD
run: $(TARGET)
	./$(TARGET) read default.bcd

# Create test BCD
create: $(TARGET)
	./$(TARGET) create test.bcd --blocksize 65564 --windowsize 32 --varwindow

# Help
help:
	@echo "BCDTool Makefile - Pure C++ BCD Parser"
	@echo "========================================"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build bcdtool (default)"
	@echo "  clean   - Remove objects and binary"
	@echo "  run     - Read default.bcd"
	@echo "  create  - Create test.bcd"
	@echo "  help    - Show this help"
	@echo ""
	@echo "Usage:"
	@echo "  make"
	@echo "  make clean"
	@echo "  make run"
	@echo "  make create"
	@echo ""
	@echo "Commands:"
	@echo "  ./$(TARGET) read <file.bcd>"
	@echo "  ./$(TARGET) create <output.bcd> [options]"
