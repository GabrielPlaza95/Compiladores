#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum Tokens {
	IF = 256,
	THEN = 257,
	ELSE = 258,
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
} Token;

int estado = 0;
int partida = 0;
int cont_sim_lido = 0;

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
	bool (*finished)(void);
	Token (*get_token)(void);
} State;

State
	init_state,
	
	state_eq_0,
	state_eq_1,
	state_deq,
	
	state_lt_0,
	state_lt_1,
	state_le,
	
	state_gt_0,
	state_gt_1,
	state_ge
;

State* next_final(char c) {
	return &init_state;
}

State *next_init(char c) {
	switch(c) {
	case ' ':
	case '\n':
		return &init_state;
	case '=':
		return &state_eq_0;
	case '<':
		return &state_lt_0;
	case '>':
		return &state_gt_0;
	}
	
	return &init_state;
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
		return &state_le;
	
	return &state_lt_1;
}

Token get_token_nop(void) {
	Token token = { 0, 0 };
	
	return token; 
}

Token get_token_eq_1(void) {
	printf("<OPDELIM, =>\n");
	
	Token token = {
		.nome_token = OPDELIM,
		.atributo = '='
	};
	
	return token;
};

Token get_token_deq(void) {
	printf("<OPDELIM, EQ>\n");
	
	Token token = {
		.nome_token = OPDELIM,
		.atributo = EQ
	};
	
	return token;
};

Token get_token_lt_1(void) {
	printf("<OPDELIM, <>\n");
	
	Token token = {
		.nome_token = OPDELIM,
		.atributo = LT
	};
	
	return token;
};

Token get_token_le(void) {
	printf("<OPDELIM, <>\n");
	
	Token token = {
		.nome_token = OPDELIM,
		.atributo = LE
	};
	
	return token;
};

void init_state_machine(void) {
	init_state.next = next_init;
	init_state.get_token = get_token_nop;
	
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
}

Token proximo_token(char **cursor, State *estado)
{
	Token token;
	char c;
	
	while ((c = *(*cursor)++) != '\0') {
		printf("%c\n", c);
		//puts(*cursor);
		
		token = estado->get_token();
		
		if (token.nome_token != 0) {
			return token;
		}
		
		State* novo_estado = estado->next(c);
		
		estado = novo_estado;
	}
	
	token.nome_token = EOF;
	token.atributo = -1;
	return(token);
}

int main ()
{
	init_state_machine();
	
	Token token;
	State* estado = &init_state;
	
	char *code = readFile("programa.txt");
	
	do {
		token = proximo_token(&code, estado);
	}
	while (token.nome_token != EOF);
}
