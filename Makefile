CC=gcc
CFLAGS=-g -Og -Wall -Werror --std=c18

all: parser

parser: lexer.c lexer.h parser.c
	$(CC) lexer.c parser.c -o parser $(CFLAGS)

clean:
	rm -f parser out.txt
