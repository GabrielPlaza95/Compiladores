#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define TOKEN_NONE (Token) { 0 }
#define TOKEN_SKIP (Token) { SKIP, 0 }

#define RESERVED_ID_N 21

enum TokenClasses {
	SKIP = -1,
	NONE = 0,
	DEQ = 256,
	NE,
	LT,
	LE,
	GT,
	GE,
	OP_DELIM,
	NUM,
	ID,
	STRING,
};


enum ErrorClasses {
	INVALID_CHARACTER,
	INVALID_TOKEN,
	INVALID_ESCAPE_SEQUENCE,
	UNTERMINATED_STRING,
	UNTERMINATED_COMMENT
};

struct state;

typedef struct {
	int class;
	int len;
	char *str;
}
Token;

typedef struct {
	Token token;
	struct state *state;
}
StateMachineOutput;

typedef struct state {
	StateMachineOutput (*next)(char *start, int token_len);
}
State;

typedef struct symbol_table_node {
	struct symbol_table_node *left;
	struct symbol_table_node *right;
	char *symbol;
}
SymbolTableNode;

typedef struct error_list_node {
	int line;
	int error_class;
	char *str;
	struct error_list_node *next;
}
ErrorListNode;

State
	state_init,
	state_error,
	
	state_eq,
	
	state_deq,
	
	state_ne_0,
	state_ne_1,
	
	state_lt,
	state_le,
	
	state_gt,
	state_ge,
	
	state_op_delim,
	
	state_num_0,
	state_num_1,
	state_num_2,
	
	state_id_0,
	state_id_1,
	
	state_str_0,
	state_str_1,
	state_str_2,
	state_str_3,
	state_str_4,
	
	state_cmt_0,
	state_cmt_1,
	state_cmt_2,
	state_cmt_3,
	state_cmt_4,
	state_cmt_5;

