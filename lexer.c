#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define TOKEN_NULL (Token) { 0 }
#define TOKEN_SKIP (Token) { SKIP, 0 }

enum Tokens {
	SKIP = -1,
	STRING = 258,
	OPDELIM = 259,
	ID = 260,
	NUM = 261,
};

typedef struct {
	int nome_token;
	int atributo;
	int len;
	char str[20];
	char *sym;
} Token;

enum Atributos {
	LT = 262,
	LE = 263,
	EQ = 264,
	NE = 265,
	GT = 266,
	GE = 267
};

typedef struct snode {
	struct snode *left;
	struct snode *right;
	char *symbol;
} symtable_node;

symtable_node *symbol_table;

char *symtable_insert(char *symbol) {
	symtable_node *next = symbol_table;
	
	if (next->symbol == NULL)
		return symbol;
	
	do {
		int cmp = strcmp(symbol, next->symbol);
		
		if (cmp == 0) {
			free(symbol);
			return(next->symbol);
		}
		next = (cmp < 0) ? next->left : next->right;
	}
	while (next != NULL);
	
	next = malloc(sizeof *next);
	next->left = NULL;
	next->right = NULL;
	next->symbol = symbol;
	
	return next->symbol;
}

bool is_digit(char c) {
	return isdigit(c) > 0;
}

void print_token(Token token) {
	puts(token.str);
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

typedef struct state {
	Token (*next)(struct state **state, char *start, int token_len);
} State;

State
	state_init,
	
	state_eq_0,
	state_eq_1,
	state_deq,
	
	state_ne_0,
	state_ne_1,
	
	state_lt_0,
	state_lt_1,
	state_le,
	
	state_gt_0,
	state_gt_1,
	state_ge,
	
	state_opdelim,
	
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
	state_cmt_5
;

Token next_init(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	switch(c) {
	case '=':
		*state = &state_eq_0;
		break;
	case '<':
		*state = &state_lt_0;
		break;
	case '>':
		*state = &state_gt_0;
		break;
	case '~':
		*state = &state_ne_0;
		break;
	case '"':
		*state = &state_str_0;
		break;
	case '-':
		*state = &state_cmt_0;
		break;
	default:
		if (isspace(c)) {
			*state = &state_init;
			return TOKEN_SKIP;
		}
		else if (strchr("+*/%(){}[];,:", c) != NULL) {
			*state = &state_opdelim;
		}
		else if (is_digit(c)) {
			*state = &state_num_0;
		}
		else if (isalpha(c) || c == '_') {
			*state = &state_id_0;
		}
		else {
			*state = &state_init;
		}
	}
	
	return TOKEN_NULL;
}

Token next_eq_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c == '=') ? &state_deq : &state_eq_1;
	
	return TOKEN_NULL;
}

Token next_eq_1(State **state, char *start, int token_len) {
	Token token = {
		.nome_token = OPDELIM,
		.atributo = '=',
		.str = "<OPDELIM, = >\n",
		.len = 1
	};
	
	return token;
};

Token next_deq(State **state, char *start, int token_len) {
	Token token = {
		.nome_token = OPDELIM,
		.atributo = EQ,
		.str = "<OPDELIM, == >\n",
		.len = 2
	};
	
	return token;
};

Token next_ne_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c == '=') ? &state_ne_1 : &state_init; // ERRO
	
	return TOKEN_NULL;
}

Token next_ne_1(State **state, char *start, int token_len) {
	Token token = {
		.nome_token = OPDELIM,
		.atributo = NE,
		.str = "<OPDELIM, ~= >\n",
		.len = 2
	};
	
	return token;
};


Token next_lt_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c == '=') ? &state_le : &state_lt_1;
	
	return TOKEN_NULL;
}

Token next_lt_1(State **state, char *start, int token_len) {
	Token t = {
		.nome_token = OPDELIM,
		.atributo = LT,
		.str = "<OPDELIM, < >\n",
		.len = 1
	};
	
	return t;
};

Token next_le(State **state, char *start, int token_len) {
	return (Token) {
		.nome_token = OPDELIM,
		.atributo = LE,
		.str = "<OPDELIM, <= >\n",
		.len = 2
	};
};

Token next_gt_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c == '=') ? &state_ge : &state_gt_1;
	
	return TOKEN_NULL;
}

Token next_gt_1(State **state, char *start, int token_len) {
	return (Token)  {
		.nome_token = OPDELIM,
		.atributo = GT,
		.str = "<OPDELIM, > >\n",
		.len = 1
	};
};

Token next_ge(State **state, char *start, int token_len) {
	return (Token) {
		.nome_token = OPDELIM,
		.atributo = GE,
		.str = "<OPDELIM, >= >\n",
		.len = 2
	};
};

Token next_opdelim(State **state, char *start, int token_len) {
	Token token = {
		.nome_token = OPDELIM,
		.atributo = start[0],
		.len = 1
	};
	
	sprintf(token.str ,"<OPDELIM, %c >\n", token.atributo);
	
	return token;
};

