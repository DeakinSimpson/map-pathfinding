CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm

SRC = $(wildcard src/*.c) $(wildcard src/routing/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
TARGET = build/pathfinder

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.c
	@if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build/

.PHONY: all clean