char *reserved_id_list[RESERVED_ID_N] = {
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

SymbolTableNode *symbol_table = NULL;
ErrorListNode *error_list = NULL;

bool error_flag = false;
int current_line = 1;

char *symbol_table_insert(char *symbol, int len) {
	SymbolTableNode *root = symbol_table;
	SymbolTableNode *next;
	
	next = malloc(sizeof *next);
	next->left = NULL;
	next->right = NULL;
	next->symbol = malloc(len + 1);
	next->symbol[len + 1] = '\0';
	strncpy(next->symbol, symbol, len);
	
	if (symbol_table == NULL) {
		symbol_table = next;
		return symbol_table->symbol;
	}
	
	while (true) {
		int cmp = strncmp(symbol, root->symbol, len);
		int root_len = strlen(root->symbol);
		
		if (cmp == 0 && len == root_len) {
			free(next);
			return(root->symbol);
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

void error_list_insert(int error_class, char *start, int token_len) {
	ErrorListNode *head = error_list;
	ErrorListNode *next;
	
	error_flag = true;
	
	next = malloc(sizeof *next);
	next->next = NULL;
	next->error_class = error_class;
	next->line = current_line;
	next->str = malloc(token_len);
	next->str[token_len] = '\0';
	strncpy(next->str, start, token_len);
	
	if (error_list == NULL) {
		error_list = next;
		return;
	}
	
	while (head->next != NULL) {
		head = head->next;
	}
	head->next = next;
}

void print_errors(void) {
	ErrorListNode *next = error_list;
	
	while (next != NULL) {
		switch (next->error_class) {
		case INVALID_CHARACTER:
			printf("Caractere inválido '%c' na linha %i\n", next->str[0], next->line);
			break;
		case INVALID_TOKEN:
			printf("Token inválido ~ na linha %i\n", next->line);
			break;
		case INVALID_ESCAPE_SEQUENCE:
			int len = strlen(next->str);
			printf("Sequência de escape inválida \\%c na linha %i\n", next->str[len - 1], next->line);
			break;
		case UNTERMINATED_STRING:
			printf("Cadeia de caracteres na linha %i e não encerrada\n", next->line);
			break;
		case UNTERMINATED_COMMENT:
			printf("Comentário de múltiplas linhas aberto na linha %i e não encerrado\n", next->line);
			break;
		}
		next = next->next;
	}
}

bool is_digit(char c) {
	return isdigit(c) > 0;
}

bool is_reserved_id(char *symbol) {
	int i, cmp;
	
	for (i = 0; i < RESERVED_ID_N; i++) {
		cmp = strcmp(symbol, reserved_id_list[i]);
		
		if (cmp == 0)
			return true;
	}
	
	return false;
}

void print_token(Token token) {
	switch (token.class) {
	case NONE:
		break;
	case DEQ:
		printf("<nome-token: '=='>\n");
		break;
	case NE:
		printf("<nome-token: '~='>\n");
		break;
	case LT:
		printf("<nome-token: '<'>\n");
		break;
	case LE:
		printf("<nome-token: '<='>\n");
		break;
	case GT:
		printf("<nome-token: '>'>\n");
		break;
	case GE:
		printf("<nome-token: '>='>\n");
		break;
	case OP_DELIM:
		printf("<nome-token: '%c'>\n", token.str[0]);
		break;
	case NUM:
		printf("<nome-token: NUM, atributo: %s>\n", token.str);
		break;
	case ID:
		if (is_reserved_id(token.str))
			printf("<nome-token: '%s'>\n", token.str);
		else
			printf("<nome-token: ID, atributo: %s>\n", token.str);
		break;
	case STRING:
		printf("<nome-token: STRING, atributo: %s>\n", token.str);
		break;
	}
}

char *readFile(char *fileName)
{
	FILE *file = fopen(fileName, "r");
	char *code;
	int n = 0;
	int c;

	if(file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	code = malloc(f_size);

	while ((c = fgetc(file))!= EOF)
	{
		code[n++] = (char) c;
	}
	code[n] = '\0';
	return code;
}

StateMachineOutput next_init(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.token = TOKEN_NONE;
	
	if (c == '\n')
		current_line++;
	
	switch(c) {
	case '=':
		out.state = &state_eq;
		break;
	case '<':
		out.state = &state_lt;
		break;
	case '>':
		out.state = &state_gt;
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
		else if (strchr("+*/%(){}[];,:", c) != NULL) {
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
			error_list_insert(INVALID_CHARACTER, start, token_len);
		}
	}
	
	return out;
}

StateMachineOutput next_eq(char *start, int token_len) {
	StateMachineOutput out;
	
	char c = start[token_len - 1];
	
	out.state = (c == '=') ? &state_deq : &state_op_delim;
	out.token = TOKEN_NONE;
	
	return out;
}

StateMachineOutput next_deq(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = DEQ,
		.token.len = 2
	};
	
	return out;
};

StateMachineOutput next_ne_0(char *start, int token_len) {
	StateMachineOutput out;
	
	char c = start[token_len - 1];
	
	if (c != '=') {
		out.state = &state_init;
		error_list_insert(INVALID_TOKEN, start, token_len);
	}
	else {
		out.state = &state_ne_1;
	}
	out.token = TOKEN_NONE;
	
	return out;
}

StateMachineOutput next_ne_1(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = NE,
		.token.len = 2
	};
	
	return out;
};


StateMachineOutput next_lt(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c == '=') ? &state_le : &state_op_delim;
	out.token = TOKEN_NONE;
	
	return out;
}

StateMachineOutput next_le(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = LE,
		.token.len = 2
	};
	
	return out;
};

StateMachineOutput next_gt(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c == '=') ? &state_ge : &state_op_delim;
	out.token = TOKEN_NONE;
	
	return out;
}

StateMachineOutput next_ge(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = GE,
		.token.len = 2
	};
	
	return out;
};

StateMachineOutput next_op_delim(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = OP_DELIM,
		.token.len = 1,
		.token.str = start
	};
	
	return out;
};

