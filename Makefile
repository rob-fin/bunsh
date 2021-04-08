.RECIPEPREFIX = >

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

TARGET  = $(BIN_DIR)/bunsh
SRCS    = $(wildcard $(SRC_DIR)/*.c)
OBJS    = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC      = gcc
CFLAGS  = -g -Wall -pedantic
LIBS    = -lreadline

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
>$(CC) $^ $(LIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
>$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
>mkdir -p $@

clean:
>@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)