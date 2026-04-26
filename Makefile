CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -ljansson

TARGET = vdb
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

FIND_SRC = find.c
FIND_LIB = libfind.so

.PHONY: all clean run lib

all: $(TARGET) lib

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lib:
	$(CC) $(CFLAGS) -fPIC -c find.c -o find.o
	$(CC) $(CFLAGS) -fPIC -c has_fields.c -o has_fields.o
	$(CC) -shared find.o has_fields.o -o libfind.so -ljansson

clean:
	rm -f $(TARGET) $(OBJ) find.o $(FIND_LIB)

run: $(TARGET)
	./$(TARGET)
