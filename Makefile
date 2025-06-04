CC := gcc
CFLAGS := -Wall -Wextra -pthread -Iinc -fPIC 
LDFLAGS := -pthread -lssl -lcrypto

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INC_DIR := inc

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

SERVER_SRC := $(filter-out $(SRC_DIR)/client.c, $(SRCS))
CLIENT_SRC := $(filter-out $(SRC_DIR)/server.c, $(SRCS))

SERVER_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRC))
CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRC))

# Targets
SERVER := $(BIN_DIR)/server
CLIENT := $(BIN_DIR)/client

all: $(SERVER) $(CLIENT)

# Compile each .c to obj/*.o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build server
$(SERVER): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Build client
$(CLIENT): $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean