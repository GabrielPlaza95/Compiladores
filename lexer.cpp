#include <stdio.h>
#include <stdlib.h>

enum Tokens {
	IF = 256,
	THEN = 257,
	ELSE = 258,
	RELOP = 259,
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

class IState {
public:
	virtual IState* next(char c) = 0;
	virtual bool finished(Token& token) = 0;
	virtual void action() = 0;

protected:
	inline static IState* initState;
	inline static IState* eqState;
};

class EqState;

class InitState : public IState {
protected:
	InitState() {};
	
public:
	InitState(InitState &other) = delete;
	void operator=(InitState const&) = delete;
	
	static IState* getState() {
		if (IState::initState == nullptr){
			IState::initState = new InitState();
		}
		return IState::initState;
	};
	
	IState* next(char c) override {
		if (c == ' ' || c == '\n') {
			return InitState::getState();
		} 
		else if (c == '=') {
			return EqState::getState();
		}
		
		
		return InitState::getState();
	}
	
	bool finished(Token& token) override {
		return false;
	};
	
	void action() override {
		printf("Initial State\n");
	};
};

class EqState : public IState {
protected:
	EqState() {};
	
public:
	EqState(EqState &other) = delete;
	void operator=(EqState const&) = delete;
	
	static IState* getState() {
		if (IState::eqState == nullptr){
			IState::eqState = new EqState();
		}
		return IState::eqState;
	};
	
	IState* next(char c) {
		return InitState::getState();
	}
	
	bool finished(Token& token) {
		return true;
	};
	
	void action() {
		printf("<relop, EQ>\n");
		auto token = new Token();
		
		token->nome_token = RELOP;
		token->atributo = EQ;
	};
};

char const* readFile(char const* fileName)
{
	FILE *file = fopen(fileName, "r");
	char *code;
	int n = 0;
	int c;

	if(file == NULL) return NULL;

	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	code = new char(f_size);

	while ((c = fgetc(file))!= EOF)
	{
		code[n++] = (char) c;
	}
	code[n] = '\0';
	return code;
}

Token proximo_token(char const* code, IState* estado)
{
	Token token;
	char c;
	
	while ((c = *code++) != '\0') {
		//printf("%c\n", c);
		//puts(code);
		
		estado->action();
		
		if (estado->finished(token)) {
			return token;
		}
		
		IState* novo_estado = estado->next(c);
		
		estado = novo_estado;
	}
	
	token.nome_token = EOF;
	token.atributo = -1;
	return(token);
}


int main ()
{
	Token token;
	
	IState* estado = InitState::getState();
	
	char const* code = readFile("programa.txt");
	
	do {
		token = proximo_token(code, estado);
	}
	while (token.nome_token != EOF);
}
