#ifndef DECLARATIONS_H
#define DECLARATIONS_H

const char* mov_name = "mov";
const char* push_name = "push";
const char* pop_name = "pop";
const char* xchg_name = "xchg";
const char* in_name = "in";
const char* out_name = "out";
const char* inc_name = "inc";
const char* dec_name = "dec";
const char* test_name = "test";
const char* rep_name = "rep";
const char* call_name = "call";
const char* jmp_name = "jmp";
const char* ret_name = "ret";

#define byte unsigned char

struct String {
	char* data;
	long size;
};

enum InstructionType {
	INST_R_M_TO_R_M,
	INST_IMD_TO_R_M,
	INST_IMD_TO_REG,
	INST_IMD_TO_ACC,
	INST_MEM_TO_ACC_VV,
	INST_R_M_TO_SEG_VV,
	INST_CONDITIONAL_JUMP,
	INST_REGISTER,
	INST_SEGMENT_REGISTER,
	INST_FIXED_PORT,
	INST_VARIABLE_PORT,
	INST_STRING_MANIPULATION,
	INST_RET_DATA,
	INST_INT,
	INST_SEGMENT,
	INST_NO_PARAMS
};

struct Instruction {
	enum InstructionType type;
	const char* name;
	char num_bytes;
	union {
		byte direction;
		byte sign;
		byte shift_rotate;
	};
	union {
		byte wide;
		byte loop_while_zero;
	};
	byte mode;
	byte reg;
	byte r_m;
	short disp;
	unsigned short data;
	unsigned short address;
	char jump_increment;
	int right_side_only;
};

enum Register {
	REG_AX, //Primary accumulator
	REG_CX, //Counter accumulator
	REG_DX, //Extended accumulator
	REG_BX, //Base accumulator
	REG_SP, //Stack pointer
	REG_BP, //Base pointer
	REG_SI, //Source index
	REG_DI, //Destination index
	REG_ES, //Extra segment
	REG_CS, //Code segment
	REG_SS, //Stack segment
	REG_DS, //Data segment
	NUM_REGS
};

struct State {
	unsigned short registers[NUM_REGS];
	long num_instructions;
	struct Instruction* instructions;
	long current_instruction;
};

#endif