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

const char* get_instruction_name_from_middle_bits(byte b){
	switch ((b & 0b00111000) >> 3){
		case 0b000: return "add";
		case 0b001: return "or";
		case 0b010: return "adc";
		case 0b011: return "sbb";
		case 0b100: return "and";
		case 0b101: return "sub";
		case 0b110: return "xor";
		case 0b111: return "cmp";
	}
	return 0;
}

int set_instruction_name_and_type(struct Instruction* instruction, byte first_byte, byte next_byte){
	if ((first_byte & 0b11111100) == 0b10001000) {
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = mov_name;
	}else if ((first_byte & 0b11111110) == 0b11000110) {
		instruction->type = INST_IMD_TO_R_M;
		instruction->name = mov_name;
	} else if ((first_byte & 0b11110000) == 0b10110000) {
		instruction->type = INST_IMD_TO_REG;
		instruction->name = mov_name;
	} else if ((first_byte & 0b11111100) == 0b10100000){
		instruction->type = INST_MEM_TO_ACC_VV;
		instruction->name = mov_name;
	} else if ((first_byte & 0b11111101) == 0b10001100){
		//Register/Memory to Segment Register (vice-versa)
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = mov_name;
		instruction->wide = 1;
		instruction->reg = 0b1000;
	} else if ((first_byte & 0b11000100) == 0b00000000){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = get_instruction_name_from_middle_bits(first_byte);
	} else if ((first_byte & 0b11111100) == 0b10000000){
		instruction->type = INST_IMD_TO_R_M;
		instruction->name = get_instruction_name_from_middle_bits(next_byte);
	} else if ((first_byte & 0b11000110) == 0b00000100){
		instruction->type = INST_IMD_TO_ACC;
		instruction->name = get_instruction_name_from_middle_bits(first_byte);
	} else if ((first_byte & 0b11111110) == 0b11111110){
		instruction->type = INST_R_M_TO_R_M;
		instruction->right_side_only = 1;

		switch ((next_byte & 0b00111000) >> 3){
			case 0b000: instruction->name = inc_name; break;
			case 0b001: instruction->name = dec_name; break;
			case 0b010: instruction->name = call_name; break;
			case 0b100: instruction->name = jmp_name; break;
			case 0b110: instruction->name = push_name; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11111110) == 0b11110110){
		instruction->type = INST_R_M_TO_R_M;
		instruction->right_side_only = 1;

		switch ((next_byte & 0b00111000) >> 3){
			case 0b000: {
				instruction->name = test_name;
				instruction->type = INST_IMD_TO_R_M;
				instruction->right_side_only = 0;
			} break;
			case 0b010: instruction->name = "not"; break;
			case 0b011: instruction->name = "neg"; break;
			case 0b100: instruction->name = "mul"; break;
			case 0b101: instruction->name = "imul"; break;
			case 0b110: instruction->name = "div"; break;
			case 0b111: instruction->name = "idiv"; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11111100) == 0b11010000){
		instruction->type = INST_R_M_TO_R_M;
		instruction->right_side_only = 2;

		switch ((next_byte & 0b00111000) >> 3){
			case 0b000: instruction->name = "rol"; break;
			case 0b001: instruction->name = "ror"; break;
			case 0b010: instruction->name = "rcl"; break;
			case 0b011: instruction->name = "rcr"; break;
			case 0b100: instruction->name = "shl"; break;
			case 0b101: instruction->name = "shr"; break;
			case 0b111: instruction->name = "sar"; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11100000) == 0b01000000){
		instruction->type = INST_REGISTER;

		switch ((first_byte & 0b00111000) >> 3){
			case 0b000: instruction->name = inc_name; break;
			case 0b001: instruction->name = dec_name; break;
			case 0b010: instruction->name = push_name; break;
			case 0b011: instruction->name = pop_name; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11100111) == 0b00000110){
		instruction->type = INST_SEGMENT_REGISTER;
		instruction->name = push_name;
	} else if (first_byte == 0b10001111){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = pop_name;
		instruction->right_side_only = 1;
	} else if ((first_byte & 0b11100111) == 0b00000111){
		instruction->type = INST_SEGMENT_REGISTER;
		instruction->name = pop_name;
	} else if ((first_byte & 0b11111110) == 0b10000110){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = xchg_name;
	} else if ((first_byte & 0b11111000) == 0b10010000){
		instruction->type = INST_REGISTER;
		instruction->name = xchg_name;
	} else if ((first_byte & 0b11111100) == 0b11100100){
		instruction->type = INST_FIXED_PORT;
		instruction->name = (first_byte & 0b10) ? out_name : in_name;
	} else if ((first_byte & 0b11111100) == 0b11101100){
		instruction->type = INST_VARIABLE_PORT;
		instruction->name = (first_byte & 0b10) ? out_name : in_name;
	} else if (first_byte == 0b10001101){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = "lea";
		instruction->direction = 1;
	} else if (first_byte == 0b11000101){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = "lds";
		instruction->direction = 1;
	} else if (first_byte == 0b11000100){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = "les";
		instruction->direction = 1;
		instruction->wide = 1;
	} else if ((first_byte & 0b11111100) == 0b10000100){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = test_name;
	} else if ((first_byte & 0b11111110) == 0b10101000){
		instruction->type = INST_IMD_TO_ACC;
		instruction->name = test_name;
	} else if ((first_byte & 0b11111110) == 0b11110010){
		instruction->type = INST_STRING_MANIPULATION;
		instruction->name = rep_name;
	} else if ((first_byte & 0b11110000) == 0b10100000){
		instruction->type = INST_STRING_MANIPULATION;
		switch ((first_byte & 0b00001110) >> 1){
			case 0b010: instruction->name = "movs"; break;
			case 0b011: instruction->name = "cmps"; break;
			case 0b101: instruction->name = "stos"; break;
			case 0b110: instruction->name = "lods"; break;
			case 0b111: instruction->name = "scas"; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11110111) == 0b11000010){
		instruction->type = INST_RET_DATA;
		instruction->name = ret_name;
	} else if ((first_byte & 0b11111110) == 0b11001100) {
		instruction->type = INST_INT;
		instruction->name = "int";

		if(!(first_byte & 0b00000001)){
			instruction->data = 3;
		}
	} else if ((first_byte & 0b11100111) == 0b00100110) {
		instruction->type = INST_SEGMENT;
	}else{
		//Instructions with no parameters
		switch (first_byte){
			case 0b11010111: instruction->name = "xlat"; break;
			case 0b10011111: instruction->name = "lahf"; break;
			case 0b10011110: instruction->name = "sahf"; break;
			case 0b10011100: instruction->name = "pushf"; break;
			case 0b10011101: instruction->name = "popf"; break;
			case 0b00110111: instruction->name = "aaa"; break;
			case 0b00100111: instruction->name = "daa"; break;
			case 0b00111111: instruction->name = "aas"; break;
			case 0b00101111: instruction->name = "das"; break;
			case 0b11010100: {
				if (next_byte == 0b00001010){
					instruction->name = "aam";
					instruction->type = INST_NO_PARAMS;
					return 2;
				}
			} break;
			case 0b11010101: {
				if (next_byte == 0b00001010){
					instruction->name = "aad";
					instruction->type = INST_NO_PARAMS;
					return 2;
				}
			} break;
			case 0b10011000: instruction->name = "cbw"; break;
			case 0b10011001: instruction->name = "cwd"; break;
			case 0b11001011:
			case 0b11000011: instruction->name = "ret"; break;
			case 0b11001110: instruction->name = "into"; break;
			case 0b11001111: instruction->name = "iret"; break;
			case 0b11111000: instruction->name = "clc"; break;
			case 0b11110101: instruction->name = "cmc"; break;
			case 0b11111001: instruction->name = "stc"; break;
			case 0b11111100: instruction->name = "cld"; break;
			case 0b11111101: instruction->name = "std"; break;
			case 0b11111010: instruction->name = "cli"; break;
			case 0b11111011: instruction->name = "sti"; break;
			case 0b11110100: instruction->name = "hlt"; break;
			case 0b10011011: instruction->name = "wait"; break;
			case 0b11110000: instruction->name = "lock"; break;
		}

		if (instruction->name){
			instruction->type = INST_NO_PARAMS;
			return 1;
		}

		//Conditional jumps
		switch (first_byte){
			case 0b01110101: instruction->name = "jnz"; break;
			case 0b01110100: instruction->name = "je"; break;
			case 0b01111100: instruction->name = "jl"; break;
			case 0b01111110: instruction->name = "jle"; break;
			case 0b01110010: instruction->name = "jb"; break;
			case 0b01110110: instruction->name = "jbe"; break;
			case 0b01111010: instruction->name = "jp"; break;
			case 0b01110000: instruction->name = "jo"; break;
			case 0b01111000: instruction->name = "js"; break;
			case 0b01111101: instruction->name = "jnl"; break;
			case 0b01111111: instruction->name = "jg"; break;
			case 0b01110011: instruction->name = "jnb"; break;
			case 0b01110111: instruction->name = "ja"; break;
			case 0b01111011: instruction->name = "jnp"; break;
			case 0b01110001: instruction->name = "jno"; break;
			case 0b01111001: instruction->name = "jns"; break;
			case 0b11100010: instruction->name = "loop"; break;
			case 0b11100001: instruction->name = "loopz"; break;
			case 0b11100000: instruction->name = "loopnz"; break;
			case 0b11100011: instruction->name = "jcxz"; break;
		}
		if (instruction->name){
			instruction->type = INST_CONDITIONAL_JUMP;
			return 1;
		}

		return 0;
	}
	return 1;
}

