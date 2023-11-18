#include "compiler.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#define TERMINAL_N 46
#define NON_TERMINAL_N 26
#define INITIAL_SYMBOL_STACK_CAP 256

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))
#define symbol_stack_push_rule(parser, ...) symbol_stack_push_rule_n(parser, sizeof ((Symbol[]) {__VA_ARGS__}) / sizeof (Symbol), __VA_ARGS__)

typedef Symbol Terminal;
typedef Symbol Nonterminal;

typedef struct parser Parser;
typedef struct symbol_stack SymbolStack;
typedef struct parser_error_list_node ParserErrorListNode;
typedef ParserErrorListNode ParserErrorList;
typedef void Rule(Parser *parser, Terminal t);

struct symbol_stack {
	int len, cap;
	Symbol *arr;
};

struct parser_error_list_node {
	int line;
	ParserErrorListNode *next;
	Symbol received;
	int expected_len;
	Symbol *expected;
};

struct parser {
	SymbolStack symbol_stack;
	ParserErrorList *error_list;

	bool panic_mode_flag;
	bool error_flag;
	int current_line;
};

Parser * parser_init(void);
void panic(Parser *parser);
SymbolStack symbol_stack_init(void);
Symbol symbol_stack_peek(Parser *parser);
Symbol symbol_stack_pop(Parser *parser);
void symbol_stack_push(Parser *parser, Symbol symbol);
void symbol_stack_push_rule_n(Parser *parser, int len,  ...);
void symbol_stack_push_rule_v(Parser *parser, int len, va_list args);
void symbol_stack_print(Parser *parser);
char * read_file(char *file_name);
bool is_terminal(Symbol symbol);
bool is_in_array(Symbol symbol, Symbol *arr, int n);
bool parser_error_detected(Parser *parser);
void parser_error_list_insert(Parser *parser, Symbol expected, int received_len, Symbol *received);
void parser_error_list_print(Parser *parser);


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
Rule rule_vars_;
Rule rule_var;
Rule rule_var_;
Rule rule_indexexp;
Rule rule_suffixexp;
Rule rule_suffixexp_;
Rule rule_fnbody;
Rule rule_fnexp;
Rule rule_fnparams;
Rule rule_names;
Rule rule_names_;

char *symbol_list[] = {
	"$", "=", "<", ">", "+", "-", "*", "/", "%", "^",
	"(", ")", "[", "]", "{", "}", ",", ";",
	"==", "~=", "<=", ">=", "..", "<num>", "<string>", "<name>",
	"do", "if", "in", "or", "and", "end", "for", "nil", "not",
	"else", "then", "true", "break", "false", "local", "until", "while", "elseif", "repeat", "return", "function", 

	"BLOCK", "STMT", "ELSESTMT", "LOCALDECL", "FNDECL", "EXPS", "EXP", "EXPS'", "EXP'", "LOOPEXP", "LOOPEXP'", "RETURNEXP",
	"TABLE", "FIELDS", "FIELD", "FIELDS'", "BINOP", "VARS", "VARS'", "VAR", "VAR'", "FNBODY", "FNEXP", "FNPARAMS", "NAMES", "NAMES'"
};

Rule *parser_table[NON_TERMINAL_N] = {
	rule_block,
	rule_stmt,
	rule_elsestmt,
	rule_localdecl,
	rule_fndecl,
	rule_exps,
	rule_exp,
	rule_exps_,
	rule_exp_,
	rule_loopexp,
	rule_loopexp_,
	rule_returnexp,
	rule_table,
	rule_fields,
	rule_field,
	rule_fields_,
	rule_binop,
	rule_vars,
	rule_vars_,
	rule_var,
	rule_var_,
	rule_fnbody,
	rule_fnexp,
	rule_fnparams,
	rule_names,
	rule_names_
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
	
	token = lexer_token_next(lexer, &code);
	//parser->current_line = lexer->current_line;
	
	do {
		symbol = symbol_stack_peek(parser);
		
		symbol_stack_print(parser);
		lexer_token_print(token);
		//printf("%s\n", code);

		if (is_terminal(symbol) || symbol == NONE) {
			if (symbol == token.class) {
				symbol_stack_pop(parser);
				token = lexer_token_next(lexer, &code);
				//parser->current_line = lexer->current_line;
			}
			else {
				parser_error_list_insert(parser, token.class, 1, &symbol);
				panic(parser);
			}
			continue;
		}
		
		if (parser->panic_mode_flag == true)
			token = lexer_token_next(lexer, &code);

		//parser->current_line = lexer->current_line;

		Rule *rule = parser_table[symbol - BLOCK];
		rule(parser, token.class);
	}
	while (symbol != NONE && token.class != NONE);

	lexer_error_list_print(lexer);
	parser_error_list_print(parser);
	
	if (!parser_error_detected(parser))
		printf("Program parsed with no errors\n");
}

