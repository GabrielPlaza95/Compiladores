#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef struct snode {
	struct snode *left;
	struct snode *right;
	char *symbol;
} symtable_node;

typedef symtable_node *symtable;

symtable symtable_create(void) {
	return malloc(sizeof (symtable_node));
}

char *symtable_insert(symtable root, char *symbol) {
	symtable_node *next = root;
	
	do {
		int cmp = strcmp(symbol, next->symbol);
		
		if (cmp < 0)
			next = next->left;
		else if (cmp < 0)
			next = next->right;
		else
			return next->symbol;
		
	} while (next != NULL);
		
	
	next = malloc(sizeof *next);
	next->left = NULL;
	next->right = NULL;
	next->symbol = symbol;
	
	return next->symbol;
}



enum Tokens {
	STRING = 258,
	OPDELIM = 259,
	ID = 260,
	NUM = 261
};

enum Atributos {
	LT = 262,
	LE = 263,
	EQ = 264,
	NE = 265,
	GT = 266,
	GE = 267
};

typedef struct {
	int nome_token;
	int atributo;
	char str[20];
	char *sym;
} Token;

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
	char *name;
	struct state *(*next)(char c);
	Token (*get_token)(char c);
} State;

symtable symbol_table;

State
	state_init,
	
	state_eq_0,
	state_eq_1,
	state_deq,
	
	state_lt_0,
	state_lt_1,
	state_le,
	
	state_gt_0,
	state_gt_1,
	state_ge,
	
	state_opdelim,
	
	state_num_0,
	state_num_1,
	state_num_2
;

State* next_final(char c) {
	return &state_init;
}

State *next_init(char c) {
	switch(c) {
	case ' ':
	case '\n':
		return &state_init;
	case '=':
		return &state_eq_0;
	case '<':
		return &state_lt_0;
	case '>':
		return &state_gt_0;
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '^':
	case '(':
	case ')':
	case '{':
	case '}':
	case '[':
	case ']':
	case ';':
	case ',':
	case ':':
		return &state_opdelim;
	default:
		if (is_digit(c))
			return &state_num_0;
	}
	
	return &state_init;
}

State *next_eq_0(char c) {
	if (c == '=')
		return &state_deq;
	
	return &state_eq_1;
}

State *next_lt_0(char c) {
	if (c == '=')
		return &state_le;
	
	return &state_lt_1;
}

State *next_gt_0(char c) {
	if (c == '=')
		return &state_ge;
	
	return &state_gt_1;
}

State *next_num_0(char c) {
	if (is_digit(c))
		return &state_num_0;
	if (c == '.')
		return &state_num_1;
	
	return &state_num_2;
}

State *next_num_1(char c) {
	if (is_digit(c))
		return &state_num_1;
	
	return &state_num_2;
}


Token get_token_nop(char c) {
	Token token = { 0, 0 };
	
	return token; 
}

Token get_token_eq_1(char c) {
	Token token = {
		.nome_token = OPDELIM,
		.atributo = '=',
		.str = "<OPDELIM, = >\n"
	};
	
	return token;
};

Token get_token_deq(char c) {
	return (Token) {
		.nome_token = OPDELIM,
		.atributo = EQ,
		.str = "<OPDELIM, == >\n"
	};
};

Token get_token_lt_1(char c) {
	return (Token)  {
		.nome_token = OPDELIM,
		.atributo = LT,
		.str = "<OPDELIM, < >\n"
	};
};

Token get_token_le(char c) {
	return (Token) {
		.nome_token = OPDELIM,
		.atributo = LE,
		.str = "<OPDELIM, <= >\n"
	};
};

Token get_token_gt_1(char c) {
	return (Token) {
		.nome_token = OPDELIM,
		.atributo = GT,
		.str = "<OPDELIM, > >\n"
	};
};

Token get_token_ge(char c) {
	return (Token) {
		.nome_token = OPDELIM,
		.atributo = GE,
		.str = "<OPDELIM, >= >\n"
	};
};

Token get_token_opdelim(char c) {
	Token token = {
		.nome_token = OPDELIM,
		.atributo = c
	};
	
	sprintf(token.str ,"<OPDELIM, %c >\n", c);
	
	return token;
};

Token get_token_num(char c) {
	return (Token) {
		.nome_token = NUM,
		.atributo = 0,
		.str = "<NUM,  >\n"
	};
};

void state_init_machine(void) {
	state_init.next = next_init;
	state_init.get_token = get_token_nop;
	
	state_eq_0.next = next_eq_0;
	state_eq_0.get_token = get_token_nop;
	
	state_eq_1.next = next_final;
	state_eq_1.get_token = get_token_eq_1;
	
	state_deq.next = next_final;
	state_deq.get_token = get_token_deq;
	
	state_lt_0.next = next_lt_0;
	state_lt_0.get_token = get_token_nop;
	
	state_lt_1.next = next_final;
	state_lt_1.get_token = get_token_lt_1;
	
	state_le.next = next_final;
	state_le.get_token = get_token_le;
	
	state_gt_0.next = next_gt_0;
	state_gt_0.get_token = get_token_nop;
	
	state_gt_1.next = next_final;
	state_gt_1.get_token = get_token_gt_1;
	
	state_ge.next = next_final;
	state_ge.get_token = get_token_ge;
	
	state_opdelim.next = next_final;
	state_opdelim.get_token = get_token_opdelim;
	
	state_num_0.next = next_num_0;
	state_num_0.get_token = get_token_nop;
	
	state_num_1.next = next_num_1;
	state_num_1.get_token = get_token_nop;
	
	state_num_2.next = next_final;
	state_num_2.get_token = get_token_num;
}

Token proximo_token(char **start)
{
	Token token;
	char c;
	char *cursor = *start;
	int token_len = 0;
	
	State* estado = &state_init;
	
	while ((c = *cursor++) != '\0') {
		// problema: números conferem um caracter após o último
		
		//printf("start: %c\ncursor: %c\n", **start, c);
		//puts(cursor);
		//printf("start: %p\ncursor: %p\n", *start, cursor);
		
		estado = estado->next(c);
		token = estado->get_token(c);
		
		token_len++;
		
		
		
		if (token.nome_token != 0) {
			int n = (estado == &state_num_2) ? -1 : 0;
			
			char * str = malloc(token_len + n + 1);
			strncpy(str, *start, token_len + n + 1);
			
			token.sym = symtable_insert(symbol_table, str);
			
			*start = cursor + n;
			
			return token;
		}
	}
	
	token.nome_token = EOF;
	token.atributo = -1;
	return(token);
}

int main ()
{
	symbol_table = symtable_create();
	
	state_init_machine();
	
	Token token;
	
	char *code = readFile("programa.txt");
	
	do {
		token = proximo_token(&code);
		print_token(token);
	}
	while (token.nome_token != EOF);
}
