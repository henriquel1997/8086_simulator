#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcmp

#include "declarations.h"
#include "execute.c"
#include "print.c"
#include "decode.c"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

int decode_and_execute(struct State* state){
	struct String code = state->code;

	long instruction_start = 0;
	struct State prev_state = { 0 };

	while(!code_ended(state)){
		struct Instruction instruction = { 0 };

		if (!decode_instruction(&instruction, state)){
			printf("Instruction not implemented (1)\n");
			return 0;
		}

		instruction.num_bytes = state->registers[REG_IP] - instruction_start;
		instruction_start = state->registers[REG_IP];

		if (!print_single_asm(instruction)){
			printf("Error printing instruction\n");
			return 0;
		}

		if (state->should_execute){
			prev_state = *state;

			printf("\n");
			if(!execute_instruction(instruction, state)){
				return 0;
			}

			printf("Registers:\n");
			print_registers(state, &prev_state);
			printf("\nFlags:\n");
			print_flags(state, &prev_state);
			printf("\n");

			if (state->step_by_step){
				printf("Press Enter to continue\n");
				getchar();
			}
		}
	}


	if (state->should_execute){
		printf("\nFinal registers:\n");
		print_registers(state, &prev_state);
		printf("\nFinal Flags:\n");
		print_flags(state, &prev_state);
		printf("\n");
	}
	return 1;
}

int main(int argc, char **argv){
	if(argc < 2){
		printf("No file specified\n");
		return -1;
	}

	struct State state = { 0 };
	state.memory = calloc(MEMORY_SIZE, sizeof(byte));

	int should_dump = 0;

	for (int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--exec") == 0){
			state.should_execute = 1;
		}else if (strcmp(argv[i], "--step_by_step") == 0){
			state.step_by_step = 1;
		}else if (strcmp(argv[i], "--dump") == 0){
			should_dump = 1;
		} else {
			printf("Unknown command: %s\n", argv[i]);
			return 0;
		}
	}

	char* file_name = argv[argc - 1];

	state.code = read_file(file_name);
	if(!state.code.data){
		printf("Couldn't read file %s\n", file_name);
		return -2;
	}

#ifdef DEBUG
	printf("Num Code Bytes: %ld\n", state.code.size);
#endif

	decode_and_execute(&state);

	if (should_dump){
		int name_len = strlen(file_name);
		int name_start = 0;
		int name_end = name_len;
		for (int i = 0; i < name_len; i++){
			if (file_name[i] == '/' || file_name[i] == '\\') {
				name_start = i + 1;
			} else if (file_name[i] == '.'){
				name_end = i;
				break;
			}
		}


		char buffer[100];
		strcpy(buffer, &file_name[name_start]);
		strcpy(&buffer[name_end - name_start], "_output_image.bmp");
		stbi_write_bmp(buffer, 64, 65, 4, state.memory);
	}
	
	return 0;
}