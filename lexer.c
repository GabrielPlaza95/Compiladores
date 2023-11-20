#include "compiler.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define RESERVED_ID_N 21

typedef enum {
	INVALID_CHARACTER,
	INVALID_TOKEN,
	INVALID_ESCAPE_SEQUENCE,
	UNTERMINATED_STRING,
	UNTERMINATED_COMMENT
}
ErrorClass;

typedef struct symbol_table_node SymbolTableNode;
typedef struct error_list_node ErrorListNode;
typedef struct state_machine_output StateMachineOutput;
typedef StateMachineOutput State(Lexer *lexer, char *start, int token_len);

struct lexer {
	SymbolTableNode *symbol_table;
	ErrorListNode *error_list;

	bool error_flag;
	int current_line;
};

struct symbol_table_node {
	SymbolTableNode *left;
	SymbolTableNode *right;
	char *symbol;
};

struct error_list_node {
	int line;
	ErrorClass error_class;
	ErrorListNode *next;
	char *str;
};

struct state_machine_output {
	Token token;
	State *state;
};

void lexer_error_list_insert(Lexer *lexer, ErrorClass error_class, char *start, int token_len);
void symbol_table_init(Lexer *lexer);
char * symbol_table_insert(Lexer *lexer, char *symbol, int len);
bool is_reserved_id(char *symbol);
bool is_digit(char c);

State state_init;
State state_error;

State state_eq;
State state_deq;

State state_ne_0;
State state_ne_1;

State state_cat_0;
State state_cat_1;

State state_lt;
State state_le;
State state_gt;
State state_ge;

State state_op_delim;

State state_num_0;
State state_num_1;
State state_num_2;

State state_id_0;
State state_id_1;

State state_str_0;
State state_str_1;
State state_str_2;
State state_str_3;
State state_str_4;

State state_cmt_0;
State state_cmt_1;
State state_cmt_2;
State state_cmt_3;
State state_cmt_4;
State state_cmt_5;

char * reserved_id_list[RESERVED_ID_N] = {
	"do",
	"if",
	"in",
	"or",
	"and",
	"end",
	"for",
	"nil",
	"not",
	"else",
	"then",
	"true",
	"break",
	"false",
	"local",
	"until",
	"while",
	"elseif",
	"repeat",
	"return",
	"function"
};

Lexer * lexer_init(void)
{
	Lexer *lexer = malloc(sizeof *lexer);
	
	lexer->symbol_table = NULL;
	lexer->error_list = NULL;

	lexer->error_flag = false;
	lexer->current_line = 1;

	symbol_table_init(lexer);

	return lexer;
}

Token lexer_token_next(Lexer *lexer, char **start)
{
	StateMachineOutput out;
	char c;
	int token_len = 0;

	State *current_state = &state_init;

	while ((c = (*start)[token_len++]) != '\0' || current_state != &state_init) {
		//printf("\nstart: %c\ncurrent: %c\nlen: %i\n", **start, c, token_len + 1);

		out = current_state(lexer, *start, token_len);

		current_state = out.state;

		if (out.token.class == SKIP) {
			*start += token_len;
			token_len = 0;

			continue;
		}

		if (out.token.class != NONE) {
			*start += out.token.len;

			out.token.line = lexer->current_line;

			return out.token;
		}
	}

	out.token.line = lexer->current_line;
	out.token.class = NONE;
	return out.token;
}

bool lexer_error_detected(Lexer *lexer)
{
	return lexer->error_flag;
}