Token next_num_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	if (is_digit(c))
		*state = &state_num_0;
	else if (c == '.')
		*state = &state_num_1;
	else
		*state = &state_num_2;
	
	return TOKEN_NULL;
}

Token next_num_1(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (is_digit(c)) ? &state_num_1 : &state_num_2;
	
	return TOKEN_NULL;
}

Token next_num_2(State **state, char *start, int token_len) {
	Token token;
	
	token_len -= 2;
	
	token.nome_token = NUM;
	token.atributo = 0;
	token.sym = malloc(token_len + 1);
	token.len = token_len;

	strncpy(token.sym, start, token_len);
	token.sym[token_len + 1] = '\0';
	
	token.sym = symtable_insert(token.sym);
	
	sprintf(token.str ,"<NUM, %s >\n", token.sym);
	
	return token;
};

Token next_id_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (isalnum(c) || c == '_') ? &state_id_0 : &state_id_1;
	
	return TOKEN_NULL;
}

Token next_id_1(State **state, char *start, int token_len) {
	Token token;
	
	token_len -= 2;
	
	token.nome_token = ID;
	token.atributo = 0;
	token.sym = malloc(token_len + 1);
	token.len = token_len;

	strncpy(token.sym, start, token_len);
	token.sym[token_len + 1] = '\0';
	
	token.sym = symtable_insert(token.sym);
	
	sprintf(token.str ,"<ID, %s >\n", token.sym);
	
	return token;
};

Token next_str_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	if (c == '"')
		*state = &state_str_2;
	else if (c == '\\')
		*state = &state_str_1;
	else
		*state = &state_str_0;
	
	return TOKEN_NULL;
}

Token next_str_1(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (strchr("abfnrtv\\", c) != NULL) ? &state_str_0 : &state_init; // ERRO
	
	return TOKEN_NULL;
}

Token next_str_2(State **state, char *start, int token_len) {
	Token token;
	
	token_len -= 1;
	
	token.nome_token = STRING;
	token.atributo = 0;
	token.sym = malloc(token_len + 1);
	token.len = token_len;

	strncpy(token.sym, start, token_len);
	token.sym[token_len + 1] = '\0';
	
	token.sym = symtable_insert(token.sym);
	
	sprintf(token.str ,"<STRING, %s >\n", token.sym);
	
	return token;
};

Token next_cmt_0(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	if (c != '-') {
		*state = &state_opdelim;
		return TOKEN_NULL;
	}
	
	*state = &state_cmt_1;
	return TOKEN_SKIP;
}

Token next_cmt_1(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	if (c == '\n')
		*state = &state_init;
	else if (c == '[')
		*state = &state_cmt_2;
	else
		*state = &state_cmt_5;
	
	return TOKEN_SKIP;
}

Token next_cmt_2(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	if (c == '\n')
		*state = &state_init;
	else if (c == '[')
		*state = &state_cmt_3;
	else
		*state = &state_cmt_5;
	
	return TOKEN_SKIP;
}

Token next_cmt_3(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c != ']') ? &state_cmt_3 : &state_cmt_4;
	
	return TOKEN_SKIP;
}

Token next_cmt_4(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c != ']') ? &state_cmt_3 : &state_init;
	
	return TOKEN_SKIP;
}

Token next_cmt_5(State **state, char *start, int token_len) {
	char c = start[token_len - 1];
	
	*state = (c != '\n') ? &state_cmt_5 : &state_init;
	
	return TOKEN_SKIP;
}


void state_machine_init(void) {
	state_init.next = next_init;
	
	state_eq_0.next = next_eq_0;
	state_eq_1.next = next_eq_1;
	state_deq.next = next_deq;
	
	state_ne_0.next = next_ne_0;
	state_ne_1.next = next_ne_1;
	
	state_lt_0.next = next_lt_0;
	state_lt_1.next = next_lt_1;
	state_le.next = next_le;
	
	state_gt_0.next = next_gt_0;
	state_gt_1.next = next_gt_1;
	state_ge.next = next_ge;
	
	state_opdelim.next = next_opdelim;
	
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

Token proximo_token(char **start)
{
	Token token;
	char c;
	int token_len = 0;
	
	State* estado = &state_init;
	
	while ((c = (*start)[token_len++]) != '\0' || estado != &state_init) {
		//printf("\nstart: %c\ncursor: %c\nlen: %i\n", **start, c, token_len);
		
		token = estado->next(&estado, *start, token_len);
		
		if (token.nome_token == SKIP) {
			*start += token_len;
			token_len = 0;
			
			continue;
		}
		
		if (token.nome_token != 0) {
			*start += token.len;
			
			return token;
		}
	}
	
	token.nome_token = EOF;
	token.atributo = -1;
	return(token);
}

int main ()
{
	symbol_table = malloc(sizeof (symtable_node));
	
	state_machine_init();
	
	Token token;
	
	char *code = readFile("programa.txt");
	
	do {
		token = proximo_token(&code);
		print_token(token);
	}
	while (token.nome_token != EOF);
}
