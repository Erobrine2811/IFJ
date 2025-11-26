CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -g
SRC = src/scanner.c src/helper.c src/parser.c src/symtable.c src/main.c src/expr_parser.c src/symstack.c src/semantic.c src/3AC.c src/3AC_patterns.c
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
	 make
	./scripts/run_tests.sh

submission:
	./scripts/prepare_submission.sh

test-lex:
	make
	cd scripts/ && ./run_lex_tests.sh

test-stx:
	make
	cd scripts/ && ./run_stx_tests.sh

test-sem:
	make
	cd scripts/ && ./run_sem_tests.sh

test-all:
	make
	cd scripts/ && ./run_all_tests.sh
