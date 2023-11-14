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
	DOT,
	LPAREN,
	RPAREN,
	LBRACE,
	RBRACE,
	LBRACKET,
	RBRACKET,
	COMMA,
	COLON,
	SEMICOLON,
	DEQ,
	NE,
	LE,
	GE,
	CAT,
	NUM,
	STRING,
	ID,
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
	VAR,
	VARS_,
	FNCALL,
	INDEXEXP,
	CALLEXP,
	PREFIXEXP,
	SUFFIXEXP,
	FNBODY,
	FNEXP,
	FNPARAMS,
	ARGS,
	ARGS_,
	NAMES,
	NAMES_
}
Symbol;

#endif
