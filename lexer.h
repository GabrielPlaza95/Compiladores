#ifndef LEXER_H
#define LEXER_H

#define TOKEN_NONE (Token) { 0 }
#define TOKEN_SKIP (Token) { SKIP, 0 }

enum symbol;

typedef enum symbol TokenClass;

typedef struct {
	TokenClass class;
	int line;
	int len;
	char *str;
}
Token;

typedef struct lexer Lexer;

struct lexer;

Lexer * lexer_init(void);
Token lexer_token_next(Lexer *lexer, char **start);
_Bool lexer_error_detected(Lexer *lexer);
void lexer_error_list_print(Lexer *lexer);
void lexer_token_print(Token token);

#endif