void lexer_error_list_print(Lexer *lexer)
{
	ErrorListNode *next = lexer->error_list;

	while (next != NULL) {
		switch (next->error_class) {
		case INVALID_CHARACTER:
			fprintf(stderr, "Caractere inválido '%c' na linha %i\n", next->str[0], next->line);
			break;
		case INVALID_TOKEN:
			fprintf(stderr, "Token inválido '%c' na linha %i\n", next->str[0], next->line);
			break;
		case INVALID_ESCAPE_SEQUENCE:
			int len = strlen(next->str);
			fprintf(stderr, "Sequência de escape inválida \\%c na linha %i\n", next->str[len - 1], next->line);
			break;
		case UNTERMINATED_STRING:
			fprintf(stderr, "Cadeia de caracteres na linha %i não encerrada\n", next->line);
			break;
		case UNTERMINATED_COMMENT:
			fprintf(stderr, "Comentário de múltiplas linhas aberto na linha %i e não encerrado\n", next->line);
			break;
		}
		next = next->next;
	}
}

void lexer_token_print(Token token)
{
	switch (token.class) {
	case NONE:
	case SKIP:
		break;
	case DEQ:
		printf("<nome-token: '=='>\n");
		break;
	case NE:
		printf("<nome-token: '~='>\n");
		break;
	case LE:
		printf("<nome-token: '<='>\n");
		break;
	case GE:
		printf("<nome-token: '>='>\n");
		break;
	case CAT:
		printf("<nome-token: '..'>\n");
		break;
	case NUM:
		printf("<nome-token: NUM, atributo: %s>\n", token.str);
		break;
	case STRING:
		printf("<nome-token: STRING, atributo: %s>\n", token.str);
		break;
	case NAME:
		printf("<nome-token: NAME, atributo: %s>\n", token.str);
		break;
	default:
		printf("<nome-token: '%s'>\n", token.str);
		break;
	}
}

void lexer_error_list_insert(Lexer *lexer, ErrorClass error_class, char *start, int token_len)
{
	ErrorListNode *head = lexer->error_list;
	ErrorListNode *next;

	lexer->error_flag = true;

	next = malloc(sizeof *next);
	next->next = NULL;
	next->error_class = error_class;
	next->line = lexer->current_line;
	next->str = malloc(token_len + 1);
	next->str[token_len] = '\0';
	strncpy(next->str, start, token_len);

	if (lexer->error_list == NULL) {
		lexer->error_list = next;
		return;
	}

	while (head->next != NULL)
		head = head->next;
	head->next = next;
}

void symbol_table_init(Lexer *lexer)
{
	for (int i = 0; i < RESERVED_ID_N; i++)
		symbol_table_insert(lexer, reserved_id_list[i], strlen(reserved_id_list[i]));
}

char * symbol_table_insert(Lexer *lexer, char *symbol, int len)
{
	SymbolTableNode *root = lexer->symbol_table;
	SymbolTableNode *next;

	next = malloc(sizeof *next);
	next->left = NULL;
	next->right = NULL;
	next->symbol = malloc(len + 1);
	next->symbol[len] = '\0';
	strncpy(next->symbol, symbol, len);

	if (lexer->symbol_table == NULL) {
		lexer->symbol_table = next;
		return lexer->symbol_table->symbol;
	}

	while (true) {
		int cmp = strncmp(symbol, root->symbol, len);
		int root_len = strlen(root->symbol);

		if (cmp == 0 && len == root_len) {
			free(next);
			return root->symbol;
		}

		if (cmp < 0 || (cmp == 0 && len < root_len)) {
			if (root->left == NULL) {
				root->left = next;
				break;
			}
			root = root->left;
		}
		else if (root->right == NULL) {
			root->right = next;
			break;
		}
		else {
			root = root->right;
		}
	}
	return next->symbol;
}

TokenClass id_class(char *symbol)
{
	int i, cmp;

	for (i = 0; i < RESERVED_ID_N; i++) {
		cmp = strcmp(symbol, reserved_id_list[i]);

		if (cmp == 0)
			return NAME + i + 1;
	}

	return NAME;
}

TokenClass op_delim_class(char symbol)
{
	//char op_delim[] = "=<>+-*/%^.()[]{},:;";
	
	char op_delim[] = "=<>+-*/%^()[]{},;";
	
	return (strchr(op_delim, symbol) - op_delim) + EQ;
}





bool is_digit(char c)
{
	return isdigit(c) > 0;
}

