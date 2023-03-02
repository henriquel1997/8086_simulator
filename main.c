#include <stdio.h>
#include <stdlib.h>

struct String {
	char* data;
	long size;
};

struct String read_file(char* filename){
	struct String string;
	FILE * file = fopen(filename, "rb");

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

#define byte unsigned char

const char* get_register_name_from_code(byte wide, byte register_code){
	if(wide){
		switch(register_code){
			case 0b000: return "ax";
			case 0b001: return "cx";
			case 0b010: return "dx";
			case 0b011: return "bx";
			case 0b100: return "sp";
			case 0b101: return "bp";
			case 0b110: return "si";
			case 0b111: return "di";
		}
	}else{
		switch(register_code){
			case 0b000: return "al";
			case 0b001: return "cl";
			case 0b010: return "dl";
			case 0b011: return "bl";
			case 0b100: return "ah";
			case 0b101: return "ch";
			case 0b110: return "dh";
			case 0b111: return "bh";
		}
	}
	return 0;
}

int decode(struct String code){
	printf("bits 16\n");

	for(long i = 0; i < code.size; i++){
		byte first_byte = code.data[i];
		byte six_bits = first_byte >> 2;
		if(six_bits == 0b100010){
			//MOV Instruction
			byte direction = (first_byte >> 1) & 0b1;
			byte wide = (first_byte >> 0) & 0b1;

			if(i + 1 >= code.size){
				printf("Missing second byte of MOV\n");
				return 0;
			}

			byte second_byte = code.data[++i];

			byte mode = second_byte >> 6;
			if(mode == 0b11){
				//Register mode
				byte reg = (second_byte >> 3) & 0b111;
				byte r_m = (second_byte >> 0) & 0b111;

				printf("mov ");
				if(direction){
					printf("%s, %s\n", get_register_name_from_code(wide, reg), get_register_name_from_code(wide, r_m));
				}else{
					printf("%s, %s\n", get_register_name_from_code(wide, r_m), get_register_name_from_code(wide, reg));
				}
			}else{
				printf("Mode %c not implemented\n", mode);
			}
		}else{
			printf("Instruction not implemented\n");
			return 0;
		}
	}
	return 1;
}

int main(int argc, char **argv){
	if(argc < 2){
		printf("No file specified\n");
		return -1;
	}

	struct String code = read_file(argv[1]);
	if(!code.data){
		printf("Couldn't read file %s\n", argv[1]);
		return -2;
	}

	decode(code);
	return 0;
}