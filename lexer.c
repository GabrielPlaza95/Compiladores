#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
} Token;

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
	
	state_opdelim
;

State* next_final(char c) {
	printf("Final");
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
}

Token proximo_token(char **cursor, State *estado)
{
	Token token;
	char c;
	
	while ((c = *(*cursor)++) != '\0') {
		printf("char: %c\n", c);
		//puts(*cursor);
		
		estado = estado->next(c);
		
		token = estado->get_token(c);
		
		if (token.nome_token != 0) return token;
	}
	
	token.nome_token = EOF;
	token.atributo = -1;
	return(token);
}

int main ()
{
	state_init_machine();
	
	Token token;
	State* estado = &state_init;
	
	char *code = readFile("programa.txt");
	
	do {
		token = proximo_token(&code, estado);
		print_token(token);
	}
	while (token.nome_token != EOF);
}
