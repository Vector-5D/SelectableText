# compiler
CXX := clang++
CC  := clang

CFLAGS := -Wall -Wshadow -Iincl --std=c23

# output
OBJ_DIR := build
SRC_DIR := src
OUTPUT  := bin/program

SRCS_C   := $(shell find $(SRC_DIR) -name *.c)
SRCS_CPP := $(shell find $(SRC_DIR) -name *.cpp)

OBJS := $(subst $(SRC_DIR), $(OBJ_DIR), $(SRCS_C:.c=.o))
OBJS += $(subst $(SRC_DIR), $(OBJ_DIR), $(SRCS_CPP:.cpp=.o))

# building
all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CXX) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf $(OBJS) $(OUTPUT)
