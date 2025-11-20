CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -g
SRC = src/scanner.c src/helper.c src/parser.c src/symtable.c src/main.c src/expr_parser.c src/symstack.c src/semantic.c src/3AC.c
OBJ = $(SRC:.c=.o)
TARGET = ifj25

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

test:
	./scripts/run_tests.sh
