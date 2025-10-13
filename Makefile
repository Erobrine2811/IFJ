CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -g
SRC = src/scanner.c src/helper.c src/scanner_test.c
OBJ = $(SRC:.c=.o)
TARGET = ifj25

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)