long decode(struct String code, struct Instruction* buffer){
	long instruction_counter = 0;

	long instruction_start = 0;
	for(long i = 0; i < code.size; i++){
		struct Instruction instruction = { 0 };

		byte first_byte = code.data[i];
		print_byte(first_byte);

		int result = set_instruction_name_and_type(&instruction, first_byte, code.data[i + 1]);
		if (!result){
			printf("Instruction not implemented (1)\n");
			return instruction_counter;
		} else if (result == 2){
			i++; //Second byte was used in the instruction parsing
		}else if (instruction.type == INST_R_M_TO_R_M) {
			//Register/Memory to Register/Memory
			if(!instruction.direction)
				instruction.direction = first_byte & 0b00000010;
			
			if(!instruction.wide)
				instruction.wide = first_byte & 0b00000001;

			if (i + 1 >= code.size) {
				printf("Missing second byte of MOV\n");
				return 0;
			}

			byte second_byte = code.data[++i];
			print_byte(second_byte);


			instruction.mode = second_byte >> 6;
			instruction.reg += (second_byte >> 3) & 0b111;
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
					byte disp_low = code.data[++i];
					print_byte(disp_low);

					if (instruction.wide){
						byte* disp_pointer = (byte*) &instruction.disp;

						disp_pointer[0] = disp_low;
						disp_pointer[1] = code.data[++i];
						print_byte(disp_pointer[1]);
					} else {
						instruction.disp = (char) disp_low;
					}

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
		}else if (instruction.type == INST_REGISTER) {
			instruction.reg = first_byte & 0b00000111;
		}else if (instruction.type == INST_SEGMENT_REGISTER || instruction.type == INST_SEGMENT) {
			instruction.reg = (first_byte & 0b00011000) >> 3;
		}else if (instruction.type == INST_FIXED_PORT) {
			instruction.wide = first_byte & 0b00000001;
			
			byte second_byte = code.data[++i];
			print_byte(second_byte);

			instruction.data = second_byte;
		}else if (instruction.type == INST_VARIABLE_PORT) {
			instruction.wide = first_byte & 0b00000001;
		}else if (instruction.type == INST_STRING_MANIPULATION) {
			instruction.wide = first_byte & 0b00000001;
		}else if (instruction.type == INST_RET_DATA) {
			byte* data_pointer = (byte*) &instruction.data;

			data_pointer[0] = code.data[++i];
			print_byte(data_pointer[0]);

			data_pointer[1] = code.data[++i];
			print_byte(data_pointer[1]);
		}else if (instruction.type == INST_INT) {
			if (!instruction.data){
				byte data = code.data[++i];
				print_byte(data);
				instruction.data = data;
			}
		}else if(instruction.type != INST_NO_PARAMS){
			printf("Instruction not implemented (2)\n");
			return instruction_counter;
		}

		instruction.num_bytes = i - instruction_start;
		instruction_start = i;

		buffer[instruction_counter++] = instruction;
#if DEBUG
		if (!print_single_asm(instruction)){
			printf("Error printing instruction\n");
			return instruction_counter;
		}
#endif
	}

	return instruction_counter;
}