StateMachineOutput state_init(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	out.token = TOKEN_NONE;

	if (c == '\n')
		lexer->current_line++;

	switch (c) {
	case '=':
		out.state = &state_eq;
		break;
	case '<':
		out.state = &state_lt;
		break;
	case '>':
		out.state = &state_gt;
		break;
	case '.':
		out.state = &state_cat_0;
		break;
	case '~':
		out.state = &state_ne_0;
		break;
	case '"':
		out.state = &state_str_0;
		break;
	case '-':
		out.state = &state_cmt_0;
		break;
	default:
		if (isspace(c)) {
			out.state = &state_init;
			out.token = TOKEN_SKIP;
		}
		else if (strchr("=<>+-*/%^()[]{},;", c) != NULL) {
			out.state = &state_op_delim;
		}
		else if (is_digit(c)) {
			out.state = &state_num_0;
		}
		else if (isalpha(c) || c == '_') {
			out.state = &state_id_0;
		}
		else {
			out.state = &state_init;
			lexer_error_list_insert(lexer, INVALID_CHARACTER, start, token_len);
		}
	}

	return out;
}

StateMachineOutput state_eq(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;

	char c = start[token_len - 1];

	out.state = (c == '=') ? &state_deq : &state_op_delim;
	out.token = TOKEN_NONE;

	return out;
}

StateMachineOutput state_deq(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out = {
		.token.class = DEQ,
		.token.len = 2
	};

	return out;
};

StateMachineOutput state_ne_0(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;

	char c = start[token_len - 1];

	if (c != '=') {
		out.state = &state_init;
		out.token = TOKEN_SKIP;
		lexer_error_list_insert(lexer, INVALID_TOKEN, start, token_len);
	}
	else {
		out.state = &state_ne_1;
		out.token = TOKEN_NONE;
	}

	return out;
}

StateMachineOutput state_ne_1(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out = {
		.token.class = NE,
		.token.len = 2
	};

	return out;
};

StateMachineOutput state_cat_0(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c != '.') {
		out.state = &state_init;
		out.token = TOKEN_SKIP;
		lexer_error_list_insert(lexer, INVALID_TOKEN, start, 1);
	}
	else {
		out.state = &state_cat_1;
		out.token = TOKEN_NONE;
	}

	return out;
};

StateMachineOutput state_cat_1(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out = {
		.token.class = CAT,
		.token.len = 2
	};

	return out;
};

StateMachineOutput state_lt(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	out.state = (c == '=') ? &state_le : &state_op_delim;
	out.token = TOKEN_NONE;

	return out;
}

StateMachineOutput state_le(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out = {
		.token.class = LE,
		.token.len = 2
	};

	return out;
};

StateMachineOutput state_gt(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	out.state = (c == '=') ? &state_ge : &state_op_delim;
	out.token = TOKEN_NONE;

	return out;
}

StateMachineOutput state_ge(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out = {
		.token.class = GE,
		.token.len = 2
	};

	return out;
};

StateMachineOutput state_op_delim(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	
	out.token.len = 1;
	out.token.str = symbol_table_insert(lexer, start, out.token.len);
	out.token.class = op_delim_class(*out.token.str);

	return out;
};

StateMachineOutput state_num_0(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (is_digit(c))
		out.state = &state_num_0;
	else if (c == '.')
		out.state = &state_num_1;
	else
		out.state = &state_num_2;

	out.token = TOKEN_NONE;
	return out;
}

StateMachineOutput state_num_1(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	out.state = is_digit(c) ? &state_num_1 : &state_num_2;
	out.token = TOKEN_NONE;

	return out;
}

StateMachineOutput state_num_2(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;

	token_len -= 2;

	out.token.class = NUM;
	out.token.len = token_len;
	out.token.str = symbol_table_insert(lexer, start, out.token.len);

	return out;
};

StateMachineOutput state_id_0(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	out.state = (isalnum(c) || c == '_') ? &state_id_0 : &state_id_1;
	out.token = TOKEN_NONE;

	return out;
}

