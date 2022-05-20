#define _CRT_SECURE_NO_WARNINGS
#define M_SIZE 1000
#define REG_SIZE 32
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <cstdio>


/*제어용 전역변수*/
char fileName[100];
const int check = 1;
static FILE* pFile = NULL;
static int continueTask = 1;
static unsigned int R[32], PC;    // 모든 레지스터와 PC 연산할때 쓰는 함수
static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];
static unsigned int var = 0xAABBCCDD; //MEM 초기화에 활용할 변수



unsigned char* rTypeName(int fct);
unsigned char* iTypeName(int opc, int* isImmediate);
unsigned char* getInstName(int opc, int fct, int* isImmediate);
//char* getOp(int opc);

char* regArr[32] = { "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
"$t8","$t9","$k0","$k1","$gp","$sp","$s8","$ra" };

/*각 format에 따른 구조체 형식이다. 강의자료 참고함.
RI = r-format 구조체
II = i-format 구조체
JI = j-format 구조체*/
union itype {
	unsigned int I;
	struct rFormat {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int rd : 5;
		unsigned int funct : 6;
	}RI;
	struct iFormat {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int offset : 16;
	}II;
	struct jFormat {
		unsigned int opcode : 6;
		unsigned int jumpAddr : 26;
	}JI;
}IR;
/*시뮬레이터에 사용될 함수 선언*/
void printNotice();
int checkArgument1(int lenCode, char type);
int checkArgument2(int lenCode, char type);
int checkArgument3(int lenCode, int type);

/*로직에 사용할 함수 선언*/
void initializeRegister();//레지스터 초기화
void setRegister(unsigned int regNum, unsigned int val);//원하는 레지스터 값을 변경할 수 있는 함수.
void setMemory(char* offset, char* val);//원하는 값으로 해당 메모리에 접근하여 값을 변경하는 함수.
void updatePC(unsigned int addr);//현재 pc값을 원하는 값으로 변경하는 함수이다.
void loadInitTask();//바이너리 파일을 로드하고 메모리에 ?적재하는 작업을 담당하는 함수.
void showRegister();//인터페이스 'r'실행시 반환되는 함수
void startGoTask();//인터페이스 'g'실행시 반환되는 함수
void startStepTask();//인터페이스 's'실행시 반환되는 함수   → debugging 함수 포함되어있음





void openBinaryFile(char* filePath);   // l 명령어 실행시 filePath를 받아서 바이너리 파일 여는 함수
unsigned int To_BigEndian(unsigned int x);  // 빅엔디안 변경 함수 => hex값
unsigned char getOp(int opc);  // opcode 확인 함수
// binary to decimal 한 값을 int값으로 저장함
//unsigned char* getInstName(int opc, int fct, int* isImmediate);   // debugging 함수
void instExecute(int opc, int fct, int* isImmediate);   // instruction 실행함수
int MEM(unsigned int A, int V, int nRW, int S); // memory access함수
// ALU
int logicOperation(int X, int Y, int C);
int addSubtract(int X, int Y, int C);
int shiftOperation(int V, int Y, int C);
int checkZero(int S);
int checkSetLess(int X, int Y);
int ALU(int X, int Y, int C, int* Z);   // R-format 명령어에서 ALU함수 필요