Parser * parser_init(void)
{
	Parser *parser = malloc(sizeof *parser);
	
	parser->symbol_stack = symbol_stack_init();
	parser->error_list = NULL;
	parser->error_flag = false;

	parser->panic_mode_flag = false;
	parser->current_line = 1;

	return parser;
}

void panic(Parser *parser)
{
	Symbol symbol;
	
	parser->panic_mode_flag = true;
	parser->error_flag = true;
	
	symbol = symbol_stack_peek(parser);
	
	while (is_terminal(symbol)) {
		symbol_stack_pop(parser);
		symbol = symbol_stack_peek(parser);
	}
}

bool parser_error_detected(Parser *parser)
{
	return parser->error_flag = true;
}

void parser_error_list_insert(Parser *parser, Symbol received, int expected_len, Symbol *expected)
{
	if (parser->panic_mode_flag == true)
		return;
	
	ParserErrorListNode *head = parser->error_list;
	ParserErrorListNode *next;

	parser->error_flag = true;

	next = malloc(sizeof *next);
	next->next = NULL;
	
	next->line = parser->current_line;

	next->received = received;
	
	next->expected_len = expected_len;
	
	size_t buf_size = sizeof (*next->expected) * next->expected_len;
	
	next->expected = malloc(buf_size);
	
	memcpy(next->expected, expected, buf_size);

	if (parser->error_list == NULL) {
		parser->error_list = next;
		return;
	}

	while (head->next != NULL)
		head = head->next;

	head->next = next;
}

