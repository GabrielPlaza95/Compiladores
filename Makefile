CC=gcc
CFLAGS=-Wall -Werror --std=c18

all: lexer

lexer: lexer.c
	$(CC) lexer.c -o lexer $(CFLAGS)

clean:
	rm -f lexer out.txt