int main(){
    //시뮬레이터 변수 설정
    char cmdLine[50];
    int lenCode;
    int cmdLen;
    int cmdErr;

    //시뮬레이터 사용법 출력
    printNotice();

    //레지스터 초기화 함수
    // Initialize
	initializeRegister();

    while(1){
        //COMAND 변수 초기화
        lenCode = 0; //명령어의 인자 수
        cmdLen = 0; //명령어의 자리수(1~2자리 식별용)
        cmdErr = 0;
        //char *cmdArr[10] = {NULL, };
    /*명령입력받기*/
        printf("Enter a command.\n>>> ");
        gets(cmdLine);
        // 공백 문자를 기준으로 문자열을 자르고 포인터 반환
        char* ptr = strtok(cmdLine, " ");

        // while (ptr != NULL)            // 자른 문자열이 나오지 않을 때까지 반복
        // {
        //     cmdArr[lenCode] = ptr;      // 문자열을 자른 뒤 메모리 주소를 문자열 포인터 배열에 저장
        //     lenCode++;

        //     ptr = strtok(NULL, " ");   // 다음 문자열을 잘라서 포인터를 반환
        // }

        // if(cmdArr[0] != NULL){
        //     cmdLen = strlen(cmdArr[0]);
        // }

		// 명령어 코드
		char* cmdArr = ptr;
		if (ptr != NULL) {  // 명령어가 null이 아니면 반환
			// 명령어 코드 글자 수
			cmdLen = strlen(cmdArr);
		}
        if(cmdLen == 1){ //명령어가 한글자일 때
            switch (*cmdArr){
        /*l 명령어*/
            case 'l':
                if(checkArgument2(lenCode, 'l') == 1) //명령어 유효성검사
                    break;
                // load program
				// ptr은 Filename 문자열을 가리킨다.
				ptr = strtok(NULL, " ");
				char* filePath = NULL;
				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) l C:\\pub\\as_ex01_arith.bin\n");
				}
				else {
					filePath = ptr;
					openBinaryFile(filePath);
				}
				break;


                break;
        /*j 명령어*/
            case 'j':
                if(checkArgument2(lenCode, 'j') == 1) //명령어 유효성검사
                    break;

                // jump, 입력한 위치에서 simulator 실행을 준비한다.
				// ptr은 프로그램 시작 위치 문자열을 가리킨다.
				ptr = strtok(NULL, " ");
				char* newAddr = NULL;
				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) j 0x40000000\n");
				}
				else {
					newAddr = ptr;
					updatePC(strtol(newAddr, NULL, 16));
				}
				break;

        /*g 명령어*/
            case 'g':
                if(checkArgument1(lenCode, 'g') == 1) //명령어 유효성검사
                    break;

                /*Go program, 현재PC위치에서 simulator가 명령어를 끝까지 처리한다.
				이때 사용되는 함수는 startGoTask()이다.*/
				if (pFile != NULL) {
					startGoTask();
				}
				else {
					printf("Error: Load Binary File in advance!\n");
				}
				break;

        /*s 명령어*/
            case 's':{
                if(checkArgument1(lenCode, 's') == 1) //명령어 유효성검사
                    break;

                /* Step, 명령어에 의하여 변경된 레지스터, 메모리 정보를 출력한다.
				이때 사용되는 함수는 startStepTask()이다.*/
				if (pFile != NULL) {
					startStepTask();
					showRegister();
				}
				else {
					printf("Error: Load Binary File in advance!\n");
				}
				break; }
        /*m 명령어*/
            case 'm':
                if(checkArgument3(lenCode, 1) == 1) //명령어 유효성검사
                    break;

               // View memory
				// ptr은 start 문자열을 가리킨다., startAddr~endAddr범위의 메모리 내용 출력
				ptr = strtok(NULL, " ");

				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) m 0x10000000 0x1000FFFF\n");
				}
				else {
					char* start = ptr;

					// ptr은 end 문자열을 가리킨다.
					ptr = strtok(NULL, " ");
					if (ptr == NULL) {
						printf("Error: Not enough arguments.\n");
						printf("ex) m 0x10000000 0x1000FFFF\n");
					}
					else {
						char* end = ptr;

						unsigned int startAddr = strtol(start, NULL, 16);
						unsigned int endAddr = strtol(end, NULL, 16);

						for (unsigned int i = startAddr; i <= endAddr; i = i + 4) {
							printf("[0x%08x] => 0x%x\n", i, MEM(i, var, 0, 2));
						}
					}
				}
				break;
        /*r 명령어*/
            case 'r':
                if(checkArgument1(lenCode, 'r') == 1) //명령어 유효성검사
                    break;

                /* View register, 현재 레지스터 내용 출력
				이때 사용되는 함수는 showRegister()이다.*/
				showRegister();
				break;

        /*x 명령어*/
            case 'x':
                printf("Terminate program.\n");
                exit(1);
                break;

        /*정의되지 않은 명령어 오류처리: 명령어 1개짜리*/
            default:
                printf("Error: Enter a valid command.");
                break;
            }

        }
        else if(cmdLen == 2)
        {
			if (strcmp(cmdArr, "sr") == 0) {
				// 특정 레지스터의 값 설정
				char* regNum = NULL;
				char* regVal = NULL;
				// ptr은 register number 문자열을 가리킨다.
				ptr = strtok(NULL, " ");

				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) sr 1 20\n");
				}
				else {
					regNum = ptr;
					// ptr은 value 문자열을 가리킨다.
					ptr = strtok(NULL, " ");
					if (ptr == NULL) {
						printf("Error: Not enough arguments.\n");
						printf("ex) sr 1 20\n");
					}
					else {
						regVal = ptr;
						setRegister(atoi(regNum), strtol(regVal, NULL, 16));
					}
				}
			}
			else if (strcmp(cmdArr, "sm") == 0) {
				// 메모리 특정 주소의 값 설정
				// ptr은 start 문자열을 가리킨다.
				ptr = strtok(NULL, " ");

				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) sm 0xFFFFFFFF 20\n");
				}
				else {
					printf("OK\n");
					char* memLoc = ptr;

					// ptr은 end 문자열을 가리킨다.
					ptr = strtok(NULL, " ");
					if (ptr == NULL) {
						printf("Error: Not enough arguments.\n");
						printf("ex) sm 0xFFFFFFFF 20\n");
					}
					else {
						printf("OK\n");
						char* memVal = ptr;
					}
				}
			}
			else {
				printf("Error: Invalid command arguments.\n");
			}
		}
         else{
            printf("Error: Enter a valid command.");
        }

        printf("\n\n");
    }
    return 0;
}
        // { //명령어가 두글자일 때

        // /*sr 명령어*/
        //     if(!strcmp(cmdArr[0], "sr")){
        //         if(checkArgument3(lenCode, 2) == 1){ //명령어 유효성검사
        //             printf("\n\n");
        //             continue;
        //         }
        //         else{
        //             //함수삽입
        //         }
        //     }

        // /*sm 명령어*/
        //     else if(!strcmp(cmdArr[0], "sm")){
        //         if(checkArgument3(lenCode, 3) == 1){ //명령어 유효성검사
        //             printf("\n\n");
        //             continue;
        //         }
        //         else{
        //             //함수삽입
        //         }
        //     }
        // /*정의되지 않은 명령어 오류처리: 명령어 2개짜리*/
        //     else{
        //         printf("Error: 올바른 명령어를 입력해주세요.");
        //     }

        // }
    /*정의되지 않은 명령어 오류처리: 명령어 입력x인 경우 + 정의되지 않은 명령어인 경우*/

