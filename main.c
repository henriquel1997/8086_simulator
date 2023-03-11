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
void print_byte(byte b){
#ifdef DEBUG
	byte mask = 0b10000000;
	while (mask){
		if (mask & b){
			printf("1");
		} else {
			printf("0");
		}
		mask >>= 1;
	}
	printf("\n");
#endif
}

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

const char* get_address_calculation_with_no_displacement(byte mode, byte r_m){
	switch (r_m){
		case 0b000: return "[bx + si]"; //byte
		case 0b001: return "[bx + di]"; //byte
		case 0b010: return "[bp + si]"; //byte
		case 0b011: return "[bp + di]"; //byte
		case 0b100: return "[si]"; //byte
		case 0b101: return "[di]"; //byte
		case 0b110: return 0; //Should never get here
		case 0b111: return "[bx]"; //byte
	}
	return 0;
}

void get_address_calculation_with_displacement(char* buffer, byte mode, byte r_m, short disp){
	if (disp == 0){
		switch (r_m) {
			case 0b000: sprintf(buffer, "[bx + si]"); break;
			case 0b001: sprintf(buffer, "[bx + di]"); break;
			case 0b010: sprintf(buffer, "[bp + si]"); break;
			case 0b011: sprintf(buffer, "[bp + di]"); break;
			case 0b100: sprintf(buffer, "[si]"); break;
			case 0b101: sprintf(buffer, "[di]"); break;
			case 0b110: sprintf(buffer, "[bp]"); break;
			case 0b111: sprintf(buffer, "[bx]"); break;
		}
	} else {
		switch (r_m) {
			case 0b000: sprintf(buffer, "[bx + si + %d]", disp); break;
			case 0b001: sprintf(buffer, "[bx + di + %d]", disp); break;
			case 0b010: sprintf(buffer, "[bp + si + %d]", disp); break;
			case 0b011: sprintf(buffer, "[bp + di + %d]", disp); break;
			case 0b100: sprintf(buffer, "[si + %d]", disp); break;
			case 0b101: sprintf(buffer, "[di + %d]", disp); break;
			case 0b110:
				if (mode == 0b00){
					sprintf(buffer, "[%d]", disp); 
				} else {
					sprintf(buffer, "[bp + %d]", disp); 
				}
				break;
			case 0b111:
				if (mode == 0b00){
					sprintf(buffer, "[%d]", disp); 
				} else {
					sprintf(buffer, "[bx + %d]", disp); 
				}
				break;
		}
	}
}

const char* mov_name = "mov";

enum InstructionType {
	INST_R_M_TO_R_M,
	INST_IMD_TO_R_M,
	INST_IMD_TO_REG,
	INST_IMD_TO_ACC,
	INST_MEM_TO_ACC_VV,
	INST_CONDITIONAL_JUMP
};

struct Instruction {
	enum InstructionType type;
	const char* name;
	char num_bytes;
	byte direction;
	byte sign;
	byte wide;
	byte mode;
	byte reg;
	byte r_m;
	short disp;
	unsigned short data;
	unsigned short address;
	char jump_increment;
	long label_pos;
};

void print_inst_with_memory_address_displacement(struct Instruction instruction){
	const char* reg_name = get_register_name_from_code(instruction.wide, instruction.reg);
	char buffer [20];
	get_address_calculation_with_displacement(buffer, instruction.mode, instruction.r_m, instruction.disp);
	if (instruction.direction){
		printf("%s %s, %s\n", instruction.name, reg_name, buffer);
	} else {
		printf("%s %s, %s\n", instruction.name, buffer, reg_name);
	}
}

