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

typedef struct {
	int class;
	int len;
	char *str;
}
Token;

struct state;

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

State
	state_init,
	
	state_eq,
	
	state_deq,
	
	state_ne_0,
	state_ne_1,
	
	state_lt_0,
	state_lt_1,
	state_le,
	
	state_gt_0,
	state_gt_1,
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
	
	state_cmt_0,
	state_cmt_1,
	state_cmt_2,
	state_cmt_3,
	state_cmt_4,
	state_cmt_5;

SymbolTableNode *symbol_table = NULL;

//char erro[] = "Token inválido: ";

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

char *symbol_table_insert(char *symbol, int len) {
	SymbolTableNode *next = symbol_table;
	
	if (next == NULL) {
		next = malloc(sizeof *next);
		next->left = NULL;
		next->right = NULL;
	}
	
	while (next != NULL && next->symbol != NULL) {
		int cmp = strncmp(symbol, next->symbol, len);
		int next_len = strlen(next->symbol);
		
		if (cmp == 0 && len == next_len)
			return(next->symbol);
	
		next = ((cmp == 0 && len < next_len) || cmp < 0 ) ? next->left : next->right;
	}
	
	next->symbol = malloc(len + 1);
	next->symbol[len + 1] = '\0';
	
	strncpy(next->symbol, symbol, len);
	
	return next->symbol;
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
		printf("<nome-token: '==', atributo: >\n");
		break;
	case NE:
		printf("<nome-token: '~=', atributo: >\n");
		break;
	case LT:
		printf("<nome-token: '<', atributo: >\n");
		break;
	case LE:
		printf("<nome-token: '<=', atributo: >\n");
		break;
	case GT:
		printf("<nome-token: '>', atributo: >\n");
		break;
	case GE:
		printf("<nome-token: '>=', atributo: >\n");
		break;
	case OP_DELIM:
		printf("<nome-token: '%c', atributo: >\n", token.str[0]);
		break;
	case NUM:
		printf("<nome-token: NUM, atributo: %s>\n", token.str);
		break;
	case ID:
		if (is_reserved_id(token.str))
			printf("<nome-token: '%s', atributo: >\n", token.str);
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
	
	out.token = TOKEN_NONE;
	
	char c = start[token_len - 1];
	
	switch(c) {
	case '=':
		out.state = &state_eq;
		break;
	case '<':
		out.state = &state_lt_0;
		break;
	case '>':
		out.state = &state_gt_0;
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
		}
	}
	
	return out;
}

StateMachineOutput next_error(char *start, int token_len) {
	StateMachineOutput out;
	
	out.state = &state_init;
	out.token = TOKEN_SKIP;
	
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
	
	out.state = (c == '=') ? &state_ne_1 : &state_init; //ERRO
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


StateMachineOutput next_lt_0(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c == '=') ? &state_le : &state_op_delim;
	
	out.token = TOKEN_NONE;
	return out;
}

StateMachineOutput next_lt_1(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = LT,
		.token.len = 1
	};
	
	return out;
};

StateMachineOutput next_le(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = LE,
		.token.len = 2
	};
	
	return out;
};

StateMachineOutput next_gt_0(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c == '=') ? &state_ge : &state_gt_1;
	
	out.token = TOKEN_NONE;
	return out;
}

StateMachineOutput next_gt_1(char *start, int token_len) {
	StateMachineOutput out = {
		.token.class = GT,
		.token.len = 1
	};
	
	return out;
};

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
	
	out.state = (strchr("abfnrtv\\", c) != NULL) ? &state_str_0 : &state_init; // ERRO
	out.token = TOKEN_NONE;
	
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
	
	if (c == '\n')
		out.state = &state_init;
	else if (c == '[')
		out.state = &state_cmt_2;
	else
		out.state = &state_cmt_5;
	
	out.token = TOKEN_SKIP;
	
	return out;
}

StateMachineOutput next_cmt_2(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	if (c == '\n')
		out.state = &state_init;
	else if (c == '[')
		out.state = &state_cmt_3;
	else
		out.state = &state_cmt_5;
	
	out.token = TOKEN_SKIP;
	
	return out;
}

StateMachineOutput next_cmt_3(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c != ']') ? &state_cmt_3 : &state_cmt_4;
	
	out.token = TOKEN_SKIP;
	
	return out;
}

StateMachineOutput next_cmt_4(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c != ']') ? &state_cmt_3 : &state_init;
	
	out.token = TOKEN_SKIP;
	
	return out;
}

StateMachineOutput next_cmt_5(char *start, int token_len) {
	StateMachineOutput out;
	char c = start[token_len - 1];
	
	out.state = (c != '\n') ? &state_cmt_5 : &state_init;
	
	out.token = TOKEN_SKIP;
	
	return out;
}


void state_machine_init(void) {
	state_init.next = next_init;
	
	state_eq.next = next_eq;
	
	state_deq.next = next_deq;
	
	state_ne_0.next = next_ne_0;
	state_ne_1.next = next_ne_1;
	
	state_lt_0.next = next_lt_0;
	state_lt_1.next = next_lt_1;
	state_le.next = next_le;
	
	state_gt_0.next = next_gt_0;
	state_gt_1.next = next_gt_1;
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

StateMachineOutput proximo_token(char **start)
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

int main ()
{
	state_machine_init();
	symbol_table_init();
	
	StateMachineOutput out;
	
	char *code = readFile("programa.txt");
	
	do {
		out = proximo_token(&code);
		print_token(out.token);
	}
	while (out.token.class != NONE);
}
