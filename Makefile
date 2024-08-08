SRC_DIR = src
INC_DIR = include
TARGET_DIR = target

# Flags
CXXFLAGS = -Wall -Wextra -g -std=c++20 -I$(INC_DIR)
#CXXFLAGS = -Wall -Wextra -O2 -std=c++20 -I$(INC_DIR)
LDFLAGS = -lkvm -lpthread

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
	$(CXX) $(CXXFLAGS) -lkvm $(SRC_DIR)/process_list_test3.cpp -o $(TARGET_DIR)/process_list_test

shell_test:
	$(CXX) $(CXXFLAGS) -lkvm $(SRC_DIR)/shell_test2.cpp -o $(TARGET_DIR)/shell_test2

shell_test3:
	$(CXX) $(CXXFLAGS) -lkvm $(SRC_DIR)/shell_test3.cpp -o $(TARGET_DIR)/shell_test3

toml_test1:
	$(CXX) $(CXXFLAGS) -lkvm $(SRC_DIR)/toml_test1.cpp -o $(TARGET_DIR)/toml_test1


toml_test2:
	$(CXX) $(CXXFLAGS) -lkvm $(SRC_DIR)/toml_test2.cpp -o $(TARGET_DIR)/toml_test2

file_perm:
	$(CXX) $(CXXFLAGS) -lkvm $(SRC_DIR)/file_perm.cpp -o $(TARGET_DIR)/file_perm

toml:
	cp $(SRC_DIR)/config.toml $(TARGET_DIR)
# Clean
clean:
	rm -rf $(TARGET_DIR)

.PHONY: all clean my_key process_list_test