//인터페이스 's'실행시 반환되는 함수
void startStepTask() {
	printf("current value : %x\n", MEM(PC, var, 0, 2));
	unsigned instBinary = MEM(PC, var, 0, 2);
	PC = PC + 4;
	/* Instruction Decode */
	// 명령어 타입(R, I, J) 체크 및
	// 명령어 type에 따라 분기하여 추출
	switch (getOp((instBinary >> 26) & 0x3F))
	{
	case 'R':
		// R-Format 기준, opcode 추출
		IR.RI.opcode = (instBinary >> 26) & 0x3F;
		// rs 추출
		IR.RI.rs = (instBinary >> 21) & 0x1F;
		// rt 추출
		IR.RI.rt = (instBinary >> 16) & 0x1F;
		// rd 추출
		IR.RI.rd = (instBinary >> 11) & 0x1F;
		// funct 추출
		IR.RI.funct = instBinary & 0x3F;

		instExecute(IR.RI.opcode, IR.RI.funct, NULL);

		// 명령어 구분에 따른 결과 출력 변화
		if (IR.RI.opcode == 0 && IR.RI.funct == 12) {
			// syscall 명령어 case
			printf("%s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL));
			continueTask = 0;
		}
		else if (IR.RI.opcode == 0 && IR.RI.funct == 8) {
			// jr 명령어 case
			printf("%s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rs]);
		}
		else {
			printf("%s %s %s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rd], regArr[IR.RI.rs], regArr[IR.RI.rt]);
		}
		break;
	case 'I':
		// I-Format 기준, opcode 추출
		IR.II.opcode = (instBinary >> 26) & 0x3F;
		int isImmediate = 0; // immediate 값이면 1로 바꿈-----------------------------------------------------------------------------------------
		// rs 추출
		IR.II.rs = (instBinary >> 21) & 0x1F;
		// rt 추출
		IR.II.rt = (instBinary >> 16) & 0x1F;
		// offset/immediate value 추출
		IR.II.offset = instBinary & 0xFFFF;

		instExecute(IR.II.opcode, 0, &isImmediate); //void instExecute(int opc, int fct, int* isImmediate) opcode일 때 fct는 0으로 통일

		// offset인지 immediate value 인지에 따른 결과 출력 변화
		printf("%s", getInstName(IR.II.opcode, 0, &isImmediate));
		if (isImmediate == 1) {
			printf(" %s %s %d\n\n", regArr[IR.II.rt], regArr[IR.II.rs], IR.II.offset);
		}
		else {
			printf(" %s %d(%s)\n\n", regArr[IR.II.rt], IR.II.offset, regArr[IR.II.rs]);
		}
		break;
	case 'J':
		// J-Format 기준, opcode 추출
		IR.JI.opcode = (instBinary >> 26) & 0x3F;
		// jump target address 추출
		IR.JI.jumpAddr = instBinary & 0x3FFFFFF;

		instExecute(IR.JI.opcode, 0, NULL);

		// 결과 출력
		printf("%s %d\n\n", getInstName(IR.JI.opcode, 0, NULL), IR.JI.jumpAddr);
		break;
	default:
		break;
	}
}
//인터페이스 'g'실행시 반환되는 함수
void startGoTask() {

	while (continueTask) {
		/* Instruction Fetch */
		printf("current value : %x\n", MEM(PC, var, 0, 2));
		unsigned instBinary = MEM(PC, var, 0, 2);
		PC = PC + 4;
		/* Instruction Decode */
		// 명령어 타입(R, I, J) 체크 및
		// 명령어 type에 따라 분기하여 추출
		switch (getOp((instBinary >> 26) & 0x3F))
		{
		case 'R':
			// R-Format 기준, opcode 추출
			IR.RI.opcode = (instBinary >> 26) & 0x3F;
			// rs 추출
			IR.RI.rs = (instBinary >> 21) & 0x1F;
			// rt 추출
			IR.RI.rt = (instBinary >> 16) & 0x1F;
			// rd 추출
			IR.RI.rd = (instBinary >> 11) & 0x1F;
			// funct 추출
			IR.RI.funct = instBinary & 0x3F;

			instExecute(IR.RI.opcode, IR.RI.funct, NULL);


			// 명령어 구분에 따른 결과 출력 변화 (For Debugging)
			if (IR.RI.opcode == 0 && IR.RI.funct == 12) {
				// syscall 명령어 case
				printf("%s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL));
				continueTask = 0;
			}
			else if (IR.RI.opcode == 0 && IR.RI.funct == 8) {
				// jr 명령어 case
				printf("%s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rs]);
			}
			else {
				printf("%s %s %s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rd], regArr[IR.RI.rs], regArr[IR.RI.rt]);
			}

			break;
		case 'I':
			// I-Format 기준, opcode 추출
			IR.II.opcode = (instBinary >> 26) & 0x3F;
			int isImmediate = 0; // immediate 값이면 1로 바꿈
			// rs 추출
			IR.II.rs = (instBinary >> 21) & 0x1F;
			// rt 추출
			IR.II.rt = (instBinary >> 16) & 0x1F;
			// offset/immediate value 추출
			IR.II.offset = instBinary & 0xFFFF;

			instExecute(IR.II.opcode, 0, &isImmediate);


			// offset인지 immediate value 인지에 따른 결과 출력 변화 (For Debugging)
			printf("%s", getInstName(IR.II.opcode, 0, &isImmediate));
			if (isImmediate == 1) {
				printf(" %s %s %d\n\n", regArr[IR.II.rt], regArr[IR.II.rs], IR.II.offset);
			}
			else {
				printf(" %s %d(%s)\n\n", regArr[IR.II.rt], IR.II.offset, regArr[IR.II.rs]);
			}

			break;
		case 'J':
			// J-Format 기준, opcode 추출
			IR.JI.opcode = (instBinary >> 26) & 0x3F;
			// jump target address 추출
			IR.JI.jumpAddr = instBinary & 0x3FFFFFF;

			instExecute(IR.JI.opcode, 0, NULL);


			// 결과 출력 (For Debugging)
			printf("%s %d\n\n", getInstName(IR.JI.opcode, 0, NULL), IR.JI.jumpAddr);

			break;
		default:
			break;
		}
	}
}
//인터페이스 'r'실행시 반환되는 함수
void showRegister() {
	// 16진수로 출력
	printf("[REGISTER]\n");
	for (int i = 0; i < REG_SIZE; i++) {
		printf("$%d=\t0x%x\n", i, R[i]);
	}
	printf("PC=\t0x%x\n", PC);
}
//시뮬레이터 사용법 출력함수
void printNotice(){
    printf("\t\t\t*Command Input Format*\n");
    printf("l Executable File Path\t\t:The file will be uploaded to the simulator memory.\n");
    printf("j Program Start Location\t:Prepare to run the simulator at the location you entered\n");
    printf("g\t\t\t\t:At the current pc location, the simulator handles the command to the end.\n");
    printf("s\t\t\t\t:Process one command and stop to receive user commands.\n");
    printf("m StartAddress EndAddress\t:Print the memory contents of the start~end range.\n");
    printf("r\t\t\t\t:Print the contents of the current register.\n");
    printf("x\t\t\t\t:Terminate the simulator program.\n");
    printf("sr Register Number Value\t:Set the value of a particular register.\n");
    printf("sm Location Value\t\t:Set the value of a memory-specific address.\n");
    printf("---------------------------------------------------------------------------------------------\n");
}
/*Instruction Decode단계 => getOp() = instruction의 Op code, 즉 operation의 종류를 반환하는 함수
0이면 R-type, 2또는3이면 J-type, 그 외는 I-type으로 처리함.
instExecute() = op, function code에 따라 명령어를 분류하고 해당되는 연산을 실행하는 함수이다.*/
unsigned char getOp(int opc) {
	char format;
	// R-Format instruction
	if (opc == 0) {
		format = 'R';
	}
	// J-Format instruction
	else if ((opc == 2) || (opc == 3)) {
		format = 'J';
	}
	// I-Format instruction
	else {
		format = 'I';
	}
	return format;
}
/*올바르지 않은 인자 확인 함수*/
int checkArgument1(int lenCode, char type){ //인자가 1개인 명령어들
    int result = 0;

    if(lenCode >= 2){
        printf("Error: Keep the format of the command.\n");

        switch (type)
        {

        case 'g':
            printf("\tex) g");
            result = 1;
            break;

        case 's':
            printf("\tex) s");
            result = 1;
            break;

        case 'r':
            printf("\tex) r");
            result = 1;
            break;

        default:
            break;
        }
    }

    return result;
}

int checkArgument2(int lenCode, char type){ //인자가 2개인 명령어들
    int result = 0;

    switch (type)
    {
    case 'l':
        if(lenCode == 2){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) l Path of Executable File Name");
        break;

    case 'j':
        if(lenCode == 2){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) j Program Start Location");
        result = 1;
        break;

    default:
        break;
    }


    return result;
}

int checkArgument3(int lenCode, int type){ //인자가 3개인 명령어들
    int result = 0;

    switch (type)
    {
/*m 명령어*/
    case 1:
        if(lenCode == 3){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) m startAddress endAddress");
        result = 1;
        break;

/*sr 명령어*/
    case 2:
        if(lenCode == 3){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) sr register number value");
        result = 1;
        break;

/*sm 명령어*/
    case 3:
        if(lenCode == 3){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) sm location value");
        result = 1;
        break;
    default:
        break;
    }

    return result;
}
//----------------------------------------------------------------             ----------------------------------------------------------------------
//     l filePath
unsigned char* getInstName(int opc, int fct, int* isImmediate) {  // 디버깅

	// int val = instruction->inst;
	// int opc = val >> 26;
	// int fct = val & 0x3f;

	switch (opc) {
		case 0:   	// R-Type 명령어
			return rTypeName(fct);
		case 2:   	// J-Type 명령어
			return "j";        /////
		case 3:		// J-Type 명령어
			return "jal";   //////
		default:	// I-Type 명령어
			return iTypeName(opc, isImmediate);
	}
}
// 바이너리 파일 여는 함수   -> l명령어
// 레지스터 초기화
void initializeRegister() {
	for (int i = 0; i < REG_SIZE; i++) {
		// 32bit
		R[i] = 0x00000000;
	}
	// PC 초기값 설정
	PC = 0x00400000;
	// SP 초기값 설정
	R[29] = 0x80000000;
}
void openBinaryFile(char* filePath) {
	//err = fopen_s(&pFile, "as_ex01_arith.bin", "rb");
	//err = fopen_s(&pFile, "as_ex02_logic.bin", "rb");
	//err = fopen_s(&pFile, "as_ex03_ifelse.bin", "rb");

	// File Validation TEST

    // FILE* testFile = NULL;
    //--------------------------------------------------------이부분 고치기 file 못읽음
	FILE* testFile = fopen( filePath, "rb");
	if (testFile == NULL) {
		printf("Cannot open file\n");
		return;// return 1에서 삭제
	}
	unsigned int data;
	unsigned int data1 = 0xAABBCCDD;
	if (fread(&data, sizeof(data1), 1, testFile) != 1)
		exit(1);
	fclose(testFile);

	// Load Real File
	fopen( filePath, "rb");
	printf("The Binary File Has Been Loaded Successfully.\n");

	// Load Init Task (메모리 적재)
	loadInitTask();
}
/*To_BigEndian = 데이터가 있을때 큰 단위가 앞으로 나오게 만드는 함수.
이진수에서는 상위비트로 갈 수록 값이 커지기 때문에 앞쪽으로 갈 수록 단위가 커진다.*/
unsigned int To_BigEndian(unsigned int x)
{
	unsigned int result = (x & 0xFF) << 24;

	result |= ((x >> 8) & 0xFF) << 16;
	result |= ((x >> 16) & 0xFF) << 8;
	result |= ((x >> 24) & 0xFF);

	return result;
}

/*Instruction Fetch단계 =>loadInintTask() = 바이너리 파일을 load하고 메모리에 적재하는 작업을 담당하는 함수*/
void loadInitTask() {
	updatePC(0x400000);
	setRegister(29, 0x80000000);

	//printf("\n%s\n", loadedFilePath);
	unsigned int data;
	unsigned int data1 = 0xAABBCCDD;
	unsigned int numInst;
	unsigned int numData;

	// Read the number of Instructions
	fread(&numInst, sizeof(data1), 1, pFile);
	numInst = To_BigEndian(numInst);
	// Read the number of Datas
	fread(&numData, sizeof(data1), 1, pFile);
	numData = To_BigEndian(numData);

	printf("size of Instructions : %d\n", numInst);
	printf("size of Datas : %d\n", numData);

	unsigned int memAddr = 0x00400000;
	unsigned int dataAddr = 0x10000000;

	for (int i = 0; i < numInst; i++) {
		if (fread(&data, sizeof(data1), 1, pFile) != 1)
			exit(1);
		// 명령어 메모리 적재
		data = To_BigEndian(data);
		printf("Instruction = %08x\n", data);

		MEM(memAddr, data, 1, 2);
		memAddr = memAddr + 4;
	}

	for (int i = 0; i < numData; i++) {
		if (fread(&data, sizeof(data1), 1, pFile) != 1)
			exit(1);
		data = To_BigEndian(data);
		// 데이터 메모리 적재
		printf("Data = %08x\n", data);

		MEM(dataAddr, data, 1, 2);
		dataAddr = dataAddr + 4;
	}
}

//현재 pc값을 원하는 값으로 변경하는 함수이다.
void updatePC(unsigned int addr) {
	PC = addr;
}
//원하는 레지스터 값을 변경할 수 있는 함수.
void setRegister(unsigned int regNum, unsigned int val) {

	R[regNum] = val;
}

//원하는 값으로 해당 메모리에 접근하여 값을 변경하는 함수.
void setMemory(char* offset, char* val) {

	R[atoi(offset)] = strtol(val, NULL, 16);
}


//Memory Access 부분이다. int MEM(unsigned int A, int V, int nRW, int S); // memory access함수
int MEM(unsigned int A, int V, int nRW, int S) {
	unsigned int sel, offset;
	unsigned char* pM;
	sel = A >> 20;
	offset = A & 0xFFFFF;

	if (sel == 0x004) pM = progMEM;			// Program memory
	else if (sel == 0x100) pM = dataMEM;	// Data memory
	else if (sel == 0x7FF) pM = stakMEM;	// Stack
	else {

		printf("No memory\n");
		// 에러 케이스 테스트를 위해 전체 프로그램을 종료하지 않고
		// 함수만 종료한다
		return 1;
	}

	if (S == 0) {
		// Byte
		if (nRW == 0) {
			// Read
			return pM[offset];
		}
		else if (nRW == 1) {
			// Write
			pM[offset] = V;
			return 1;
		}
		else {
			printf("nRW input error\n");
			return 1;
			//exit(1);
		}
	}
	else if (S == 1) {
		// Half word
		if (offset % 2 != 0)	// Half-word-aligned Check
		{
			printf("Not an half-word-aligned address input!\n");
			return 1;
			//exit(1);
		}
		if (nRW == 0) {
			// Read
			int result = (pM[offset] << 8) + (pM[offset + 1]);
			return result;
		}
		else if (nRW == 1) {
			// Write
			pM[offset] = (V >> 8) & 0xFF;
			pM[offset + 1] = V & 0xFF;
		}
		else {
			printf("nRW input error\n");
			return 1;
			//exit(1)
		}
	}
	else if (S == 2) {
		// Word
		if (offset % 4 != 0)	// Word-aligned Check
		{
			printf("Not an word-aligned address input!\n");
			return 1;
			//exit(1)
		}
		if (nRW == 0) {
			// Read
			int result = (pM[offset] << 24) + (pM[offset + 1] << 16) + (pM[offset + 2] << 8) + (pM[offset + 3]);
			return result;
		}
		else if (nRW == 1) {
			// Write
			pM[offset] = (V >> 24) & 0xFF;
			pM[offset + 1] = (V >> 16) & 0xFF;
			pM[offset + 2] = (V >> 8) & 0xFF;
			pM[offset + 3] = V & 0xFF;
		}
		else {
			printf("nRW input error\n");
			return 1;
			//exit(1)
		}
	}
	else {//S가 유효하지 않은 값일 경우 오류처리
		printf("Size input error\n");
		return 1;
		//exit(1)
	}
}
/*R-format + (I-format 또는 J-format)라는 2가지 format으로 나누었다.
switch문을 사용해 case마다 명령어 처리했다.
각 instruction은 MIPS simulator 강의자료 참고함*/
void instExecute(int opc, int fct, int* isImmediate) {
	//zero flag 선언
	int Z;
	Z = 0;

	int sub;

    if(opc != 0){
        // I-Format 또는 J-Format 인 경우
        switch(opc){
            case 1:
			// bltz
			// 0보다 작으면 이동

			

			// ALU의 checkSetLess연산(0과 비교)
			// if문을 통해 1, 0을 구분해도 되는지 모르겠습니다.
			if(ALU(R[IR.RI.rs], 0, 0x4, &Z)) { 
				// 32bit로 sign extension 한 immediate 상수값 << 2 + ( PC + 4 )  
				//---> mips에서는 PC의 비트수와 offset의 비트수가 다르기떄문에 offset을 32비트로 만들어서 사용한다고 하는데 C언어에서는 어떻게 처리되는지 모르겠습니다. (bltz, beq, bne)
				updatePC((MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2)<<2)+(PC+4)); 
				break;
			}
			else {
				updatePC(PC+4);
				break;
			}
			
            case 2:
            //j
            updatePC(IR.JI.jumpAddr);  // L로 이동
            break;
            case 3:

            case 4:
			// beq
			// 같으면 이동

			// 먼저 sub연산으로 두개의 레지스터값이 같은지 확인하였고 (같은값 = 0, 다른값 != 0)
			// checkZero함수로 1, 0을 판별하도록 하였는데 따로 함수를 가져와 판별해도 되는지 혹 단순히 if문만으로 판별해도되는지 모르겠습니다.
			
			
			sub = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x9, &Z);   //ALU의 sub연산


			if(checkZero(sub)) { // if sub ==0 , 32bit로 sign extension 한 immediate 상수값 << 2 + ( PC + 4 ) 
				updatePC((MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2)<<2)+(PC+4)); 
				break;
			}
			else {  // if sub != 0 , 다음 명령어 실행을 위해 PC + 4를 해준다.
				updatePC(PC+4);
				break;
			}
			

            case 5:
			// bne
			// 다르면 이동

			// 먼저 sub연산으로 두개의 레지스터값이 같은지 확인하였고 (같은값 = 0, 다른값 != 0)
			// checkZero함수로 1, 0을 판별하도록 하였는데 따로 함수를 가져와 판별해도 되는지 혹 단순히 if문만으로 판별해도되는지 모르겠습니다.
			
			sub = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x9, &Z);   //ALU의 sub연산

			if(!(checkZero(sub))) { 		// if sub !=0 , 32bit로 sign extension 한 immediate 상수값 << 2 + ( PC + 4 )
				updatePC((MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2)<<2)+(PC+4)); 
				break;
			}
			else {  // if sub == 0 , 다음 명령어 실행을 위해 PC + 4를 해준다.
				updatePC(PC+4);
				break;
			}

            case 8:
			// addi
				
				R[IR.RI.rt] = ALU(R[IR.RI.rs], MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2), 0x8, &Z);   //ALU의 addi연산
				break;

            case 10:
			// slti
				
				R[IR.RI.rt] = ALU(R[IR.RI.rs], MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2), 0x4, &Z);   // ALU의 checkSetLess연산
				break;

            case 12: 
				//andi
				
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2); //메모리에서 상수값i 받아오기
				R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.II.rt], 0x8, &Z);//ALU의 addi연산
				//R[IR.RI.rt] = ALU(R[IR.RI.rs], MEM(R[IR.II.rs] + IR.II.offset, NULL, 0, 2), 12, &Z);
            case 13:
				//ori
				
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2); //메모리에서 상수값i 받아오기
				R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.II.rt], 0xb, &Z);//ALU의 ori연산
			case 14:
				//xori
			
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2); //메모리에서 상수값i 받아오기
				R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.II.rt], 0xc, &Z);//ALU의 ori연산
            case 15:
            case 32:
			// lb

            case 35:
				// lw
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2);
			break;
            case 36:
            case 40:
            case 43:
				// sw
				MEM(R[IR.II.rs] + IR.II.offset, R[IR.II.rt], 1, 2);
				break;
            default:
				// not found
				break;
        }
    }else {
		// R-Format 인 경우
		switch (fct) {
		case 0: {
			// sll
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x1, &Z);
			break; }
		case 2: {
			// srl
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x2, &Z);
			break; }
		case 3: {
			// sra
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x3, &Z);
			break; }
		case 8:
			// jr
			updatePC(R[31]);	// go to $ra
			break;
		case 12:
			// syscall
			continueTask = 0;
			break;
		case 16:
			// mfhi
			break;
		case 18:
			// mflo
			break;
		case 24:
			// mul
			
			break;
		case 32: {
			// add
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x8, &Z);
			break; }
		case 34: {
			// sub
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x9, &Z);
			break; }
		case 36: {
			// and
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 12, &Z);
			break; }
		case 37: {
			// or
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 13, &Z);
			break; }
		case 38: {
			// xor
		
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 14, &Z);
			break; }
		case 39: {
			// nor
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 15, &Z);
			break; }
		case 42: {
			// slt
		
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 4, &Z);
			break; }
		default:
			// NOT FOUND!
			break;
		}
	}
}

