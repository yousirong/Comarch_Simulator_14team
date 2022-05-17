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



/**

typedef union _instInfo {
	int inst;
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
	struct {
		int op;
		int rd;
		int rs;
		int rt;
		int func;
		int imm;
	} fields;
	struct {
		int aluop;
		//아래는 memory, register 접근에서 사용되는 것 같습니다.
		//int mw;
		//int mr;
		//int mtr;
		//int asrc;
		//int btype;
		//int rdst;
		//int rw;
	} signals;
} instInfo;


**/

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
// 정은찬
// rtype  0 2 3 8 12 37
// itype  1 4 5  8 10 32
// 정재윤
// rtype  16 18 (32) (34) (36) (38)
// itype  (12) (13) (14) 15 24  34
// 이준용  나머지랑 인터페이스 함수 넣기
unsigned char* rTypeName(int fct) {
	switch (fct) {
		case 0:
			return "sll"; ////14
		case 2:
			return "srl";  /////
		case 3:
			return "sra"; ////
		case 4:
			return "sllv";
		case 6:
			return "srlv";
		case 7:
			return "srav";
		case 8:
			return "jr";  ///////
		case 9:
			return "jalr";
		case 12:
			return "syscall";  ///////
		case 16:
			return "mfhi";  //////
		case 17:
			return "mthi";
		case 18:
			return "mflo";   //////
		case 19:
			return "mtlo";
		case 24:
			return "mult";
		case 25:
			return "multu";
		case 26:
			return "div";
		case 27:
			return "divu";
		case 32:
			//레지스터 값 가져오기
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//RY = R[Y]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, RY, 8, 0) //c32 == 2, c10 == 0만족하기 위해 C = 8, 제로플래그 0 으로

			return "add";  ////
		case 33:
			return "addu";
		case 34:
			//레지스터 값 가져오기
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//RY = R[Y]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, RY, 9, 0) //c32 == 2, c10 == 1만족하기 위해 C = 9, 제로플래그 0 으로 
			return "sub";  /////
		case 35:
			return "subu";
		case 36:
			//레지스터 값 가져오기
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//RY = R[Y]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, RY, a, 0) //A >> 2 = 11, A & 3 = 0 결국 logicOperation(RX,RY,0)가능
			return "and";   //////
		case 37:
			//레지스터 값 가져오기
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//RY = R[Y]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, RY, b, 0) //B >> 2 = 11, B & 3 = 1 결국 logicOperation(RX,RY,1)가능
			return "or";    /////
		case 38:
			//레지스터 값 가져오기
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//RY = R[Y]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, RY, c, 0) //C >> 2 = 11, C & 3 = 2 결국 logicOperation(RX,RY,2)가능
			return "xor";   /////
		case 39:
			//레지스터 값 가져오기
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//RY = R[Y]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, RY, d, 0) //D >> 2 = 11, D & 3 = 3 결국 logicOperation(RX,RY,3)가능
			return "nor"; //////
		case 42:
			return "slt";   //////
		case 43:
			return "sltu";
		default:
			return "ERROR";
	}
}

unsigned char* typeName(int opc) {
	switch (opc) {
		case 1:
			return "bltz"; //// 15
        case 2:  // j
			return "j";   ////   J-Type
		case 3:
			return "jal"; ////	J-Type
		case 4:
			return "beq";   /////
		case 5:
			return "bne";   /////
		case 6:
			return "blez";
		case 7:
			return "bgtz";
		case 8:
			return "addi";  /////
		case 9:
			return "addiu";
		case 10:
			return "slti";   /////
		case 11:
			return "sltiu";
		case 12:
			//X는 레지스터 값, Y는 상수

			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, Y, 8, 0) //c32 == 2, c10 == 0만족하기 위해 C = 8, 제로플래그 0 으로
			return "andi";    /////
		case 13:
			//X는 레지스터 값, Y는 상수

			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, Y, b, 0) //B >> 2 = 11, B & 3 = 1 결국 logicOperation(RX,Y,1)가능
			return "ori";    /////
		case 14:
			//X는 레지스터 값, Y는 상수

			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//ALU(RX, Y, c, 0) //C >> 2 = 11, C & 3 = 2 결국 logicOperation(RX,Y,2)가능
			return "xori";   /////
		case 15:
			//X는 상위의 16비트, Y는 상수


			return "lui"; ////
		case 24:

			return "mul";     ///////
		case 32:
			return "lb";   /////
		case 33:
			return "lh";
		case 34:
			//X 레지스터 주소, Y 워드
			//RX = R[X]	//레지스터의 X위치에서 저장된 값 가져오기
			//워드 읽어와서 Y값 지정
			//ALU(RX, Y, 8, 0) //c32 == 2, c10 == 0만족하기 위해 C = 8, 제로플래그 0 으로
			//-> 레지스터에 Y값 저장

			return "lw";    //////
		case 36:
			return "lbu";    /////
		case 37:
			return "lhu";
		case 40:
			return "sb";   /////
		case 41:
			return "sh";
		case 43:
			return "sw";   //////
		default:
			return "ERROR";
	}
}

unsigned char* getInstName(int opc, int fct, int* isImmediate) {  // 디버깅

	// int val = instruction->inst;
	// int opc = val >> 26;
	// int fct = val & 0x3f;

	switch (opc) {
		case 0:   	// R-Type 명령어
			return rTypeName(fct);
		//case 2:   	// J-Type 명령어
		//	return "j";        /////
		//case 3:		// J-Type 명령어
		//	return "jal";   //////
		default:	// J-Type or I-Type 명령어
			return typeName(opc);
	}
}
//
unsigned char getOp(int opc) {

	char format;

	if (opc == 0) {
		format = 'R';
	}
	else if (opc == 2 || opc == 3) {
		format = 'j';
	}
	else {
		format = 'I';
	}

	return format;
}

