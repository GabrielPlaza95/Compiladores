CC=gcc
CFLAGS=-g -Og -Wall -Werror --std=c18

all: lexer parser

lexer: lexer.c
	$(CC) lexer.c -o lexer $(CFLAGS)

parser: lexer.c lexer.h parser.c
	$(CC) lexer.c parser.c -o parser $(CFLAGS)

clean:
	rm -f lexer out.txt
