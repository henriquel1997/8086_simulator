static const char* segment_modifier = "";
const char* get_segment_modifier(){
	const char* result = segment_modifier;
	segment_modifier = "";
	return result;
}


const char* get_suffix(struct Instruction instruction){
	if (instruction.right_side_only == 2){
		if (instruction.shift_rotate) return ", cl";
		else return ", 1";
	}
	return "";
}

void get_address_calculation_with_displacement(char* buffer, byte mode, byte r_m, short disp){
	const char* mod = get_segment_modifier();

	if (disp == 0){
		switch (r_m) {
			case 0b000: sprintf(buffer, "%s[bx + si]", mod); break;
			case 0b001: sprintf(buffer, "%s[bx + di]", mod); break;
			case 0b010: sprintf(buffer, "%s[bp + si]", mod); break;
			case 0b011: sprintf(buffer, "%s[bp + di]", mod); break;
			case 0b100: sprintf(buffer, "%s[si]", mod); break;
			case 0b101: sprintf(buffer, "%s[di]", mod); break;
			case 0b110: sprintf(buffer, "%s[bp]", mod); break;
			case 0b111: sprintf(buffer, "%s[bx]", mod); break;
		}
	} else {
		switch (r_m) {
			case 0b000: sprintf(buffer, "%s[bx + si + %d]", mod, disp); break;
			case 0b001: sprintf(buffer, "%s[bx + di + %d]", mod, disp); break;
			case 0b010: sprintf(buffer, "%s[bp + si + %d]", mod, disp); break;
			case 0b011: sprintf(buffer, "%s[bp + di + %d]", mod, disp); break;
			case 0b100: sprintf(buffer, "%s[si + %d]", mod, disp); break;
			case 0b101: sprintf(buffer, "%s[di + %d]", mod, disp); break;
			case 0b110:
				if (mode == 0b00){
					sprintf(buffer, "%s[%d]", mod, disp); 
				} else {
					sprintf(buffer, "%s[bp + %d]", mod, disp); 
				}
				break;
			case 0b111:
				if (mode == 0b00){
					sprintf(buffer, "%s[%d]", mod, disp); 
				} else {
					sprintf(buffer, "%s[bx + %d]", mod, disp); 
				}
				break;
		}
	}
}

const char* get_segment_register_from_code(byte code){
	switch (code & 0b11){
		case 0b00: return "es";
		case 0b01: return "cs";
		case 0b10: return "ss";
		case 0b11: return "ds";
	}
	return 0;
}

