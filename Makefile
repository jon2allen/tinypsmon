SRC_DIR = src
INC_DIR = include
TARGET_DIR = target

OSNAME = $$(uname)
OSNAME4 != echo $(OSNAME)
# Flags
CXXFLAGS = -Wall -Wextra -g -std=c++20 -I$(INC_DIR)
#CXXFLAGS = -Wall -Wextra -O2 -std=c++20 -I$(INC_DIR)

#include platform-specific MakefIle
include Makefile.$(OSNAME4)
# Source files
SRCS = ./$(SRC_DIR)/main.cpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(TARGET_DIR)/%.o)

# Executable
EXEC = ./$(TARGET_DIR)/tinypsmon

# Default target
all: $(EXEC)

# Link
$(EXEC): $(OBJS)
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(CXXFLAGS)

# Compile
$(TARGET_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Additional targets
my_key:
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/my_key.cpp -o $(TARGET_DIR)/my_key
	cp ./$(SRC_DIR)/my_key.toml ./$(TARGET_DIR)

process_list_test:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SRC_DIR)/process_list_test3.cpp -o $(TARGET_DIR)/process_list_test

shell_test:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SRC_DIR)/shell_test2.cpp -o $(TARGET_DIR)/shell_test2

shell_test3:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SRC_DIR)/shell_test3.cpp -o $(TARGET_DIR)/shell_test3

