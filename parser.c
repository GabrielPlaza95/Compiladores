#include "compiler.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TERMINAL_N 48
#define NON_TERMINAL_N 32

#define INITIAL_SYMBOL_STACK_CAP 256

typedef Symbol Terminal;
typedef Symbol Nonterminal;

typedef struct parser Parser;
typedef struct symbol_stack SymbolStack;
typedef void Rule(Parser *parser, Terminal t);

struct symbol_stack {
	int len, cap;
	Symbol *arr;
};

struct parser {
	SymbolStack symbol_stack;
	//ErrorListNode *error_list;

	bool panic_mode_flag;
	int current_line;
};

Parser * parser_init(void);
void parse_token(Parser *parser, Token token);
SymbolStack symbol_stack_init(void);
void symbol_stack_push(Parser *parser, Symbol symbol);
Symbol symbol_stack_peek(Parser *parser);
Symbol symbol_stack_pop(Parser *parser);
char * read_file(char *file_name);
bool is_terminal(Symbol symbol);

int main(int argc, char *argv[])
{
	Token token;
	Lexer *lexer = lexer_init();
	Parser *parser = parser_init();

	if (argv[1] == NULL) {
		fprintf(stderr, "Erro: arquivo para leitura deve ser informado\n");
		exit(-1);
	}

	char *code = read_file(argv[1]);
	
	if (code == NULL) {
		fprintf(stderr, "Erro: arquivo não encontrado\n");
		exit(-1);
	}

	do {
		token = next_token(lexer, &code);
		parse_token(parser, token);

		if (!error_detected(lexer))
			print_token(token);
	}
	while (token.class != NONE);

	print_errors(lexer);
}

Parser * parser_init(void)
{
	Parser *parser = malloc(sizeof *parser);
	
	parser->symbol_stack = symbol_stack_init();
	//parser->error_list = NULL;

	parser->panic_mode_flag = false;
	parser->current_line = 1;

	return parser;
}


void parse_token(Parser *parser, Token token)
{
	Symbol symbol = symbol_stack_peek(parser);

	if (is_terminal(symbol) || symbol == END) {
		if (symbol == token.class)
			symbol_stack_pop(parser);
		else
			printf("Parse error!\n");
			//parse_error();
		return;
	}

	// Rule *rule = parser_table(symbol);
 // 
	// rule(parser, token.class);

// 	if (rule != NULL) {
// 		symbol_stack_pop(parser);
// 		
// 		Symbol *rhs = rule.rhs;
// 		
// 		push_rule_rhs(rhs);
// 		
// 		write_rule(rule);
// 	}
// 	else {
// 		parse_error();
// 	}
}

SymbolStack symbol_stack_init(void)
{
	SymbolStack stack;
	
	stack.cap = INITIAL_SYMBOL_STACK_CAP;
	stack.arr = malloc(stack.cap * sizeof *stack.arr);
	
	stack.arr[0] = NONE;
	stack.arr[1] = BLOCK;
	stack.len = 2;
	
	return stack;
}

void symbol_stack_push(Parser *parser, Symbol symbol)
{
	SymbolStack stack;
	
	stack = parser->symbol_stack;
	
	while (stack.len >= stack.cap) {
		stack.cap *= 2;
		stack.arr = realloc(stack.arr, stack.cap * sizeof *stack.arr);
	}
	
	stack.arr[stack.len++] = symbol;
}

Symbol symbol_stack_peek(Parser *parser)
{
	SymbolStack stack;
	
	stack = parser->symbol_stack;
	
	return stack.arr[stack.len - 1];
}

Symbol symbol_stack_pop(Parser *parser)
{
	SymbolStack stack;
	
	stack = parser->symbol_stack;
	
	return stack.arr[--stack.len];
}

bool is_terminal(Symbol sym) {
	return sym >= BLOCK;
}

char * read_file(char *file_name)
{
	FILE *file = fopen(file_name, "r");
	long f_size;
	char *code, *cur;

	if (file == NULL)
		return NULL;

	fseek(file, 0, SEEK_END);
	f_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	code = malloc(f_size);
	cur = code;

	while ((*cur++ = fgetc(file)) != EOF);
	*--cur = '\0';

	return code;
}