const char* get_register_name_from_code(byte wide, byte register_code){
	if (register_code & 0b1000){
		return get_segment_register_from_code(register_code);
	}else if(wide){
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

int get_address_calculation_with_no_displacement(char* buffer, byte mode, byte r_m){
	const char* mod = get_segment_modifier();

	switch (r_m){
		case 0b000: sprintf(buffer, "%s[bx + si]", mod); break;
		case 0b001: sprintf(buffer, "%s[bx + di]", mod); break;
		case 0b010: sprintf(buffer, "%s[bp + si]", mod); break;
		case 0b011: sprintf(buffer, "%s[bp + di]", mod); break;
		case 0b100: sprintf(buffer, "%s[si]", mod); break;
		case 0b101: sprintf(buffer, "%s[di]", mod); break;
		case 0b110: return 0; //Should never get here
		case 0b111: sprintf(buffer, "%s[bx]", mod); break;
	}
	return 1;
}

void print_inst_with_memory_address_displacement(struct Instruction instruction){
	char buffer [20];
	get_address_calculation_with_displacement(buffer, instruction.mode, instruction.r_m, instruction.disp);
	if (instruction.right_side_only) {
		if (instruction.wide){
			printf("%s word %s%s\n", instruction.name, buffer, get_suffix(instruction));
		} else {
			printf("%s byte %s%s\n", instruction.name, buffer, get_suffix(instruction));
		}
		return;
	}

	const char* reg_name = get_register_name_from_code(instruction.wide, instruction.reg);
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
				char address_calc [20];
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
			const char* r_m_name = get_register_name_from_code(instruction.wide, instruction.r_m);
			if (instruction.right_side_only){
				printf("%s %s%s\n", instruction.name, r_m_name, get_suffix(instruction));
				return 1;
			}
			
			const char* reg_name = get_register_name_from_code(instruction.wide, instruction.reg);
			if (instruction.direction) {
				printf("%s %s, %s\n", instruction.name, reg_name, r_m_name);
			} else {
				printf("%s %s, %s\n", instruction.name, r_m_name, reg_name);
			}
		} else {
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
		long offset = instruction.jump_increment + instruction.num_bytes;
		if (offset >= 0){
			printf("%s $+%ld\n", instruction.name, offset);
		} else {
			printf("%s $%ld\n", instruction.name, offset);
		}
	}else if (instruction.type == INST_REGISTER) {
		const char* prefix = "";
		if (instruction.name == xchg_name){
			prefix = "ax, ";
		}
		printf("%s %s%s\n", instruction.name, prefix, get_register_name_from_code(1, instruction.reg));
	}else if (instruction.type == INST_SEGMENT_REGISTER) {
		const char* seg_reg = get_segment_register_from_code(instruction.reg);
		printf("%s %s\n", instruction.name, seg_reg);
	}else if (instruction.type == INST_FIXED_PORT) {
		const char* temp;
		if (instruction.wide){
			temp = instruction.name == in_name ? "%s ax, %d\n" : "%s %d, ax\n";
		} else {
			temp = instruction.name == in_name ? "%s al, %d\n" : "%s %d, al\n";
		}
		printf(temp, instruction.name, instruction.data);
	}else if (instruction.type == INST_VARIABLE_PORT) {
		if (instruction.wide){
			if (instruction.name == in_name){
				printf("%s ax, dx\n", instruction.name);
			} else {
				printf("%s dx, ax\n", instruction.name);
			}
		} else {
			if (instruction.name == in_name){
				printf("%s al, dx\n", instruction.name);
			} else {
				printf("%s dx, al\n", instruction.name);
			}
		}
	}else if (instruction.type == INST_STRING_MANIPULATION) {
		if (instruction.name == rep_name){
			printf("rep\n");
		} else {
			printf("%s%s\n", instruction.name, instruction.wide ? "w" : "b");
		}
	}else if (instruction.type == INST_RET_DATA || instruction.type == INST_INT) {
		printf("%s %d\n", instruction.name, instruction.data);
	}else if (instruction.type == INST_SEGMENT) {
		switch (instruction.reg & 0b11){
			case 0b00: segment_modifier = "es:"; break;
			case 0b01: segment_modifier = "cs:"; break;
			case 0b10: segment_modifier = "ss:"; break;
			case 0b11: segment_modifier = "ds:"; break;
		}
	}else if (instruction.type == INST_NO_PARAMS) {
		printf("%s\n", instruction.name);
	} else {
		return 0;
	}

	return 1;
}

int print_asm(struct Instruction* instructions, long num_instructions){
	printf("bits 16\n");

	for (long i = 0; i < num_instructions; i++){
		if (!print_single_asm(instructions[i])){
			printf("Error printing instruction\n");
			return 0;
		}
	}

	return 1;
}

void print_byte_no_new_line(byte b){
	byte mask = 0b10000000;
	while (mask){
		if (mask & b){
			printf("1");
		} else {
			printf("0");
		}
		mask >>= 1;
	}
}

void print_register_with_binary(struct State* state, const char* name, int index){
	printf("%s: %d [", name, state->registers[index]);
	print_byte_no_new_line(state->registers[index] >> 8);
	printf("_");
	print_byte_no_new_line(state->registers[index] & 0b0000000011111111);
	printf("]\n");
}

void print_registers(struct State* state){
	print_register_with_binary(state, "ax", REG_AX);
	print_register_with_binary(state, "bx", REG_BX);
	print_register_with_binary(state, "cx", REG_CX);
	print_register_with_binary(state, "dx", REG_DX);
	printf("sp: %d\n", state->registers[REG_SP]);
	printf("bp: %d\n", state->registers[REG_BP]);
	printf("si: %d\n", state->registers[REG_SI]);
	printf("di: %d\n", state->registers[REG_DI]);
	printf("es: %d\n", state->registers[REG_ES]);
	printf("ss: %d\n", state->registers[REG_SS]);
	printf("ds: %d\n", state->registers[REG_DS]);
	printf("cs: %d\n", state->registers[REG_CS]);
}