// ex ) add $t1, $t2, $t3
int ALU(int X, int Y, int C, int* Z) {
    // X = 4-bit input number
    // Y = 4-bit input number
    // C = carry into LSB position

    // Z = Zero Flag
    //Zero Flag:  This bit is updated as a result of all operations.
    //If the result of an operation is zero, then Z is asserted.
    //If the result of an operation is not zero, then Z is 0.
	int c32, c10;
	int ret;

	c32 = (C >> 2) & 3;
	c10 = C & 3;
	if (c32 == 0) {
		//shift
		ret = shiftOperation(X, Y, c10);   //ALU control input {0,1,2,3}  -> {0,1,2,3}  >> 2 == 0 -> 0 & 3 == 0
	}
	else if (c32 == 1) {  //ALU control input 4 -> (4 >>2) == 1 => 1 & 3 == 1(001)
		// set less
		ret = checkSetLess(X, Y);
	}
	else if (c32 == 2) {  //ALU control input 8 -> 8>>2 == 2 -> 2 & 3 == 2(010)
		// addsubtract
		ret = addSubtract(X, Y, c10);  // addSubtract함수에서 0은 add 1은 subtract
		*Z = checkZero(ret);  // 0 or 1
	}
	else {
		// logic      //ALU control input  15 -> (15>>2) & 3 == 3
		ret = logicOperation(X, Y, c10);
	}
	return ret;
    //ret output
}
int logicOperation(int X, int Y, int C) {
	if (C < 0 || C > 3) {
		printf("error in logic operation\n");
		exit(1);
	}
	if (C == 0) {  //ALU control 0000
		// AND
		return X & Y;
	}
	else if (C == 1) { //ALU control 0001
		// OR
		return X | Y;
	}
	else if (C == 2) { //ALU control 0010
		// XOR
		return X ^ Y;
	}
	else {    //ALU control 1100
		// NOR
		return ~(X | Y);
	}
}

