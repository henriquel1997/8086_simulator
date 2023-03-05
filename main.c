#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcpy

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
		case 0b000: return "[bx + si]";
		case 0b001: return "[bx + di]";
		case 0b010: return "[bp + si]";
		case 0b011: return "[bp + di]";
		case 0b100: return "[si]";
		case 0b101: return "[di]";
		case 0b110: return 0; //Should never get here
		case 0b111: return "[bx]";
	}
	return 0;
}

void get_address_calculation_with_displacement(char* buffer, byte mode, byte r_m, byte d_low, byte d_high){
	short data = 0;
	byte* data_pointer = (byte*) &data;
	
	if (d_high == 0){
		char signed_data = (char)d_low;
		data = signed_data;
	} else {
		data_pointer[0] = d_low;
		data_pointer[1] = d_high;
	}

	if (data == 0){
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
			case 0b000: sprintf(buffer, "[bx + si + %d]", data); break;
			case 0b001: sprintf(buffer, "[bx + di + %d]", data); break;
			case 0b010: sprintf(buffer, "[bp + si + %d]", data); break;
			case 0b011: sprintf(buffer, "[bp + di + %d]", data); break;
			case 0b100: sprintf(buffer, "[si + %d]", data); break;
			case 0b101: sprintf(buffer, "[di + %d]", data); break;
			case 0b110:
				if (mode == 0b00){
					sprintf(buffer, "[%d]", data); 
				} else {
					sprintf(buffer, "[bp + %d]", data); 
				}
				break;
			case 0b111:
				if (mode == 0b00){
					sprintf(buffer, "[%d]", data); 
				} else {
					sprintf(buffer, "[bx + %d]", data); 
				}
				break;
		}
	}
}

void print_mov_with_memory_address_displacement(byte direction, byte wide, byte mode, byte reg, byte r_m, byte disp_low, byte disp_high){
	const char* reg_name = get_register_name_from_code(wide, reg);
	char buffer [20];
	get_address_calculation_with_displacement(buffer, mode, r_m, disp_low, disp_high);
	if (direction){
		printf("mov %s, %s\n", reg_name, buffer);
	} else {
		printf("mov %s, %s\n", buffer, reg_name);
	}
}

int decode(struct String code){
	printf("bits 16\n");

	for(long i = 0; i < code.size; i++){
		byte first_byte = code.data[i];
		print_byte(first_byte);

		if ((first_byte & 0b11111100) == 0b10001000) {
			//MOV Instruction
			byte direction = first_byte & 0b00000010;
			byte wide 		= first_byte & 0b00000001;

			if (i + 1 >= code.size) {
				printf("Missing second byte of MOV\n");
				return 0;
			}

			byte second_byte = code.data[++i];
			print_byte(second_byte);


			byte mode = second_byte >> 6;
			byte reg = (second_byte >> 3) & 0b111;
			byte r_m = (second_byte >> 0) & 0b111;

			if (mode == 0b00) {
				//Memory mode with no displacement*
				if (r_m != 0b110){
					const char* reg_name = get_register_name_from_code(wide, reg);
					const char* address_calc = get_address_calculation_with_no_displacement(mode, r_m);
					if (direction){
						printf("mov %s, %s\n", reg_name, address_calc);
					} else {
						printf("mov %s, %s\n", address_calc, reg_name);
					}
				} else {
					//*There actually is a 16-bit displacement if r_m is 0b110
					byte disp_low = code.data[++i];
					print_byte(disp_low);
					byte disp_high = code.data[++i];
					print_byte(disp_high);
					
					print_mov_with_memory_address_displacement(direction, wide, mode, reg, r_m, disp_low, disp_high);
				}
			} else if (mode == 0b01 || mode == 0b10) {
				//Memory mode with 8-bit(0b01) or 16-bit(0b10) displacement
				byte disp_low = code.data[++i];
				print_byte(disp_low);

				byte disp_high = 0;
				if (mode == 0b10) {
					disp_high = code.data[++i];
					print_byte(disp_high);
				}

				print_mov_with_memory_address_displacement(direction, wide, mode, reg, r_m, disp_low, disp_high);
			} else if (mode == 0b11) {
				//Register mode
				if (direction) {
					printf("mov %s, %s\n", get_register_name_from_code(wide, reg), get_register_name_from_code(wide, r_m));
				} else {
					printf("mov %s, %s\n", get_register_name_from_code(wide, r_m), get_register_name_from_code(wide, reg));
				}
			} else {
				printf("Mode not implemented: ");
				print_byte(mode);
				return -1;
			}
		}else if ((first_byte & 0b11111110) == 0b11000110) {
			//Immediate to Register or Memory MOV
			byte wide = first_byte & 0b00000001;

			byte second_byte = code.data[++i];
			print_byte(second_byte);

			byte mode = second_byte >> 6;
			byte r_m = (second_byte >> 0) & 0b111;

			unsigned short data = 0;
			byte* data_pointer = (byte*) &data;

			char buffer[20];
			if (mode == 0b00 && r_m != 0b110){
				byte data_low = code.data[++i];
				print_byte(data_low);
				data_pointer[0] = data_low;

				if (wide){
					byte data_high = code.data[++i];
					print_byte(data_high);
					data_pointer[1] = data_high;
				}

				strcpy(buffer, get_address_calculation_with_no_displacement(mode, r_m));
			} else {
				byte disp_low = code.data[++i];
				print_byte(disp_low);
				byte disp_high = code.data[++i];
				print_byte(disp_high);

				byte data_low = code.data[++i];
				print_byte(data_low);
				data_pointer[0] = data_low;

				if (wide){
					byte data_high = code.data[++i];
					print_byte(data_high);
					data_pointer[1] = data_high;
				}

				get_address_calculation_with_displacement(buffer, mode, r_m, disp_low, disp_high);
			}

			if (wide){
				printf("mov %s, word %d\n", buffer, data);
			} else {
				printf("mov %s, byte %d\n", buffer, data);
			}

		}else if ((first_byte & 0b11110000) == 0b10110000) {
			//Immediate to Register MOV
			byte wide = first_byte & 0b00001000;
			byte reg = first_byte & 0b00000111;

			byte data_low = code.data[++i];
			print_byte(data_low);

			unsigned short data = 0;
			byte* data_pointer =  (byte*) &data;
			data_pointer[0] = data_low;

			if (wide){
				byte data_high = code.data[++i];
				print_byte(data_high);
				data_pointer[1] = data_high;
			}

			printf("mov %s, %d\n", get_register_name_from_code(wide, reg), data);
		}else if ((first_byte & 0b11111100) == 0b10100000) {
			//Memory to Accumulator or Accumulator to Memory
			byte direction = first_byte & 0b00000010;
			byte wide 		= first_byte & 0b00000001;

			unsigned short address = 0;
			byte* data_pointer =  (byte*) &address;

			byte address_low = code.data[++i];
			print_byte(address_low);
			data_pointer[0] = address_low;

			byte address_high = code.data[++i];
			print_byte(address_high);
			data_pointer[1] = address_high;

			const char* acc;
			if (wide){
				acc = "ax";
			} else {
				acc = "al";
			}

			if (direction){
				printf("mov [%d], %s\n", address, acc);
			} else {
				printf("mov %s, [%d]\n", acc, address);
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

#ifdef DEBUG
	printf("Num Code Bytes: %ld\n", code.size);
#endif

	decode(code);
	return 0;
}