#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#define byte unsigned char

struct String {
	char* data;
	long size;
};

enum InstructionName {
	INST_NAME_NO_NAME,
	INST_NAME_MOV,
	INST_NAME_ADD,
	INST_NAME_OR,
	INST_NAME_ADC,
	INST_NAME_SBB,
	INST_NAME_AND,
	INST_NAME_SUB,
	INST_NAME_XOR,
	INST_NAME_CMP,
	INST_NAME_INC,
	INST_NAME_DEC,
	INST_NAME_CALL,
	INST_NAME_JMP,
	INST_NAME_PUSH,
	INST_NAME_TEST,
	INST_NAME_NOT,
	INST_NAME_NEG,
	INST_NAME_MUL,
	INST_NAME_IMUL,
	INST_NAME_DIV,
	INST_NAME_IDIV,
	INST_NAME_ROL,
	INST_NAME_ROR,
	INST_NAME_RCL,
	INST_NAME_RCR,
	INST_NAME_SHL,
	INST_NAME_SHR,
	INST_NAME_SAR,
	INST_NAME_POP,
	INST_NAME_XCHG,
	INST_NAME_OUT,
	INST_NAME_IN,
	INST_NAME_LEA,
	INST_NAME_LDS,
	INST_NAME_LES,
	INST_NAME_REP,
	INST_NAME_MOVS,
	INST_NAME_CMPS,
	INST_NAME_STOS,
	INST_NAME_LODS,
	INST_NAME_SCAS,
	INST_NAME_RET,
	INST_NAME_INT,
	INST_NAME_XLAT,
	INST_NAME_LAHF,
	INST_NAME_SAHF,
	INST_NAME_PUSHF,
	INST_NAME_POPF,
	INST_NAME_AAA,
	INST_NAME_DAA,
	INST_NAME_AAS,
	INST_NAME_DAS,
	INST_NAME_AAM,
	INST_NAME_AAD,
	INST_NAME_CBW,
	INST_NAME_CWD,
	INST_NAME_INTO,
	INST_NAME_IRET,
	INST_NAME_CLC,
	INST_NAME_CMC,
	INST_NAME_STC,
	INST_NAME_CLD,
	INST_NAME_STD,
	INST_NAME_CLI,
	INST_NAME_STI,
	INST_NAME_HLT,
	INST_NAME_WAIT,
	INST_NAME_LOCK,
	INST_NAME_JNZ,
	INST_NAME_JE,
	INST_NAME_JL,
	INST_NAME_JLE,
	INST_NAME_JB,
	INST_NAME_JBE,
	INST_NAME_JP,
	INST_NAME_JO,
	INST_NAME_JS,
	INST_NAME_JNL,
	INST_NAME_JG,
	INST_NAME_JNB,
	INST_NAME_JA,
	INST_NAME_JNP,
	INST_NAME_JNO,
	INST_NAME_JNS,
	INST_NAME_LOOP,
	INST_NAME_LOOPZ,
	INST_NAME_LOOPNZ,
	INST_NAME_JCXZ
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
	enum InstructionName name;
	enum InstructionType type;
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

enum Flags {
	FLAG_CF, //Carry Flag
	FLAG_AF, //Auxiliary Flag
	FLAG_SF, //Sign Flag
	FLAG_ZF, //Zero Flag
	FLAG_PF, //Parity Flag
	FLAG_OF, //Overflow Flag
	NUM_FLAGS
};

struct State {
	unsigned short registers[NUM_REGS];
	byte flags[NUM_FLAGS];
	long num_instructions;
	struct Instruction* instructions;
	long current_instruction;
};

byte is_inst_wide(struct Instruction instruction){
	return instruction.name == INST_NAME_MOV ? instruction.wide : (instruction.wide && !instruction.sign);
}

const char* get_inst_name(struct Instruction instruction) {
	switch (instruction.name) {
		case INST_NAME_NO_NAME: return "no_name";
		case INST_NAME_MOV: return "mov";
		case INST_NAME_ADD: return "add";
		case INST_NAME_OR: return "or";
		case INST_NAME_ADC: return "adc";
		case INST_NAME_SBB: return "sbb";
		case INST_NAME_AND: return "and";
		case INST_NAME_SUB: return "sub";
		case INST_NAME_XOR: return "xor";
		case INST_NAME_CMP: return "cmp";
		case INST_NAME_INC: return "inc";
		case INST_NAME_DEC: return "dec";
		case INST_NAME_CALL: return "call";
		case INST_NAME_JMP: return "jmp";
		case INST_NAME_PUSH: return "push";
		case INST_NAME_TEST: return "test";
		case INST_NAME_NOT: return "not";
		case INST_NAME_NEG: return "neg";
		case INST_NAME_MUL: return "mul";
		case INST_NAME_IMUL: return "imul";
		case INST_NAME_DIV: return "div";
		case INST_NAME_IDIV: return "idiv";
		case INST_NAME_ROL: return "rol";
		case INST_NAME_ROR: return "ror";
		case INST_NAME_RCL: return "rcl";
		case INST_NAME_RCR: return "rcr";
		case INST_NAME_SHL: return "shl";
		case INST_NAME_SHR: return "shr";
		case INST_NAME_SAR: return "sar";
		case INST_NAME_POP: return "pop";
		case INST_NAME_XCHG: return "xchg";
		case INST_NAME_OUT: return "out";
		case INST_NAME_IN: return "in";
		case INST_NAME_LEA: return "lea";
		case INST_NAME_LDS: return "lds";
		case INST_NAME_LES: return "les";
		case INST_NAME_REP: return "rep";
		case INST_NAME_MOVS: return "movs";
		case INST_NAME_CMPS: return "cmps";
		case INST_NAME_STOS: return "stos";
		case INST_NAME_LODS: return "lods";
		case INST_NAME_SCAS: return "scas";
		case INST_NAME_RET: return "ret";
		case INST_NAME_INT: return "int";
		case INST_NAME_XLAT: return "xlat";
		case INST_NAME_LAHF: return "lahf";
		case INST_NAME_SAHF: return "sahf";
		case INST_NAME_PUSHF: return "pushf";
		case INST_NAME_POPF: return "popf";
		case INST_NAME_AAA: return "aaa";
		case INST_NAME_DAA: return "daa";
		case INST_NAME_AAS: return "aas";
		case INST_NAME_DAS: return "das";
		case INST_NAME_AAM: return "aam";
		case INST_NAME_AAD: return "aad";
		case INST_NAME_CBW: return "cbw";
		case INST_NAME_CWD: return "cwd";
		case INST_NAME_INTO: return "into";
		case INST_NAME_IRET: return "iret";
		case INST_NAME_CLC: return "clc";
		case INST_NAME_CMC: return "cmc";
		case INST_NAME_STC: return "stc";
		case INST_NAME_CLD: return "cld";
		case INST_NAME_STD: return "std";
		case INST_NAME_CLI: return "cli";
		case INST_NAME_STI: return "sti";
		case INST_NAME_HLT: return "hlt";
		case INST_NAME_WAIT: return "wait";
		case INST_NAME_LOCK: return "lock";
		case INST_NAME_JNZ: return "jnz";
		case INST_NAME_JE: return "je";
		case INST_NAME_JL: return "jl";
		case INST_NAME_JLE: return "jle";
		case INST_NAME_JB: return "jb";
		case INST_NAME_JBE: return "jbe";
		case INST_NAME_JP: return "jp";
		case INST_NAME_JO: return "jo";
		case INST_NAME_JS: return "js";
		case INST_NAME_JNL: return "jnl";
		case INST_NAME_JG: return "jg";
		case INST_NAME_JNB: return "jnb";
		case INST_NAME_JA: return "ja";
		case INST_NAME_JNP: return "jnp";
		case INST_NAME_JNO: return "jno";
		case INST_NAME_JNS: return "jns";
		case INST_NAME_LOOP: return "loop";
		case INST_NAME_LOOPZ: return "loopz";
		case INST_NAME_LOOPNZ: return "loopnz";
		case INST_NAME_JCXZ: return "jcxz";
	}
	return 0;
}

#endif