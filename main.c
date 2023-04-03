#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcmp

#include "declarations.h"
#include "execute.c"
#include "print.c"
#include "decode.c"

struct String read_file(char* file_name){
	struct String string;
	FILE * file = fopen(file_name, "rb");

	if (file){
	  fseek(file, 0, SEEK_END);
	  string.size = ftell(file);
	  fseek(file, 0, SEEK_SET);
	  string.data = malloc(string.size);
	  if (string.data){
	    fread(string.data, 1, string.size, file);
	  }
	  fclose(file);
	}

	return string;
}

int main(int argc, char **argv){
	if(argc < 2){
		printf("No file specified\n");
		return -1;
	}

	int should_execute = 0;
	char* file_name;
	if (strcmp(argv[1], "--exec") == 0){
		should_execute = 1;
		file_name = argv[2];
	} else {
		file_name = argv[1];
	}

	struct String code = read_file(file_name);
	if(!code.data){
		printf("Couldn't read file %s\n", file_name);
		return -2;
	}

#ifdef DEBUG
	printf("Num Code Bytes: %ld\n", code.size);
#endif

	struct State state = { 0 };

	state.instructions = malloc(sizeof(struct Instruction) * code.size);
	state.num_instructions = decode(code, state.instructions);

	if (should_execute){
		struct State prev_state = state;

		printf("\n");
		while (execute_instruction(&state)){
			print_single_asm(state.instructions[state.current_instruction - 1]);
			printf("Registers:\n");
			print_registers(&state, &prev_state);
			printf("\nFlags:\n");
			print_flags(&state, &prev_state);
			printf("\n");
			
			prev_state = state;
		}

		printf("\nFinal registers:\n");
		print_registers(&state, &prev_state);
	} else {
#ifndef DEBUG
		print_asm(state.instructions, state.num_instructions);
#endif
	}
	
	return 0;
}