int print_single_asm(struct Instruction instruction){
	if (instruction.type == INST_R_M_TO_R_M) {
		if (instruction.mode == 0b00) {
			//Memory mode with no displacement*
			if (instruction.r_m != 0b110) {
				const char* reg_name = get_register_name_from_code(instruction.wide, instruction.reg);
				const char* address_calc = get_address_calculation_with_no_displacement(instruction.mode, instruction.r_m);
				if (instruction.direction) {
					printf("%s %s, %s\n", instruction.name, reg_name, address_calc);
				} else {
					printf("%s %s, %s\n", instruction.name, address_calc, reg_name);
				}
			} else {
				//*There actually is a 16-bit displacement if r_m is 0b110
				print_inst_with_memory_address_displacement(instruction);
			}
		} else if (instruction.mode == 0b01 || instruction.mode == 0b10) {
			//Memory mode with 8-bit(0b01) or 16-bit(0b10) displacement
			print_inst_with_memory_address_displacement(instruction);
		} else if (instruction.mode == 0b11) {
			//Register mode
			if (instruction.direction) {
				printf("%s %s, %s\n", instruction.name, get_register_name_from_code(instruction.wide, instruction.reg), get_register_name_from_code(instruction.wide, instruction.r_m));
			} else {
				printf("%s %s, %s\n", instruction.name, get_register_name_from_code(instruction.wide, instruction.r_m), get_register_name_from_code(instruction.wide, instruction.reg));
			}
		} else {
			printf("Mode not implemented: ");
			print_byte(instruction.mode);
			return 0;
		}
	} else if (instruction.type == INST_IMD_TO_R_M) {
		if (instruction.mode == 0b11) {
			//Register mode
			printf("%s %s, %d\n", instruction.name, get_register_name_from_code(instruction.wide, instruction.r_m), instruction.data);
		} else {
			//Memory mode
			char buffer[20];
			if (instruction.mode == 0b00 && instruction.r_m != 0b110) {
				sprintf(buffer, "%s", get_address_calculation_with_no_displacement(instruction.mode, instruction.r_m));
			} else {
				get_address_calculation_with_displacement(buffer, instruction.mode, instruction.r_m, instruction.disp);
			}

			const char* prefix = "byte";
			if (instruction.wide){
				prefix = "word";
			}

			byte is_wide = instruction.name == mov_name ? instruction.wide : (!instruction.sign && instruction.wide);
			if (is_wide) {
				printf("%s %s %s, word %d\n", instruction.name, prefix, buffer, instruction.data);
			} else {
				printf("%s %s %s, byte %d\n", instruction.name, prefix, buffer, instruction.data);
			}
		}
	} else if (instruction.type == INST_IMD_TO_REG) {
		printf("%s %s, %d\n", instruction.name, get_register_name_from_code(instruction.wide, instruction.reg), instruction.data);
	} else if (instruction.type == INST_MEM_TO_ACC_VV) {
		const char* acc;
		if (instruction.wide) {
			acc = "ax";
		} else {
			acc = "al";
		}

		if (instruction.direction) {
			printf("%s [%d], %s\n", instruction.name, instruction.address, acc);
		} else {
			printf("%s %s, [%d]\n", instruction.name, acc, instruction.address);
		}
	}else if(instruction.type == INST_IMD_TO_ACC){
		const char* reg_name = "al";
		if (instruction.wide){
			reg_name = "ax";
		}

		printf("%s %s, %d\n", instruction.name, reg_name, instruction.data);
	}else if (instruction.type == INST_CONDITIONAL_JUMP) {
		if (instruction.label_pos >= 0){
			printf("%s label%ld\n", instruction.name, instruction.label_pos);
		} else {
			printf("%s %d\n", instruction.name, instruction.jump_increment);
		}
	} else {
		return 0;
	}

	return 1;
}

int print_asm(struct Instruction* instructions, long num_instructions){
	char* label_array = calloc(num_instructions, sizeof(char));

	long total_bytes = 0;
	for (long i = 0; i < num_instructions; i++){
		if(instructions[i].type == INST_CONDITIONAL_JUMP){
			char inc = instructions[i].jump_increment;

			long label_pos = -1;
			if (inc > 0){
				for (label_pos = i + 1; label_pos < num_instructions; label_pos++){
					inc -= instructions[label_pos].num_bytes;
					if (inc == 0){
						label_pos++;
						break;
					} else if (inc < 0) return 0;
				}
			}else{
				for (label_pos = i; label_pos > 0; label_pos--){
					inc += instructions[label_pos].num_bytes;
					if (inc == 0) break;
					else if (inc > 0) return 0;
				}
			}

			if (label_pos >= 0 && label_pos < num_instructions){
				label_array[label_pos] = 1;
				instructions[i].label_pos = label_pos;
			} else {
				return 0;
			}
		}
		total_bytes += instructions[i].num_bytes;
	}

	printf("bits 16\n");

	for (long i = 0; i < num_instructions; i++){
		if (label_array[i]){
			printf("label%ld:\n", i);
		}
		print_single_asm(instructions[i]);
	}

	return 1;
}

