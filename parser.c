#include "lexer.h"

int main(int argc, char *argv[])
{
	Lexer lexer = lexer_init();

	StateMachineOutput out;

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
		out = next_token(&lexer, &code);

		if (lexer.error_flag == false)
			print_token(out.token);
	}
	while (out.token.class != NONE);

	print_errors(&lexer);
}
