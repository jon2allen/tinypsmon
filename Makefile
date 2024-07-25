#  compiler
CXX = clang++18 

# Directories
SRC_DIR = src 
INC_DIR = include
TARGET_DIR = target

# Flags
CXXFLAGS = -Wall -Wextra -std=c++20 -I$(INC_DIR)
LDFLAGS = -lkvm -lpthread 

# Source files
# SRCS = $(wildcard $(SRC_DIR)/*.cpp)

SRCS = $(SRC_DIR)/jon_proc_test.cpp

# Object files
#OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(TARGET_DIR)/%.o)
OBJS = $(SRC_DIR)/jon_proc_test.o

# Executable
EXEC = $(TARGET_DIR)/tinypsmon

# Default target
all: $(EXEC)

# Link
$(EXEC): $(OBJS)
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile
$(TARGET_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(TARGET_DIR)

.PHONY: all clean

