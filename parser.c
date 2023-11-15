#include "compiler.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TERMINAL_N 48
#define NON_TERMINAL_N 32

#define INITIAL_SYMBOL_STACK_CAP 256

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))

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
void panic(Parser *parser);
SymbolStack symbol_stack_init(void);
Symbol symbol_stack_peek(Parser *parser);
Symbol symbol_stack_pop(Parser *parser);
void symbol_stack_push(Parser *parser, Symbol symbol);
void symbol_stack_print(Parser *parser);
char * read_file(char *file_name);
bool is_terminal(Symbol symbol);
bool is_in_array(Symbol symbol, Symbol *arr, int n);


Rule rule_block;
Rule rule_stmt;
Rule rule_elsestmt;
Rule rule_localdecl;
Rule rule_fndecl;
Rule rule_exps;
Rule rule_exp;
Rule rule_exps_;
Rule rule_exp_;
Rule rule_loopexp;
Rule rule_loopexp_;
Rule rule_returnexp;
Rule rule_table;
Rule rule_fields;
Rule rule_field;
Rule rule_fields_;
Rule rule_binop;
Rule rule_vars;
Rule rule_var;
Rule rule_vars_;
Rule rule_fncall;
Rule rule_indexexp;
Rule rule_callexp;
Rule rule_prefixexp;
Rule rule_suffixexp;
Rule rule_fnbody;
Rule rule_fnexp;
Rule rule_fnparams;
Rule rule_args;
Rule rule_args_;
Rule rule_names;
Rule rule_names_;

Rule *parser_table[32] = {
	rule_block,
	rule_stmt,
	// rule_elsestmt,
	// rule_localdecl,
	// rule_fndecl,
	// rule_exps,
	// rule_exp,
	// rule_exps_,
	// rule_exp_,
	// rule_loopexp,
	// rule_loopexp_,
	// rule_returnexp,
	// rule_table,
	// rule_fields,
	// rule_field,
	// rule_fields_,
	// rule_binop,
	// rule_vars,
	// rule_var,
	// rule_vars_,
	// rule_fncall,
	// rule_indexexp,
	// rule_callexp,
	// rule_prefixexp,
	// rule_suffixexp,
	// rule_fnbody,
	// rule_fnexp,
	// rule_fnparams,
	// rule_args,
	// rule_args_,
	// rule_names,
	// rule_names_
};

int main(int argc, char *argv[])
{
	Symbol symbol;
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
	
	token = next_token(lexer, &code);
	
	do {
		symbol = symbol_stack_peek(parser);
		
		symbol_stack_print(parser);
		print_token(token);
		//printf("%s\n", code);

		if (is_terminal(symbol) || symbol == NONE) {
			if (symbol == token.class) {
				symbol_stack_pop(parser);
				token = next_token(lexer, &code);
			}
			else {
				panic(parser);
			}
			continue;
		}
		
		if (parser->panic_mode_flag == true)
			token = next_token(lexer, &code);

		Rule *rule = parser_table[symbol - BLOCK];
		rule(parser, token.class);
	}
	while (symbol != NONE && token.class != NONE);

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


void panic(Parser *parser)
{
	Symbol symbol;
	
	parser->panic_mode_flag = true;
	
	symbol = symbol_stack_peek(parser);
	
	while (is_terminal(symbol)) {
		symbol_stack_pop(parser);
		symbol = symbol_stack_peek(parser);
	}
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
	SymbolStack *stack;
	
	stack = &parser->symbol_stack;
	
	while (stack->len >= stack->cap) {
		stack->cap *= 2;
		stack->arr = realloc(stack->arr, stack->cap * sizeof *stack->arr);
	}
	
	stack->arr[stack->len++] = symbol;
}

Symbol symbol_stack_peek(Parser *parser)
{
	SymbolStack *stack;
	
	stack = &parser->symbol_stack;
	
	return stack->arr[stack->len - 1];
}

Symbol symbol_stack_pop(Parser *parser)
{
	SymbolStack *stack;
	
	stack = &parser->symbol_stack;
	
	return stack->arr[--stack->len];
}

void symbol_stack_print(Parser *parser)
{
	SymbolStack *stack;
	
	stack = &parser->symbol_stack;
	
	for (int i = 0; i < stack->len; i++)
		printf("%d ", stack->arr[i]);
	
	puts("\n");
}

bool is_terminal(Symbol symbol)
{
	return symbol < BLOCK && symbol > NONE;
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

bool is_in_array(Symbol symbol, Symbol *arr, int n)
{
	for (int i = 0; i < n; i++)
		if (symbol == arr[i])
			return true;
	
	return false;
}

void rule_block(Parser *parser, Terminal t)
{
	Terminal first[] = { DO, WHILE, BREAK, IF, FOR, LOCAL, RETURN, ID, FUNCTION, LPAREN };
	Terminal follow[] = { NONE, ELSEIF, ELSE, END };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		symbol_stack_push(parser, BLOCK);
		symbol_stack_push(parser, SEMICOLON);
		symbol_stack_push(parser, STMT);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		// recover
		parser->panic_mode_flag = false;
	}
	else {
		panic(parser);
	}
}
void rule_stmt(Parser *parser, Terminal t)
{
	Terminal first[] = { DO, WHILE, BREAK, IF, FOR, LOCAL, RETURN, ID, FUNCTION, LPAREN };
	Terminal follow[] = { SEMICOLON };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		switch (t) {
		case DO:
			symbol_stack_push(parser, END);
			symbol_stack_push(parser, BLOCK);
			symbol_stack_push(parser, DO);
			break;
		case WHILE:
			symbol_stack_push(parser, END);
			symbol_stack_push(parser, BLOCK);
			symbol_stack_push(parser, DO);
			symbol_stack_push(parser, EXP);
			symbol_stack_push(parser, WHILE);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser); // Correto?
		
		parser->panic_mode_flag = false;
		// Travando aqui
	}
	else {
		panic(parser);
	}
}
// void rule_elsestmt(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_localdecl(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fndecl(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_exps(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_exp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_exps_(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_exp_(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_loopexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_loopexp_(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_returnexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_table(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fields(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_field(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fields_(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_binop(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_vars(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_var(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_vars_(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fncall(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_indexexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_callexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_prefixexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_suffixexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fnbody(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fnexp(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_fnparams(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_args(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_args_(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_names(Parser *parser, Terminal t)
// {
// 	
// }
// void rule_names_(Parser *parser, Terminal t)
// {
// 	
// }