StateMachineOutput state_id_1(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;

	token_len -= 2;

	out.token.len = token_len;
	out.token.str = symbol_table_insert(lexer, start, out.token.len);
	out.token.class = id_class(out.token.str);
	
	return out;
};

StateMachineOutput state_str_0(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n')
		lexer->current_line++;

	if (c == '\0') {
		out.state = state_init;
		lexer_error_list_insert(lexer, UNTERMINATED_STRING, start, token_len - 1);
		return out;
	}

	if (c == '"')
		out.state = &state_str_2;
	else if (c == '\\')
		out.state = &state_str_1;
	else
		out.state = &state_str_0;

	out.token = TOKEN_NONE;
	return out;
}

StateMachineOutput state_str_1(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n')
		lexer->current_line++;

	if (c == '\0') {
		out.state = &state_init;
		lexer_error_list_insert(lexer, UNTERMINATED_STRING, start, token_len - 1);
		return out;
	}

	if (strchr("abfnrtv\\\"", c) == NULL) {
		out.state = &state_str_3;
		out.token = TOKEN_SKIP;
		lexer_error_list_insert(lexer, INVALID_ESCAPE_SEQUENCE, start, token_len);
	}
	else {
		out.state = &state_str_0;
		out.token = TOKEN_NONE;
	}

	return out;
}

StateMachineOutput state_str_2(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;

	token_len -= 1;

	out.token.class = STRING;
	out.token.len = token_len;
	out.token.str = symbol_table_insert(lexer, start, out.token.len);

	return out;
};

StateMachineOutput state_str_3(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n')
		lexer->current_line++;

	if (c == '\0') {
		out.state = &state_init;
		lexer_error_list_insert(lexer, UNTERMINATED_STRING, start, token_len - 1);
		return out;
	}

	if (c == '"')
		out.state = &state_init;
	else if (c == '\\')
		out.state = &state_str_4;
	else
		out.state = &state_str_3;

	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_str_4(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n')
		lexer->current_line++;

	if (c == '\0') {
		out.state = &state_init;
		lexer_error_list_insert(lexer, UNTERMINATED_STRING, start, token_len - 1);
		return out;
	}

	if (strchr("abfnrtv\\\"", c) == NULL)
		lexer_error_list_insert(lexer, INVALID_ESCAPE_SEQUENCE, start, token_len);

	out.state = &state_str_3;
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_cmt_0(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c != '-') {
		out.state = &state_op_delim;
		out.token = TOKEN_NONE;
		return out;
	}

	out.state = &state_cmt_1;
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_cmt_1(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n' || c == '\0') {
		out.state = &state_init;
		lexer->current_line++;
	}
	else if (c == '[') {
		out.state = &state_cmt_2;
	}
	else {
		out.state = &state_cmt_5;
	}
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_cmt_2(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n') {
		out.state = &state_init;
		lexer->current_line++;
	}
	else if (c == '[') {
		out.state = &state_cmt_3;
	}
	else {
		out.state = &state_cmt_5;
	}
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_cmt_3(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n')
		lexer->current_line++;

	if (c == '\0') {
		out.state = state_init;
		lexer_error_list_insert(lexer, UNTERMINATED_COMMENT, start, token_len - 1);
		return out;
	}

	out.state = (c != ']') ? &state_cmt_3 : &state_cmt_4;
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_cmt_4(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n')
		lexer->current_line++;

	if (c == '\0') {
		out.state = &state_init;
		lexer_error_list_insert(lexer, UNTERMINATED_COMMENT, start, token_len - 1);
		return out;
	}

	out.state = (c != ']') ? &state_cmt_3 : &state_init;
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput state_cmt_5(Lexer *lexer, char *start, int token_len)
{
	StateMachineOutput out;
	char c = start[token_len - 1];

	if (c == '\n') {
		out.state = &state_init;
		lexer->current_line++;
	}
	else {
		out.state = &state_cmt_5;
	}
	out.token = TOKEN_SKIP;

	return out;
}