const char* get_instruction_name(byte b){
	switch ((b & 0b00111000) >> 3){
		case 0b000: return "add";
		case 0b101: return "sub";
		case 0b111: return "cmp";
	}
	return 0;
}

long decode(struct String code, struct Instruction* buffer){
	long instruction_counter = 0;

	long instruction_start = 0;
	for(long i = 0; i < code.size; i++){
		struct Instruction instruction = { 0 };

		byte first_byte = code.data[i];
		print_byte(first_byte);

		instruction.name = mov_name;
		if ((first_byte & 0b11111100) == 0b10001000) {
			instruction.type = INST_R_M_TO_R_M;
		}else if ((first_byte & 0b11111110) == 0b11000110) {
			instruction.type = INST_IMD_TO_R_M;
		} else if ((first_byte & 0b11110000) == 0b10110000) {
			instruction.type = INST_IMD_TO_REG;
		} else if ((first_byte & 0b11111100) == 0b10100000){
			instruction.type = INST_MEM_TO_ACC_VV;

		} else if ((first_byte & 0b11000100) == 0b00000000){
			instruction.type = INST_R_M_TO_R_M;
			instruction.name = get_instruction_name(first_byte);
		} else if ((first_byte & 0b11111100) == 0b10000000){
			instruction.type = INST_IMD_TO_R_M;
			instruction.name = get_instruction_name(code.data[i + 1]);
		} else if ((first_byte & 0b11000110) == 0b00000100){
			instruction.type = INST_IMD_TO_ACC;
			instruction.name = get_instruction_name(first_byte);
		}else{
			switch (first_byte){
				case 0b01110101: instruction.name = "jnz"; break;
				case 0b01110100: instruction.name = "je"; break;
				case 0b01111100: instruction.name = "jl"; break;
				case 0b01111110: instruction.name = "jle"; break;
				case 0b01110010: instruction.name = "jb"; break;
				case 0b01110110: instruction.name = "jbe"; break;
				case 0b01111010: instruction.name = "jp"; break;
				case 0b01110000: instruction.name = "jo"; break;
				case 0b01111000: instruction.name = "js"; break;
				case 0b01111101: instruction.name = "jnl"; break;
				case 0b01111111: instruction.name = "jg"; break;
				case 0b01110011: instruction.name = "jnb"; break;
				case 0b01110111: instruction.name = "ja"; break;
				case 0b01111011: instruction.name = "jnp"; break;
				case 0b01110001: instruction.name = "jno"; break;
				case 0b01111001: instruction.name = "jns"; break;
				case 0b11100010: instruction.name = "loop"; break;
				case 0b11100001: instruction.name = "loopz"; break;
				case 0b11100000: instruction.name = "loopnz"; break;
				case 0b11100011: instruction.name = "jcxz"; break;
				default:
					printf("Instruction not implemented (1)\n");
					return instruction_counter;
			}
			instruction.type = INST_CONDITIONAL_JUMP;
			instruction.label_pos = -1;
		}

		if (instruction.type == INST_R_M_TO_R_M) {
			//Register/Memory to Register/Memory
			instruction.direction = first_byte & 0b00000010;
			instruction.wide 		 = first_byte & 0b00000001;

			if (i + 1 >= code.size) {
				printf("Missing second byte of MOV\n");
				return 0;
			}

			byte second_byte = code.data[++i];
			print_byte(second_byte);


			instruction.mode = second_byte >> 6;
			instruction.reg = (second_byte >> 3) & 0b111;
			instruction.r_m = (second_byte >> 0) & 0b111;

			if (instruction.mode == 0b00 && instruction.r_m == 0b110) {
				//Memory mode with no displacement*
				//*There actually is a 16-bit displacement if r_m is 0b110
				byte* disp_pointer = (byte*) &instruction.disp;
				disp_pointer[0] = code.data[++i];
				print_byte(disp_pointer[0]);
				disp_pointer[1] = code.data[++i];
				print_byte(disp_pointer[1]);
					
			} else if (instruction.mode == 0b01 || instruction.mode == 0b10) {
				//Memory mode with 8-bit(0b01) or 16-bit(0b10) displacement
				byte d_low = code.data[++i];
				print_byte(d_low);

				if (instruction.mode == 0b10) {
					byte* disp_pointer = (byte*) &instruction.disp;
					disp_pointer[0] = d_low;
					disp_pointer[1] = code.data[++i];
					print_byte(disp_pointer[1]);
				} else {
					instruction.disp = (char)d_low;
				}
			}
		}else if (instruction.type == INST_IMD_TO_R_M) {
			//Immediate to Register or Memory
			instruction.sign = (first_byte & 0b00000010) >> 1;
			instruction.wide = first_byte & 0b00000001;
			byte is_wide = instruction.name == mov_name ? instruction.wide : (!instruction.sign && instruction.wide);

			byte second_byte = code.data[++i];
			print_byte(second_byte);

			instruction.mode = second_byte >> 6;
			instruction.r_m = (second_byte >> 0) & 0b111;

			byte* data_pointer = (byte*) &instruction.data;

			if (instruction.mode == 0b11){
				//Register mode
				data_pointer[0] = code.data[++i];
				print_byte(data_pointer[0]);

				if (is_wide){
					data_pointer[1] = code.data[++i];
					print_byte(data_pointer[1]);
				}
			} else {
				//Memory mode
				if (instruction.mode == 0b00 && instruction.r_m != 0b110) {
					data_pointer[0] = code.data[++i];
					print_byte(data_pointer[0]);

					if (is_wide) {
						data_pointer[1] = code.data[++i];
						print_byte(data_pointer[1]);
					}
				} else {
					byte* disp_pointer = (byte*) &instruction.disp;

					disp_pointer[0] = code.data[++i];
					print_byte(disp_pointer[0]);
					disp_pointer[1] = code.data[++i];
					print_byte(disp_pointer[1]);

					data_pointer[0] = code.data[++i];
					print_byte(data_pointer[0]);

					if (is_wide) {
						data_pointer[1] = code.data[++i];
						print_byte(data_pointer[1]);
					}
				}
			}

		}else if (instruction.type == INST_IMD_TO_REG) {
			//Immediate to Register
			instruction.wide = first_byte & 0b00001000;
			instruction.reg = first_byte & 0b00000111;

			byte* data_pointer = (byte*) &instruction.data;

			data_pointer[0] = code.data[++i];
			print_byte(data_pointer[0]);

			if (instruction.wide){
				data_pointer[1] = code.data[++i];
				print_byte(data_pointer[1]);
			}
		}else if (instruction.type == INST_MEM_TO_ACC_VV) {
			//Memory to Accumulator or Accumulator to Memory
			instruction.direction = first_byte & 0b00000010;
			instruction.wide 		= first_byte & 0b00000001;

			byte* address_pointer = (byte*) &instruction.address;

			address_pointer[0] = code.data[++i];
			print_byte(address_pointer[0]);

			address_pointer[1] = code.data[++i];
			print_byte(address_pointer[1]);
		}else if (instruction.type == INST_IMD_TO_ACC) {
			instruction.wide = first_byte & 0b00000001;

			byte* data_pointer = (byte*) &instruction.data;

			data_pointer[0] = code.data[++i];
			print_byte(data_pointer[0]);

			if (instruction.wide){
				data_pointer[1] = code.data[++i];
				print_byte(data_pointer[1]);
			}
		}else if (instruction.type == INST_CONDITIONAL_JUMP) {
			instruction.jump_increment = code.data[++i];
			print_byte(instruction.jump_increment);
		}else{
			printf("Instruction not implemented (2)\n");
			return instruction_counter;
		}

		instruction.num_bytes = i - instruction_start;
		instruction_start = i;

		buffer[instruction_counter++] = instruction;
#if DEBUG
		print_single_asm(instruction);
#endif
	}

	return instruction_counter;
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

#ifdef DEBUG
	printf("Num Code Bytes: %ld\n", code.size);
#endif

	struct Instruction* instructions = malloc(sizeof(struct Instruction) * code.size);
	long num_instructions = decode(code, instructions);

	print_asm(instructions, num_instructions);
	return 0;
}