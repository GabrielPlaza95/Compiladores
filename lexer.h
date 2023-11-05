#include <stdbool.h>

#ifndef LEXER_H
#define LEXER_H

#define TOKEN_NONE (Token) { 0 }
#define TOKEN_SKIP (Token) { SKIP, 0 }

typedef enum {
	SKIP = -1,
	NONE = 0,
	DEQ = 256,
	NE,
	LE,
	GE,
	CAT,
	OP_DELIM,
	NUM,
	ID,
	STRING
}
TokenClass;

typedef struct {
	TokenClass class;
	int len;
	char *str;
}
Token;

typedef struct lexer Lexer;

struct lexer;

Lexer * lexer_init(void);
Token next_token(Lexer *lexer, char **start);
bool error_detected(Lexer *lexer);
void print_errors(Lexer *lexer);
void print_token(Token token);

#endif
