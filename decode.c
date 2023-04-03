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

enum InstructionName get_instruction_name_from_middle_bits(byte b){
	switch ((b & 0b00111000) >> 3){
		case 0b000: return INST_NAME_ADD;
		case 0b001: return INST_NAME_OR;
		case 0b010: return INST_NAME_ADC;
		case 0b011: return INST_NAME_SBB;
		case 0b100: return INST_NAME_AND;
		case 0b101: return INST_NAME_SUB;
		case 0b110: return INST_NAME_XOR;
		case 0b111: return INST_NAME_CMP;
	}
	return 0;
}

int set_instruction_name_and_type(struct Instruction* instruction, byte first_byte, byte next_byte){
	if ((first_byte & 0b11111100) == 0b10001000) {
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_MOV;
	}else if ((first_byte & 0b11111110) == 0b11000110) {
		instruction->type = INST_IMD_TO_R_M;
		instruction->name = INST_NAME_MOV;
	} else if ((first_byte & 0b11110000) == 0b10110000) {
		instruction->type = INST_IMD_TO_REG;
		instruction->name = INST_NAME_MOV;
	} else if ((first_byte & 0b11111100) == 0b10100000){
		instruction->type = INST_MEM_TO_ACC_VV;
		instruction->name = INST_NAME_MOV;
	} else if ((first_byte & 0b11111101) == 0b10001100){
		//Register/Memory to Segment Register (vice-versa)
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_MOV;
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
			case 0b000: instruction->name = INST_NAME_INC; break;
			case 0b001: instruction->name = INST_NAME_DEC; break;
			case 0b010: instruction->name = INST_NAME_CALL; break;
			case 0b100: instruction->name = INST_NAME_JMP; break;
			case 0b110: instruction->name = INST_NAME_PUSH; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11111110) == 0b11110110){
		instruction->type = INST_R_M_TO_R_M;
		instruction->right_side_only = 1;

		switch ((next_byte & 0b00111000) >> 3){
			case 0b000: {
				instruction->type = INST_IMD_TO_R_M;
				instruction->name = INST_NAME_TEST;
				instruction->right_side_only = 0;
			} break;
			case 0b010: instruction->name = INST_NAME_NOT; break;
			case 0b011: instruction->name = INST_NAME_NEG; break;
			case 0b100: instruction->name = INST_NAME_MUL; break;
			case 0b101: instruction->name = INST_NAME_IMUL; break;
			case 0b110: instruction->name = INST_NAME_DIV; break;
			case 0b111: instruction->name = INST_NAME_IDIV; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11111100) == 0b11010000){
		instruction->type = INST_R_M_TO_R_M;
		instruction->right_side_only = 2;

		switch ((next_byte & 0b00111000) >> 3){
			case 0b000: instruction->name = INST_NAME_ROL; break;
			case 0b001: instruction->name = INST_NAME_ROR; break;
			case 0b010: instruction->name = INST_NAME_RCL; break;
			case 0b011: instruction->name = INST_NAME_RCR; break;
			case 0b100: instruction->name = INST_NAME_SHL; break;
			case 0b101: instruction->name = INST_NAME_SHR; break;
			case 0b111: instruction->name = INST_NAME_SAR; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11100000) == 0b01000000){
		instruction->type = INST_REGISTER;

		switch ((first_byte & 0b00111000) >> 3){
			case 0b000: instruction->name = INST_NAME_INC; break;
			case 0b001: instruction->name = INST_NAME_DEC; break;
			case 0b010: instruction->name = INST_NAME_PUSH; break;
			case 0b011: instruction->name = INST_NAME_POP; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11100111) == 0b00000110){
		instruction->type = INST_SEGMENT_REGISTER;
		instruction->name = INST_NAME_PUSH;
	} else if (first_byte == 0b10001111){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_POP;
		instruction->right_side_only = 1;
	} else if ((first_byte & 0b11100111) == 0b00000111){
		instruction->type = INST_SEGMENT_REGISTER;
		instruction->name = INST_NAME_POP;
	} else if ((first_byte & 0b11111110) == 0b10000110){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_XCHG;
	} else if ((first_byte & 0b11111000) == 0b10010000){
		instruction->type = INST_REGISTER;
		instruction->name = INST_NAME_XCHG;
	} else if ((first_byte & 0b11111100) == 0b11100100){
		instruction->type = INST_FIXED_PORT;
		instruction->name = (first_byte & 0b10) ? INST_NAME_OUT : INST_NAME_IN;
	} else if ((first_byte & 0b11111100) == 0b11101100){
		instruction->type = INST_VARIABLE_PORT;
		instruction->name = (first_byte & 0b10) ? INST_NAME_OUT : INST_NAME_IN;
	} else if (first_byte == 0b10001101){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_LEA;
		instruction->direction = 1;
	} else if (first_byte == 0b11000101){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_LDS;
		instruction->direction = 1;
	} else if (first_byte == 0b11000100){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_LES;
		instruction->direction = 1;
		instruction->wide = 1;
	} else if ((first_byte & 0b11111100) == 0b10000100){
		instruction->type = INST_R_M_TO_R_M;
		instruction->name = INST_NAME_TEST;
	} else if ((first_byte & 0b11111110) == 0b10101000){
		instruction->type = INST_IMD_TO_ACC;
		instruction->name = INST_NAME_TEST;
	} else if ((first_byte & 0b11111110) == 0b11110010){
		instruction->type = INST_STRING_MANIPULATION;
		instruction->name = INST_NAME_REP;
	} else if ((first_byte & 0b11110000) == 0b10100000){
		instruction->type = INST_STRING_MANIPULATION;
		switch ((first_byte & 0b00001110) >> 1){
			case 0b010: instruction->name = INST_NAME_MOVS; break;
			case 0b011: instruction->name = INST_NAME_CMPS; break;
			case 0b101: instruction->name = INST_NAME_STOS; break;
			case 0b110: instruction->name = INST_NAME_LODS; break;
			case 0b111: instruction->name = INST_NAME_SCAS; break;
			default: return 0;
		}
	} else if ((first_byte & 0b11110111) == 0b11000010){
		instruction->type = INST_RET_DATA;
		instruction->name = INST_NAME_RET;
	} else if ((first_byte & 0b11111110) == 0b11001100) {
		instruction->type = INST_INT;
		instruction->name = INST_NAME_INT;

		if(!(first_byte & 0b00000001)){
			instruction->data = 3;
		}
	} else if ((first_byte & 0b11100111) == 0b00100110) {
		instruction->type = INST_SEGMENT;
	}else{
		//Instructions with no parameters
		switch (first_byte){
			case 0b11010111: instruction->name = INST_NAME_XLAT; break;
			case 0b10011111: instruction->name = INST_NAME_LAHF; break;
			case 0b10011110: instruction->name = INST_NAME_SAHF; break;
			case 0b10011100: instruction->name = INST_NAME_PUSHF; break;
			case 0b10011101: instruction->name = INST_NAME_POPF; break;
			case 0b00110111: instruction->name = INST_NAME_AAA; break;
			case 0b00100111: instruction->name = INST_NAME_DAA; break;
			case 0b00111111: instruction->name = INST_NAME_AAS; break;
			case 0b00101111: instruction->name = INST_NAME_DAS; break;
			case 0b11010100: {
				if (next_byte == 0b00001010){
					instruction->name = INST_NAME_AAM;
					instruction->type = INST_NO_PARAMS;
					return 2;
				}
			} break;
			case 0b11010101: {
				if (next_byte == 0b00001010){
					instruction->name = INST_NAME_AAD;
					instruction->type = INST_NO_PARAMS;
					return 2;
				}
			} break;
			case 0b10011000: instruction->name = INST_NAME_CBW; break;
			case 0b10011001: instruction->name = INST_NAME_CWD; break;
			case 0b11001011:
			case 0b11000011: instruction->name = INST_NAME_RET; break;
			case 0b11001110: instruction->name = INST_NAME_INTO; break;
			case 0b11001111: instruction->name = INST_NAME_IRET; break;
			case 0b11111000: instruction->name = INST_NAME_CLC; break;
			case 0b11110101: instruction->name = INST_NAME_CMC; break;
			case 0b11111001: instruction->name = INST_NAME_STC; break;
			case 0b11111100: instruction->name = INST_NAME_CLD; break;
			case 0b11111101: instruction->name = INST_NAME_STD; break;
			case 0b11111010: instruction->name = INST_NAME_CLI; break;
			case 0b11111011: instruction->name = INST_NAME_STI; break;
			case 0b11110100: instruction->name = INST_NAME_HLT; break;
			case 0b10011011: instruction->name = INST_NAME_WAIT; break;
			case 0b11110000: instruction->name = INST_NAME_LOCK; break;
		}

		if (instruction->name){
			instruction->type = INST_NO_PARAMS;
			return 1;
		}

		//Conditional jumps
		switch (first_byte){
			case 0b01110101: instruction->name = INST_NAME_JNZ; break;
			case 0b01110100: instruction->name = INST_NAME_JE; break;
			case 0b01111100: instruction->name = INST_NAME_JL; break;
			case 0b01111110: instruction->name = INST_NAME_JLE; break;
			case 0b01110010: instruction->name = INST_NAME_JB; break;
			case 0b01110110: instruction->name = INST_NAME_JBE; break;
			case 0b01111010: instruction->name = INST_NAME_JP; break;
			case 0b01110000: instruction->name = INST_NAME_JO; break;
			case 0b01111000: instruction->name = INST_NAME_JS; break;
			case 0b01111101: instruction->name = INST_NAME_JNL; break;
			case 0b01111111: instruction->name = INST_NAME_JG; break;
			case 0b01110011: instruction->name = INST_NAME_JNB; break;
			case 0b01110111: instruction->name = INST_NAME_JA; break;
			case 0b01111011: instruction->name = INST_NAME_JNP; break;
			case 0b01110001: instruction->name = INST_NAME_JNO; break;
			case 0b01111001: instruction->name = INST_NAME_JNS; break;
			case 0b11100010: instruction->name = INST_NAME_LOOP; break;
			case 0b11100001: instruction->name = INST_NAME_LOOPZ; break;
			case 0b11100000: instruction->name = INST_NAME_LOOPNZ; break;
			case 0b11100011: instruction->name = INST_NAME_JCXZ; break;
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
			byte is_wide = is_inst_wide(instruction);

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