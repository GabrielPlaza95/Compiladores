#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

char * read_file(char *file_name);

int main(int argc, char *argv[])
{
	Token token;
	Lexer *lexer = lexer_init();

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
		//parse_token(token);

		//if (!error_detected(lexer))
		//	print_token(token);
	}
	while (token.class != NONE);

	print_errors(lexer);
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