int addSubtract(int X, int Y, int C) {
	int ret;
	if (C < 0 || C > 1) {
		printf("error in add/subtract operation\n");
		exit(1);
	}
	if (C == 0) {
		// add
		ret = X + Y;
	}
	else {
		// subtract
		ret = X - Y;
	}
	return ret;
}

// V is 5 bit shift amount
int shiftOperation(int V, int Y, int C) {
	int ret;
	if (C < 0 || C > 3) {
		printf("error in shift operation\n");
		exit(1);
	}
	if (C == 0) {
		// No shift : 그대로 반환
		ret = V;
	}
	else if (C == 1) {
		// Logical left
		ret = V << Y;
	}
	else if (C == 2) {
		// Logical right
		ret = V >> Y;
	}
	else {
		// Arith right
		ret = V >> Y;
	}
	return ret;
}

// 이함수는 add 또는 subtract 수행 시만
// 사용하여 Z값을 설정한다.
int checkZero(int S) {
	int ret = 0;
	// check if S is zero,
	// and return 1 if it is zero
	// else return 0
	if (S == 0) {
		ret = 1;
	}
	return ret;
}

int checkSetLess(int X, int Y) {
	int ret;

	// check if X < Y,
	// and return 1 if it is true
	// else return 0
	if (Y > X) {
		ret = 1;
	}
	else {
		ret = 0;
	}
	return ret;
}

