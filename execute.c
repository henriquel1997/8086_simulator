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

int execute_instruction(struct State* state){
	if (state->current_instruction == state->num_instructions) return 0;

	struct Instruction instruction = state->instructions[state->current_instruction++];

	if (instruction.type == INST_R_M_TO_R_M) {
		if (instruction.mode == 0b00) {
			//Memory mode with no displacement*
			if (instruction.r_m != 0b110) {
				/*char address_calc [20];
				get_address_calculation_with_no_displacement(address_calc, instruction.mode, instruction.r_m);
				if (instruction.right_side_only) {
					if (instruction.wide){
						printf("%s word %s%s\n", instruction.name, address_calc, get_suffix(instruction));
					} else {
						printf("%s byte %s%s\n", instruction.name, address_calc, get_suffix(instruction));
					}
					return 1;
				}
				
				const char* reg_name = get_register_name_from_code(instruction.wide, instruction.reg);
				
				if (instruction.direction) {
					printf("%s %s, %s\n", instruction.name, reg_name, address_calc);
				} else {
					printf("%s %s, %s\n", instruction.name, address_calc, reg_name);
				}*/
				printf("INST_R_M_TO_R_M Mode 00b with R_M != 110b not implemented\n");
			} else {
				/*//There actually is a 16-bit displacement if r_m is 0b110
				print_inst_with_memory_address_displacement(instruction);*/
				printf("INST_R_M_TO_R_M Mode 00b with R_M == 110b not implemented\n");
			}
		} else if (instruction.mode == 0b01 || instruction.mode == 0b10) {
			/*//Memory mode with 8-bit(0b01) or 16-bit(0b10) displacement
			print_inst_with_memory_address_displacement(instruction);*/
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

			if (instruction.wide){
				state->registers[to_index] = state->registers[from_index];
			} else {
				int from_is_low = from_index < 4;
				unsigned short value = state->registers[from_index % 4];
				value = from_is_low ? (value & 0b0000000011111111) : (value >> 8);

				int to_is_low = to_index < 4;
				if (to_is_low){
					state->registers[to_index] = (state->registers[to_index] & 0b1111111100000000) + (value & 0b0000000011111111);
				} else {
					to_index = to_index % 4;
					state->registers[to_index] = (state->registers[to_index] & 0b0000000011111111) + (value << 8);
				}
			}
			return 1;
		}
	} else if (instruction.type == INST_IMD_TO_R_M) {
		if (instruction.mode == 0b11) {
			/*//Register mode
			printf("%s %s, %d\n", instruction.name, get_register_name_from_code(instruction.wide, instruction.r_m), instruction.data);*/
			printf("INST_IMD_TO_R_M Mode 11b not implemented\n");
		} else {
			/*//Memory mode
			char buffer[20];
			if (instruction.mode == 0b00 && instruction.r_m != 0b110) {
				get_address_calculation_with_no_displacement(buffer, instruction.mode, instruction.r_m);
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
			}*/
			printf("INST_IMD_TO_R_M Mode everything but 11b not implemented\n");
		}
	} else if (instruction.type == INST_IMD_TO_REG) {
		int reg_index = get_register_index(instruction.reg);
		if (instruction.wide) {
			state->registers[reg_index] = instruction.data;
		} else {
			int is_low = reg_index < 4;
			if (is_low){
				state->registers[reg_index] = (state->registers[reg_index] & 0b1111111100000000) + (instruction.data & 0b0000000011111111);
			} else {
				reg_index = reg_index % 4;
				state->registers[reg_index] = (state->registers[reg_index] & 0b0000000011111111) + (instruction.data << 8);
			}
		}
		return 1;
	} else if (instruction.type == INST_MEM_TO_ACC_VV) {
		/*const char* acc;
		if (instruction.wide) {
			acc = "ax";
		} else {
			acc = "al";
		}

		if (instruction.direction) {
			printf("%s [%d], %s\n", instruction.name, instruction.address, acc);
		} else {
			printf("%s %s, [%d]\n", instruction.name, acc, instruction.address);
		}*/
		printf("INST_MEM_TO_ACC_VV not implemented\n");
	}
	return 0;
}