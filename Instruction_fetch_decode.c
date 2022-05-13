// Instruction Fetch & Decode


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//  -->  Union - struct형식으로 변경
typedef struct _instinfo{
	int inst;
	Signals signals;
	Fields fields;
	int pc;
	//아래 변수는 decode 과정 이후에 사용.
	//int aluout;
	//int memout;
	//int sourcereg;
	//int targetreg;
	//int destreg;
	//int destdata;
	//char string[30];
	//int s1data;
	//int s2data;
	//int input1;
	//int input2;
} InstInfo;

//decode 과정에서 사용되는 structure
typedef struct _fields{
	int rd;
	int rs;
	int rt;
	int imm;
	int op;
	int func;
} Fields;

//decode 과정 결과를 ALU로 넘겨주는 signal structure
typedef struct _signals{
	int aluop;
	//아래는 memory, register 접근에서 사용되는 것 같습니다.
	//int mw;
	//int mr;
	//int mtr;
	//int asrc;
	//int btype;
	//int rdst;
	//int rw;
} Signals;


//‘Instruction to be’에 나온 instruction(표 참고하여 값 계산했습니다.)
//위의 표
#define is_bltz    instruction->fields.op == 1
#define is_j    instruction->fields.op == 2
#define is_jal    instruction->fields.op == 3
#define is_beq    instruction->fields.op == 4
#define is_bne    instruction->fields.op == 5
#define is_addi    instruction->fields.op == 8
#define is_slti    instruction->fields.op == 10
#define is_andi    instruction->fields.op == 12
#define is_ori    instruction->fields.op == 13
#define is_xori    instruction->fields.op == 14
#define is_lui    instruction->fields.op == 15
#define is_lb    instruction->fields.op == 32
#define is_lw    instruction->fields.op == 35
#define is_lbu    instruction->fields.op == 36
#define is_sb    instruction->fields.op == 40
#define is_sw    instruction->fields.op == 43
//아래의 표(opcode가 0인 R-format인 경우 해당됨)
//#define is_R_format    instruction->fields.op == 0
#define is_sll    instruction->fields.op == 0 && instruction->fields.op == 0
#define is_srl    instruction->fields.op == 0 && instruction->fields.op == 2
#define is_sra    instruction->fields.op == 0 && instruction->fields.op == 3
#define is_jr    instruction->fields.op == 0 && instruction->fields.op == 8
#define is_syscall    instruction->fields.op == 0 && instruction->fields.op == 12
#define is_mfhi    instruction->fields.op == 0 && instruction->fields.op == 16
#define is_mflo    instruction->fields.op == 0 && instruction->fields.op == 18
#define is_mul    instruction->fields.op == 0 && instruction->fields.op == 24
#define is_add    instruction->fields.func == 0 && instruction->fields.op == 32
#define is_sub    instruction->fields.op == 0 && instruction->fields.op == 34
#define is_and    instruction->fields.op == 0 && instruction->fields.op == 36
#define is_or    instruction->fields.func == 0 && instruction->fields.op == 37
#define is_xor    instruction->fields.func == 0 && instruction->fields.op == 38
#define is_nor    instruction->fields.op == 0 && instruction->fields.op == 39
#define is_slt    instruction->fields.op == 0 && instruction->fields.op == 42

#define nullCheck  if (instruction == NULL || instruction->inst == 0) return;

int instmem[100];  // instruction 개수?에 따라 값 변경.(instruction memory)
int pc;

//alu로 값 넘겨줄 때 사용
//아래의 예처럼 신호를 구성해 사용하면 될 것 같습니다.
// 000 = and, 100 = or, 001 = add, 101 = sub, 010 = not, 011 = xor, 110 = slt
typedef enum {
	INV = -1 ,
	AND = 0,
	ADD = 1,
	NOT = 2,
	XOR = 3,
	OR  = 4,
	SUB = 5,
	SLT = 6
} ALUOps;

typedef enum { R_format = 1, I_format = 2, J_format = 3 } InstFormat;
InstFormat getFormat (InstInfo *instruction) {
	if (is_jal || is_j) return J_format;
	else if (is_lw || is_sw) return I_format;
	else return R_format;
}

void fetch(InstInfo *instruction)
{
	instruction->inst = instmem[pc];        // fetch instruction from instruction memory
	pc += 4; // fetch 과정이 끝나면 pc의 값을 4 증가시켜줌.
}

