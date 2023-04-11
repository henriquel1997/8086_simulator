int get_register_index(byte register_code){
	if (register_code & 0b1000){
		switch (register_code & 0b11){
			case 0b00: return REG_ES;
			case 0b01: return REG_CS;
			case 0b10: return REG_SS;
			case 0b11: return REG_DS;
		}
	}else{
		switch(register_code & 0b111){
			case 0b000: return REG_AX;
			case 0b001: return REG_CX;
			case 0b010: return REG_DX;
			case 0b011: return REG_BX;
			case 0b100: return REG_SP;
			case 0b101: return REG_BP;
			case 0b110: return REG_SI;
			case 0b111: return REG_DI;
		}
	}
	return 0;
}

void set_arithmetic_flags(enum InstructionName name, unsigned short reg_after, struct State* state){
	switch (name) {
		case INST_NAME_ADD:
		case INST_NAME_SUB:
		case INST_NAME_CMP:
			break;
		default:
			return;
	}

	state->flags[FLAG_SF] = reg_after >> 15 & 1;
	state->flags[FLAG_ZF] = reg_after == 0;

	//Brian Kernighan’s Algorithm
	int num_bits = 0;
	unsigned short n = reg_after & 0b11111111;
	while (n){
		n &= n - 1;
		num_bits++;
	}
	state->flags[FLAG_PF] = num_bits % 2 == 0;
}

unsigned short get_memory_index(struct Instruction instruction, struct State* state){
	unsigned short memory_index = 0;

	if(instruction.mode != 0b11){
		switch (instruction.r_m) {
			case 0b000: memory_index = state->registers[REG_BX] + state->registers[REG_SI]; break;
			case 0b001: memory_index = state->registers[REG_BX] + state->registers[REG_DI]; break;
			case 0b010: memory_index = state->registers[REG_BP] + state->registers[REG_SI]; break;
			case 0b011: memory_index = state->registers[REG_BP] + state->registers[REG_DI]; break;
			case 0b100: memory_index = state->registers[REG_SI]; break;
			case 0b101: memory_index = state->registers[REG_DI]; break;
			case 0b110: memory_index = state->registers[REG_BP]; break;
			case 0b111: memory_index = state->registers[REG_BX]; break;
		}
	}

	memory_index += instruction.disp;
	return memory_index;
}

int execute_store(struct Instruction instruction, struct State* state, unsigned short value){
	unsigned short memory_index = get_memory_index(instruction, state);

	if (instruction.wide){
		unsigned char* data_pointer = (unsigned char*) &value;
		state->memory[memory_index + 0] = data_pointer[0];
		state->memory[memory_index + 1] = data_pointer[1];
	} else {
		state->memory[memory_index] = value;
	}

	return 1;
}

int execute_load(struct Instruction instruction, struct State* state){
	unsigned short memory_index = get_memory_index(instruction, state);
	int register_index = get_register_index(instruction.reg);

	if (instruction.wide){
		unsigned char* reg_pointer = (unsigned char*) &state->registers[register_index];
		reg_pointer[0] = state->memory[memory_index + 0];
		reg_pointer[1] = state->memory[memory_index + 1];
	} else {
		printf("Not wide load not implemented\n");
		return 0;
	}

	return 1;
}