// 정은찬
// rtype  0 2 3 8 12 37
// itype  1 4 5  8 10 32
// 정재윤
// rtype  16 18 32 34 36 38
// itype  12 13 14 15 24  34
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
			return "add";  ////
		case 33:
			return "addu";
		case 34:
			return "sub";  /////
		case 35:
			return "subu";
		case 36:
			return "and";   //////
		case 37:
			return "or";    /////
		case 38:
			return "xor";   /////
		case 39:
			return "nor"; //////
		case 42:
			return "slt";   //////
		case 43:
			return "sltu";
		default:
			return "ERROR";
	}
}

unsigned char* iTypeName(int opc, int* isImmediate) {
	switch (opc) {
		case 1:
			return "bltz"; //// 15
        case 2:  // j
            return "j";
        case 3: // jal;
            return "jal";
		case 4:
        	*isImmediate = 1;
			return "beq";   /////
		case 5:
        	*isImmediate = 1;
			return "bne";   /////
		case 6:
			return "blez";
		case 7:
			return "bgtz";
		case 8:
        	*isImmediate = 1;
			return "addi";  /////
		case 9:
        	*isImmediate = 1;
			return "addiu";
		case 10:
        	*isImmediate = 1;
			return "slti";   /////
		case 11:
			return "sltiu";
		case 12:
        	*isImmediate = 1;
			return "andi";    /////
		case 13:
        	*isImmediate = 1;
			return "ori";    /////
		case 14:
        	*isImmediate = 1;
			return "xori";   /////
		case 15:
        	*isImmediate = 1;
			return "lui"; ////
		case 24:
			return "mul";     ///////
		case 32:
			return "lb";   /////
		case 33:
			return "lh";
		case 34:
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