void decode(InstInfo *instruction)
{
	nullCheck; //instruction 없으면 decode 함수 종료

	int val = instruction->inst; //pc에 따른 명령어값 val에 저장해 사용.

	//fill fields structure
	instruction->fields.op      = (val >> 26) & 0x03f;
	instruction->fields.rs      = (val >> 21) & 0x01f;
	instruction->fields.rt      = (val >> 16) & 0x01f;
	instruction->fields.rd      = (val >> 11) & 0x01f;
	instruction->fields.func    = val & 0x03f;
	instruction->fields.imm     = (((val & 0xffff) << 16) >> 16);

	//위에서 구성한 신호를 이용해 decode를 통한 결과를 ALU로 넣으면 될 것 같습니다.
	//fill signals structure
	if (is_add) {           // add
		instruction->signals.aluop  = ADD;
	} else if (is_or) {     // or
		instruction->signals.aluop  = OR;
	} else if (is_xor) {    // xor
		instruction->signals.aluop  = XOR;
	} else if (is_slt) {    // slt
		instruction->signals.aluop  = SLT;
	} else if (is_lw) {     // lw
		instruction->signals.aluop  = ADD;
	} else if (is_sw) { 	// sw
		instruction->signals.aluop  = ADD;
	} else if (is_j) {      // j
		instruction->signals.aluop  = INV;
	} else if (is_jal) {    // jal
		instruction->signals.aluop  = INV;
	}

	// fill in s1data and input2
	// Set the data up for executing
	// J_format은 j, jal 이므로 해당 과정 불필요.
	// switch (getFormat(instruction)) {
	// 	case (R_format) :
	// 		instruction->input1  = instruction->fields.rs;
	// 		instruction->s1data  = regfile[instruction->fields.rs];
	// 		instruction->input2  = instruction->fields.rt;
	// 		instruction->s2data  = regfile[instruction->fields.rt];
	// 		instruction->destreg = instruction->fields.rd;
	// 		break;
	// 	case (I_format) :
	// 		instruction->input1  = instruction->fields.rs;
	// 		instruction->s1data  = regfile[instruction->fields.rs];
	// 		instruction->input2  = instruction->fields.rt;
	// 		instruction->s2data  = regfile[instruction->fields.rt];
	// 		instruction->destreg = instruction->fields.rt;
	// 		break;
	// }
}

char* rTypeName(int fct) {
	switch (fct) {
		case 0:
			return "sll";
			break;
		case 2:
			return "srl";
			break;
		case 3:
			return "sra";
			break;
		case 4:
			return "sllv";
			break;
		case 6:
			return "srlv";
			break;
		case 7:
			return "srav";
			break;
		case 8:
			return "jr";
			break;
		case 9:
			return "jalr";
			break;
		case 12:
			return "syscall";
			break;
		case 16:
			return "mfhi";
			break;
		case 17:
			return "mthi";
			break;
		case 18:
			return "mflo";
			break;
		case 19:
			return "mtlo";
			break;
		case 24:
			return "mult";
			break;
		case 25:
			return "multu";
			break;
		case 26:
			return "div";
			break;
		case 27:
			return "divu";
			break;
		case 32:
			return "add";
			break;
		case 33:
			return "addu";
			break;
		case 34:
			return "sub";
			break;
		case 35:
			return "subu";
			break;
		case 36:
			return "and";
			break;
		case 37:
			return "or";
			break;
		case 38:
			return "xor";
			break;
		case 39:
			return "nor";
			break;
		case 42:
			return "slt";
			break;
		case 43:
			return "sltu";
			break;
		default:
			return "ERROR";
			break;
	}
}

char* iTypeName(int opc) {
	switch (opc) {
		case 1:
			return "bltz";
			break;
		case 4:
			return "beq";
			break;
		case 5:
			return "bne";
			break;
		case 6:
			return "blez";
			break;
		case 7:
			return "bgtz";
			break;
		case 8:
			return "addi";
			break;
		case 9:
			return "addiu";
			break;
		case 10:
			return "slti";
			break;
		case 11:
			return "sltiu";
			break;
		case 12:
			return "andi";
			break;
		case 13:
			return "ori";
			break;
		case 14:
			return "xori";
			break;
		case 15:
			return "lui";
			break;
		case 32:
			return "lb";
			break;
		case 33:
			return "lh";
			break;
		case 34:
			return "lw";
			break;
		case 36:
			return "lbu";
			break;
		case 37:
			return "lhu";
			break;
		case 40:
			return "sb";
			break;
		case 41:
			return "sh";
			break;
		case 43:
			return "sw";
			break;
		default:
			return "ERROR";
			break;
	}
}

char* getInstName(int opc, int fct) {
	
	// int val = instruction->inst;
	// int opc = val >> 26;
	// int fct = val & 0x3f;

	switch (opc) {
		case 0:   	// R-Type 명령어
			return rTypeName(fct);
			break;
		case 2:   	// J-Type 명령어
			return "j";
			break;
		case 3:		// J-Type 명령어	
			return "jal";
			break;
		default:	// I-Type 명령어
			return iTypeName(opc);
			break;
	}
}

char* getOp(int opc) {
	// int val = instruction->inst;
	// int opc = val >> 26;

	switch (opc) {
		case 0:
			return "R";
			break;
		case 2:
			return "J";
			break;
		case 3:
			return "J";
			break;
		
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 32:
		case 33:
		case 34:
		case 36:
		case 37:
		case 40:
		case 41:
		case 43:
			return "I";
			break;
		default:
			return "ERROR";
			break;
	}
}