int execute_instruction(struct Instruction instruction, struct State* state){
#define set_carry_flag_add(operand1, operand2) state->flags[FLAG_CF] = (((((unsigned int) (operand1)) + ((unsigned int) operand2)) >> 16) > 0)
#define set_carry_flag_add_byte(operand1, operand2) state->flags[FLAG_CF] = (((((unsigned short) (operand1)) + ((unsigned short) operand2)) >> 8) > 0)
#define set_carry_flag_sub(operand1, operand2) state->flags[FLAG_CF] = ((operand1) < (operand2))

//TODO: This aux flag is stil not working properly on listing 47, seems to work fine on listing 50 though
#define set_carry_aux_flag_add(operand1, operand2) state->flags[FLAG_AF] = (((((unsigned short)(((unsigned char*) &(operand1))[0])) + ((unsigned short)(((unsigned char*) &(operand2))[0]))) >> 8) > 0)
#define set_carry_aux_flag_sub(operand1, operand2) state->flags[FLAG_AF] = (((unsigned char*) &(operand1))[0] < ((unsigned char*) &(operand2))[0])

#define set_overflow_flag_add(operand1, operand2, result) state->flags[FLAG_OF] = ((((operand1) >> 15) == ((operand2) >> 15)) && (((operand2) >> 15) != ((result) >> 15)))
#define set_overflow_flag_sub(operand1, operand2, result) state->flags[FLAG_OF] = ((((operand1) >> 15) != ((operand2) >> 15)) && (((operand2) >> 15) == ((result) >> 15)))

	if (instruction.type == INST_R_M_TO_R_M) {
		if (instruction.mode == 0b00) {
			if (instruction.name == INST_NAME_MOV) {
				if (instruction.direction) {
					if (!execute_load(instruction, state)) return 0;
				} else {
					unsigned short value = state->registers[get_register_index(instruction.reg)];
					if (!execute_store(instruction, state, value)) return 0;
				}
			} else if (instruction.name == INST_NAME_ADD) {
				int register_index = get_register_index(instruction.reg);
				unsigned short value = state->registers[register_index];

				if (instruction.direction){
					if (!execute_load(instruction, state)) return 0;
					state->registers[register_index] += value;
				} else {
					printf("INST_R_M_TO_R_M Mode 0b00 not implemented for ADD\n");
					return 0;
					//if (!execute_store(instruction, state, value)) return 0;
				}
			} else {
				printf("INST_R_M_TO_R_M Mode 0b00 not implemented\n");
				return 0;
			}
			return 1;
		} else if (instruction.mode == 0b01 || instruction.mode == 0b10) {
			//Memory mode with 8-bit(0b01) or 16-bit(0b10) displacement
			printf("INST_R_M_TO_R_M Mode 01b or 10b not implemented\n");
		} else if (instruction.mode == 0b11) {
			int to_index;
			int from_index;
			if (instruction.direction){
				to_index = get_register_index(instruction.reg);
				from_index = get_register_index(instruction.r_m);
			} else {
				to_index = get_register_index(instruction.r_m);
				from_index = get_register_index(instruction.reg);
			}

			unsigned short dest_before = state->registers[to_index];
			unsigned short dest_after = 0;

			if (instruction.name == INST_NAME_MOV) {
				if (instruction.wide) {
					state->registers[to_index] = state->registers[from_index];
				} else {
					int from_is_low = from_index < 4;
					unsigned short value = state->registers[from_index % 4];
					value = from_is_low ? (value & 0b0000000011111111) : (value >> 8);

					int to_is_low = to_index < 4;
					if (to_is_low) {
						state->registers[to_index] = (state->registers[to_index] & 0b1111111100000000) + (value & 0b0000000011111111);
					} else {
						to_index = to_index % 4;
						state->registers[to_index] = (state->registers[to_index] & 0b0000000011111111) + (value << 8);
					}
				}
			}else if (instruction.name == INST_NAME_ADD) {
				if (instruction.wide) {
					dest_after = state->registers[to_index] + state->registers[from_index];

					set_carry_flag_add(state->registers[to_index], state->registers[from_index]);
					set_carry_aux_flag_add(state->registers[to_index], state->registers[from_index]);
					set_overflow_flag_add(state->registers[to_index], state->registers[from_index], dest_after);
					
					state->registers[to_index] = dest_after;
				} else {
					printf("INST_R_M_TO_R_M not wide for add not implemented\n");
					return 0;
				}
			} else if (instruction.name == INST_NAME_SUB || instruction.name == INST_NAME_CMP) {
				if (instruction.wide) {
					dest_after = state->registers[to_index] - state->registers[from_index];

					set_carry_flag_sub(state->registers[to_index], state->registers[from_index]);
					set_carry_aux_flag_sub(state->registers[to_index], state->registers[from_index]);
					set_overflow_flag_sub(state->registers[to_index], state->registers[from_index], dest_after);
				} else {
					printf("INST_R_M_TO_R_M not for wide %s not implemented\n", get_inst_name(instruction));
					return 0;
				}

				if (instruction.name == INST_NAME_SUB){
					state->registers[to_index] = dest_after;
				}
			} else {
				printf("INST_R_M_TO_R_M Mode 11b not implemented for %s\n", get_inst_name(instruction));
				return 0;
			}


			set_arithmetic_flags(instruction.name, dest_after, state);
			return 1;
		}
	} else if (instruction.type == INST_IMD_TO_R_M) {
		int reg_index = get_register_index(instruction.r_m);

		if (instruction.mode == 0b11) {
			//Register mode

			unsigned short dest_before = state->registers[reg_index];
			unsigned short dest_after = 0;
			if (instruction.name == INST_NAME_ADD) {
				if (instruction.wide) {
					if (is_inst_wide(instruction)){
						dest_after = state->registers[reg_index] + instruction.data;

						set_carry_flag_add(state->registers[reg_index], instruction.data);
						set_carry_aux_flag_add(state->registers[reg_index], instruction.data);
						set_overflow_flag_add(state->registers[reg_index], instruction.data, dest_after);
					} else {
						signed char* signed_data = (signed char*)&instruction.data;
						dest_after = state->registers[reg_index] + (*signed_data);

						set_carry_flag_add_byte(state->registers[reg_index], (*signed_data));
						set_carry_aux_flag_add(state->registers[reg_index], (*signed_data));
						set_overflow_flag_add(state->registers[reg_index], (*signed_data), dest_after);
					}

					state->registers[reg_index] = dest_after;
				} else {
					printf("INST_IMD_TO_R_M not wide for add not implemented\n");
					return 0;
				}
			} else if (instruction.name == INST_NAME_SUB || instruction.name == INST_NAME_CMP) {
				if (instruction.wide) {
					if (is_inst_wide(instruction)){
						dest_after = state->registers[reg_index] - instruction.data;

						set_carry_flag_sub(state->registers[reg_index], instruction.data);
						set_carry_aux_flag_sub(state->registers[reg_index], instruction.data);
						set_overflow_flag_sub(state->registers[reg_index], instruction.data, dest_after);
					} else {
						signed char* signed_data = (signed char*)&instruction.data;
						dest_after = state->registers[reg_index] - (*signed_data);

						set_carry_flag_sub(state->registers[reg_index], (*signed_data));
						set_carry_aux_flag_sub(state->registers[reg_index], (*signed_data));
						set_overflow_flag_sub(state->registers[reg_index], (*signed_data), dest_after);
					}
				} else {
					printf("INST_IMD_TO_R_M not wide for %s not implemented\n", get_inst_name(instruction));
					return 0;
				}

				if (instruction.name == INST_NAME_SUB){
					state->registers[reg_index] = dest_after;
				}
			} else {
				printf("INST_IMD_TO_R_M Mode 11b not implemented for %s\n", get_inst_name(instruction));
				return 0;
			}

			set_arithmetic_flags(instruction.name, dest_after, state);
			return 1;
		} else {
			//Memory mode
			if (instruction.name == INST_NAME_MOV){
				if (!execute_store(instruction, state, instruction.data)) return 0;
			} else {
				printf("INST_IMD_TO_R_M Mode everything but 11b not implemented for %s\n", get_inst_name(instruction));
				return 0;
			}

			return 1;
		}
	} else if (instruction.type == INST_IMD_TO_REG) {
		int reg_index = get_register_index(instruction.reg);

		if (instruction.name == INST_NAME_MOV) {
			if (instruction.wide) {
				state->registers[reg_index] = instruction.data;
			} else {
				int is_low = reg_index < 4;
				if (is_low) {
					state->registers[reg_index] = (state->registers[reg_index] & 0b1111111100000000) + (instruction.data & 0b0000000011111111);
				} else {
					reg_index = reg_index % 4;
					state->registers[reg_index] = (state->registers[reg_index] & 0b0000000011111111) + (instruction.data << 8);
				}
			}
		} else {
			printf("INST_IMD_TO_REG not implemented for %s\n", get_inst_name(instruction));
			return 0;
		}

		return 1;
	} else if (instruction.type == INST_MEM_TO_ACC_VV) {
		printf("INST_MEM_TO_ACC_VV not implemented\n");
	} else if (instruction.type == INST_CONDITIONAL_JUMP) {
#define execute_jump() state->registers[REG_IP] += instruction.jump_increment

		if (instruction.name == INST_NAME_JNZ) {
			if (!state->flags[FLAG_ZF]) execute_jump();
		} else if (instruction.name == INST_NAME_JE) {
			if(state->flags[FLAG_ZF]) execute_jump();
		} else if (instruction.name == INST_NAME_JP) {
			if(state->flags[FLAG_PF]) execute_jump();
		} else if (instruction.name == INST_NAME_JB) {
			if(state->flags[FLAG_CF]) execute_jump();
		} else if (instruction.name == INST_NAME_LOOPNZ) {
			state->registers[REG_CX]--;
			if(state->registers[REG_CX] != 0 && !state->flags[FLAG_ZF]) execute_jump();
		} else {
			printf("INST_CONDITIONAL_JUMP not implemented for %s\n", get_inst_name(instruction));
			return 0;
		}

		return 1;
	} else {
		printf("Instruction type %d not implemented\n", instruction.type);
	}
	return 0;
}