StateMachineOutput next_num_0(char *start, int token_len) {
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

StateMachineOutput next_num_1(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = is_digit(c) ? &state_num_1 : &state_num_2;
	out.token = TOKEN_NONE;
	
	return out;
}

StateMachineOutput next_num_2(char *start, int token_len) {
	StateMachineOutput out;
	
	token_len -= 2;
	
	out.token.class = NUM;
	out.token.len = token_len;
	out.token.str = symbol_table_insert(start, out.token.len);
	
	return out;
};

StateMachineOutput next_id_0(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (isalnum(c) || c == '_') ? &state_id_0 : &state_id_1;
	out.token = TOKEN_NONE;
	
	return out;
}

StateMachineOutput next_id_1(char *start, int token_len) {
	StateMachineOutput out;
	
	token_len -= 2;
	
	out.token.class = ID;
	out.token.len = token_len;
	out.token.str = symbol_table_insert(start, out.token.len);
	
	return out;
};

StateMachineOutput next_str_0(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		current_line++;
	
	if (c == '\0') {
		out.state = &state_init;
		error_list_insert(UNTERMINATED_STRING, start, token_len - 1);
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

StateMachineOutput next_str_1(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		current_line++;
	
	if (c == '\0') {
		out.state = &state_init;
		error_list_insert(UNTERMINATED_STRING, start, token_len - 1);
		return out;
	}
	
	if (strchr("abfnrtv\\\"", c) == NULL) {
		out.state = &state_str_3;
		out.token = TOKEN_SKIP;
		error_list_insert(INVALID_ESCAPE_SEQUENCE, start, token_len);
	}
	else {
		out.state = &state_str_0;
		out.token = TOKEN_NONE;
	}

	return out;
}

StateMachineOutput next_str_2(char *start, int token_len) {
	StateMachineOutput out;
	
	token_len -= 1;
	
	out.token.class = STRING;
	out.token.len = token_len;
	out.token.str = symbol_table_insert(start, out.token.len);
	
	return out;
};

StateMachineOutput next_str_3(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		current_line++;
	
	if (c == '\0') {
		out.state = &state_init;
		error_list_insert(UNTERMINATED_STRING, start, token_len - 1);
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

StateMachineOutput next_str_4(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		current_line++;
	
	if (c == '\0') {
		out.state = &state_init;
		error_list_insert(UNTERMINATED_STRING, start, token_len - 1);
		return out;
	}
	
	if (strchr("abfnrtv\\\"", c) == NULL)
		error_list_insert(INVALID_ESCAPE_SEQUENCE, start, token_len);
	
	out.state = &state_str_3;
	out.token = TOKEN_SKIP;

	return out;
}

StateMachineOutput next_cmt_0(char *start, int token_len) {
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

StateMachineOutput next_cmt_1(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n' || c == '\0') { 
		out.state = &state_init;
		current_line++;
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

StateMachineOutput next_cmt_2(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n') {
		out.state = &state_init;
		current_line++;
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

StateMachineOutput next_cmt_3(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		current_line++;
	
	if (c == '\0') {
		out.state = &state_init;
		error_list_insert(UNTERMINATED_COMMENT, start, token_len - 1);
		return out;
	}
	
	out.state = (c != ']') ? &state_cmt_3 : &state_cmt_4;
	out.token = TOKEN_SKIP;
	
	return out;
}

StateMachineOutput next_cmt_4(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		current_line++;
	
	if (c == '\0') {
		out.state = &state_init;
		error_list_insert(UNTERMINATED_COMMENT, start, token_len - 1);
		return out;
	}
	
	out.state = (c != ']') ? &state_cmt_3 : &state_init;
	out.token = TOKEN_SKIP;
	
	return out;
}

StateMachineOutput next_cmt_5(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n') {
		out.state = &state_init;
		current_line++;
	} else {
		out.state = &state_cmt_5;
	}
	out.token = TOKEN_SKIP;
	
	return out;
}

void state_machine_init(void) {
	state_init.next = next_init;
	
	state_eq.next = next_eq;
	
	state_deq.next = next_deq;
	
	state_ne_0.next = next_ne_0;
	state_ne_1.next = next_ne_1;
	
	state_lt.next = next_lt;
	state_le.next = next_le;
	
	state_gt.next = next_gt;
	state_ge.next = next_ge;
	
	state_op_delim.next = next_op_delim;
	
	state_num_0.next = next_num_0;
	state_num_1.next = next_num_1;
	state_num_2.next = next_num_2;
	
	state_id_0.next = next_id_0;
	state_id_1.next = next_id_1;
	
	state_str_0.next = next_str_0;
	state_str_1.next = next_str_1;
	state_str_2.next = next_str_2;
	state_str_3.next = next_str_3;
	state_str_4.next = next_str_4;
	
	state_cmt_0.next = next_cmt_0;
	state_cmt_1.next = next_cmt_1;
	state_cmt_2.next = next_cmt_2;
	state_cmt_3.next = next_cmt_3;
	state_cmt_4.next = next_cmt_4;
	state_cmt_5.next = next_cmt_5;
}

void symbol_table_init(void) {
	for (int i = 0; i < RESERVED_ID_N; i++)
		symbol_table_insert(reserved_id_list[i], strlen(reserved_id_list[i]));
}

StateMachineOutput next_token(char **start)
{
	StateMachineOutput out;
	char c;
	int token_len = 0;
	
	State* current_state = &state_init;
	
	while ((c = (*start)[token_len++]) != '\0' || current_state != &state_init) {
		//printf("\nstart: %c\ncurrent: %c\nlen: %i\n", **start, c, token_len);
		
		out = current_state->next(*start, token_len);
		
		current_state = out.state;
		
		if (out.token.class == SKIP) {
			*start += token_len;
			token_len = 0;
			
			continue;
		}
		
		if (out.token.class != 0) {
			*start += out.token.len;
			
			return out;
		}
	}
	
	out.token.class = NONE;
	return(out);
}

int main(int argc, char *argv[])
{
	state_machine_init();
	symbol_table_init();
	
	StateMachineOutput out;
	
	char *code = readFile(argv[1]);
	
	do {
		out = next_token(&code);
		
		if (error_flag == false)
			print_token(out.token);
	}
	while (out.token.class != NONE);
	
	print_errors();
}