void parser_error_list_print(Parser *parser)
{	
	ParserErrorListNode *next = parser->error_list;

	while (next != NULL) {
		fprintf(stderr, "Recebeu %s mas esperava ", symbol_list[next->received]);
		
		for (int i = 0; i < next->expected_len; i++) {
			fprintf(stderr, "%s, ", symbol_list[next->expected[i]]);
		}
		fprintf(stderr, "\n");
		
		next = next->next;
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

void symbol_stack_push_rule_n(Parser *parser, int len,  ...)
{
	va_list args;
	va_start(args, len);

	symbol_stack_push_rule_v(parser, len, args);
	va_end(args);
}

void symbol_stack_push_rule_v(Parser *parser, int len, va_list args)
{
	Symbol symbol = va_arg(args, Symbol);
	
	if (len <= 0)
		return;

	symbol_stack_push_rule_v(parser, --len, args);
	symbol_stack_push(parser, symbol);
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
	Symbol symbol;
	
	stack = &parser->symbol_stack;
	
	for (int i = 0; i < stack->len; i++) {
		symbol = stack->arr[i];
		printf("%s ", symbol_list[symbol]);
	}
	
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
	Terminal first[] = { DO, WHILE, BREAK, IF, FOR, LOCAL, RETURN, NAME, FUNCTION, LPAREN };
	Terminal follow[] = { NONE, ELSEIF, ELSE, END };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		symbol_stack_push_rule(parser, STMT, SEMICOLON, BLOCK);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}

void rule_stmt(Parser *parser, Terminal t)
{
	Terminal first[] = { DO, WHILE, BREAK, IF, FOR, LOCAL, RETURN, NAME, FUNCTION, LPAREN };
	Terminal follow[] = { SEMICOLON };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		switch (t) {
		case DO:
			symbol_stack_push_rule(parser, DO, BLOCK, END);
			break;
		case WHILE:
			symbol_stack_push_rule(parser, WHILE, EXP, DO, BLOCK, DO, END);
			break;
		case BREAK:
			symbol_stack_push_rule(parser, BREAK);
			break;
		case IF:
			symbol_stack_push_rule(parser, IF, EXP, THEN, BLOCK, ELSESTMT, END);
			break;
		case FOR:
			symbol_stack_push_rule(parser, FOR, LOOPEXP, DO, BLOCK, END);
			break;
		case LOCAL:
			symbol_stack_push_rule(parser, LOCAL, LOCALDECL);
			break;
		case RETURN:
			symbol_stack_push_rule(parser, RETURN, RETURNEXP);
			break;
		case NAME:
		case LPAREN:
			symbol_stack_push_rule(parser, VARS, EQ, EXPS);
			break;
		case FUNCTION:
			symbol_stack_push_rule(parser, FNDECL);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_elsestmt(Parser *parser, Terminal t)
{
	Terminal first[] = { ELSEIF, ELSE };
	Terminal follow[] = { END };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		switch (t) {
		case ELSEIF:
			symbol_stack_push_rule(parser, ELSEIF, EXP, THEN, BLOCK, ELSESTMT);
			break;
		case ELSE:
			symbol_stack_push_rule(parser, ELSE, BLOCK);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_localdecl(Parser *parser, Terminal t)
{
	Terminal first[] = { FUNCTION, NAME };
	Terminal follow[] = { SEMICOLON };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		switch (t) {
		case FUNCTION:
			symbol_stack_push_rule(parser, FNDECL);
			break;
		case NAME:
			symbol_stack_push_rule(parser, NAMES, EQ, EXPS);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_fndecl(Parser *parser, Terminal t)
{
	Terminal first[] = { FUNCTION };
	Terminal follow[] = { SEMICOLON };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push_rule(parser, FUNCTION, NAME, FNBODY);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		panic(parser);
	}
}
void rule_exps(Parser *parser, Terminal t)
{
	Terminal first[] = { NOT, SUB, NIL, TRUE, FALSE, NUM, STRING, NAME, FUNCTION, LPAREN, LBRACE };
	Terminal follow[] = { SEMICOLON, RPAREN };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, EXP, EXPS_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_exp(Parser *parser, Terminal t)
{
	Terminal first[] = { NOT, SUB, NIL, TRUE, FALSE, NUM, STRING, NAME, FUNCTION, LPAREN, LBRACE };
	Terminal follow[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		switch (t) {
		case NOT:
			symbol_stack_push_rule(parser, NOT, EXP, EXP_);
			break;
		case SUB:
			symbol_stack_push_rule(parser, SUB, EXP, EXP_);
			break;
		case NIL:
			symbol_stack_push_rule(parser, NIL, EXP_);
			break;
		case TRUE:
			symbol_stack_push_rule(parser, TRUE, EXP_);
			break;
		case FALSE:
			symbol_stack_push_rule(parser, FALSE, EXP_);
			break;
		case NUM:
			symbol_stack_push_rule(parser, NUM, EXP_);
			break;
		case STRING:
			symbol_stack_push_rule(parser, STRING, EXP_);
			break;
		case FUNCTION:
			symbol_stack_push_rule(parser, FNEXP, EXP_);
			break;
		case LBRACE:
			symbol_stack_push_rule(parser, TABLE, EXP_);
			break;
		case LPAREN:
		case NAME:
			symbol_stack_push_rule(parser, VAR, EXP_);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_exps_(Parser *parser, Terminal t)
{
	Terminal first[] = { COMMA };
	Terminal follow[] = { SEMICOLON };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, COMMA, EXP, EXPS_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_exp_(Parser *parser, Terminal t)
{
	Terminal first[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW };
	Terminal follow[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, BINOP, EXP);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_loopexp(Parser *parser, Terminal t)
{
	Terminal first[] = { EQ, COMMA };
	Terminal follow[] = { DO };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		switch (t) {
		case EQ:
			symbol_stack_push_rule(parser, EQ, EXP, EXP, LOOPEXP_);
			break;
		case COMMA:
			symbol_stack_push_rule(parser, NAMES_, IN, EXPS);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_loopexp_(Parser *parser, Terminal t)
{
	Terminal first[] = { COMMA };
	Terminal follow[] = { DO };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, COMMA, EXP);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_returnexp(Parser *parser, Terminal t)
{
	Terminal first[] = { NOT, SUB, NIL, TRUE, FALSE, NUM, STRING, NAME, FUNCTION, LPAREN, LBRACE };
	Terminal follow[] = { SEMICOLON };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, EXPS);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_table(Parser *parser, Terminal t)
{
	Terminal first[] = { LBRACE };
	Terminal follow[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, LBRACE, FIELDS, RBRACE );
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_fields(Parser *parser, Terminal t)
{
	Terminal first[] = { LBRACKET, NAME };
	Terminal follow[] = { RBRACKET };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push_rule(parser, FIELD, FIELDS_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_field(Parser *parser, Terminal t)
{
	Terminal first[] = { LBRACKET, NAME };
	Terminal follow[] = { RBRACKET };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		switch (t) {
		case LBRACKET:
			symbol_stack_push_rule(parser, LBRACKET, EXP, RBRACKET, EQ, EXP);
			break;
		case NAME:
			symbol_stack_push_rule(parser, NAME, EQ, EXP);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_fields_(Parser *parser, Terminal t)
{
	Terminal first[] = { COMMA };
	Terminal follow[] = { RBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push_rule(parser, COMMA, FIELD, FIELDS_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_binop(Parser *parser, Terminal t)
{
	Terminal first[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW };
	Terminal follow[] = { NOT, NIL, TRUE, FALSE, NUM, STRING, NAME, FUNCTION, LPAREN, LBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push(parser, t);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		panic(parser);
	}
}
void rule_vars(Parser *parser, Terminal t)
{
	Terminal first[] = { NAME, LPAREN };
	Terminal follow[] = { EQ };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push_rule(parser, VAR, VARS_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_vars_(Parser *parser, Terminal t)
{
	Terminal first[] = { COMMA };
	Terminal follow[] = { EQ };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push_rule(parser, COMMA, VAR, VARS_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_var(Parser *parser, Terminal t)
{
	Terminal first[] = { NAME, LPAREN };
	Terminal follow[] = { COMMA, EQ,
		OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE
	};
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		switch (t) {
		case NAME:
			symbol_stack_push_rule(parser, NAME, VAR_);
			break;
		case LPAREN:
			symbol_stack_push_rule(parser, LPAREN, EXP, RPAREN, VAR_);
			break;
		default:
			break;
		}
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_var_(Parser *parser, Terminal t)
{
	Terminal first[] = { LBRACKET };
	Terminal follow[] = { COMMA, EQ,
		OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE
	};
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);

		symbol_stack_push_rule(parser, LBRACKET, EXP, RBRACKET, VAR);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_fnbody(Parser *parser, Terminal t)
{
	Terminal first[] = { LPAREN };
	Terminal follow[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, LPAREN, FNPARAMS, RPAREN, BLOCK, END);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_fnexp(Parser *parser, Terminal t)
{
	Terminal first[] = { FUNCTION };
	Terminal follow[] = { OR, AND, GT, LT, GE, LE, NE, DEQ, CAT, ADD, SUB, MUL, DIV, POW, COMMA, SEMICOLON, DO, THEN, RPAREN, RBRACKET, RBRACE };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, FUNCTION, FNBODY);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_fnparams(Parser *parser, Terminal t)
{
	Terminal first[] = { NAME };
	Terminal follow[] = { RPAREN };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push(parser, NAMES);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		panic(parser);
	}
}
void rule_names(Parser *parser, Terminal t)
{
	Terminal first[] = { NAME };
	Terminal follow[] = { IN, RPAREN, EQ };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, NAME, NAMES_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
void rule_names_(Parser *parser, Terminal t)
{
	Terminal first[] = { COMMA };
	Terminal follow[] = { IN, RPAREN, EQ };
	
	if (is_in_array(t, first, ARRAY_LEN(first))) {
		symbol_stack_pop(parser);
		
		symbol_stack_push_rule(parser, COMMA, NAME, NAMES_);
	}
	else if (is_in_array(t, follow, ARRAY_LEN(follow))) {
		symbol_stack_pop(parser);
		
		parser->panic_mode_flag = false;
	}
	else {
		parser_error_list_insert(parser, t, ARRAY_LEN(first), first);
		panic(parser);
	}
}
