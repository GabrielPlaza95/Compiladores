#ifndef COMPILER_H
#define COMPILER_H

typedef enum symbol {
	// Terminals
	SKIP = -1,
	NONE = 0,
	EQ,
	LT,
	GT,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	POW,
	LPAREN,
	RPAREN,
	LBRACKET,
	RBRACKET,
	LBRACE,
	RBRACE,
	COMMA,
	SEMICOLON,
	DEQ,
	NE,
	LE,
	GE,
	CAT,
	NUM,
	STRING,
	NAME,
	DO,
	IF,
	IN,
	OR,
	AND,
	END,
	FOR,
	NIL,
	NOT,
	ELSE,
	THEN,
	TRUE,
	BREAK,
	FALSE,
	LOCAL,
	UNTIL,
	WHILE,
	ELSEIF,
	REPEAT,
	RETURN,
	FUNCTION,
	
	// Nonterminals
	BLOCK,
	STMT,
	ELSESTMT,
	LOCALDECL,
	FNDECL,
	EXPS,
	EXP,
	EXPS_,
	EXP_,
	LOOPEXP,
	LOOPEXP_,
	RETURNEXP,
	TABLE,
	FIELDS,
	FIELD,
	FIELDS_,
	BINOP,
	VARS,
	VARS_,
	VAR,
	VAR_,
	FNBODY,
	FNEXP,
	FNPARAMS,
	NAMES,
	NAMES_
}
Symbol;

typedef enum {
	INVALID_CHARACTER,
	INVALID_TOKEN,
	INVALID_ESCAPE_SEQUENCE,
	UNTERMINATED_STRING,
	UNTERMINATED_COMMENT
}
ErrorClass;

typedef struct error_list_node ErrorListNode;

struct error_list_node {
	int line;
	ErrorClass error_class;
	ErrorListNode *next;
	char *str;
};

#endif
