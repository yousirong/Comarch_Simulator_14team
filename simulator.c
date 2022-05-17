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
static unsigned int R[32], PC;
static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];

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
unsigned char* getInstName(int opc, int fct, int* isImmediate);   // debugging 함수
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


    while(1){
        //COMAND 변수 초기화
        lenCode = 0; //명령어의 인자 수
        cmdLen = 0; //명령어의 자리수(1~2자리 식별용)
        cmdErr = 0;
        char *cmdArr[10] = {NULL, };




    /*명령입력받기*/
        printf("Enter a command.\n>>> ");
        gets(cmdLine);

        char* ptr = strtok(cmdLine, " ");

        while (ptr != NULL)            // 자른 문자열이 나오지 않을 때까지 반복
        {
            cmdArr[lenCode] = ptr;      // 문자열을 자른 뒤 메모리 주소를 문자열 포인터 배열에 저장
            lenCode++;

            ptr = strtok(NULL, " ");   // 다음 문자열을 잘라서 포인터를 반환
        }

        if(cmdArr[0] != NULL){
            cmdLen = strlen(cmdArr[0]);
        }

    /*어떤 명령어인지 식별*/
        if(cmdLen == 1){ //명령어가 한글자일 때

            switch (*cmdArr[0])
            {
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
					printf("\tex) l C:\\pub\\as_ex01_arith.bin\n");
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

                //함수삽입

                break;

        /*g 명령어*/
            case 'g':
                if(checkArgument1(lenCode, 'g') == 1) //명령어 유효성검사
                    break;

                //함수삽입

                break;

        /*s 명령어*/
            case 's':
                if(checkArgument1(lenCode, 's') == 1) //명령어 유효성검사
                    break;

                //함수삽입

                break;

        /*m 명령어*/
            case 'm':
                if(checkArgument3(lenCode, 1) == 1) //명령어 유효성검사
                    break;

                //함수삽입

                break;

        /*r 명령어*/
            case 'r':
                if(checkArgument1(lenCode, 'r') == 1) //명령어 유효성검사
                    break;

                //함수삽입

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


        else if(cmdLen == 2){ //명령어가 두글자일 때

        /*sr 명령어*/
            if(!strcmp(cmdArr[0], "sr")){
                if(checkArgument3(lenCode, 2) == 1){ //명령어 유효성검사
                    printf("\n\n");
                    continue;
                }
                else{
                    //함수삽입
                }
            }

        /*sm 명령어*/
            else if(!strcmp(cmdArr[0], "sm")){
                if(checkArgument3(lenCode, 3) == 1){ //명령어 유효성검사
                    printf("\n\n");
                    continue;
                }
                else{
                    //함수삽입
                }
            }
        /*정의되지 않은 명령어 오류처리: 명령어 2개짜리*/
            else{
                printf("Error: Enter a valid command.");
            }

        }
    /*정의되지 않은 명령어 오류처리: 명령어 입력x인 경우 + 정의되지 않은 명령어인 경우*/
        else{
            printf("Error: Enter a valid command.");
        }

        printf("\n\n");
    }
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
		return 1;
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


//Memory Access 부분이다.
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
    if(opc != 0){
        // I-Format 또는 J-Format 인 경우
        switch(opc){
            case 1:
            case 2:
            //j
            updatePC(IR.JI.jumpAddr);  // L로 이동
            break;
            case 3:
            case 4:
            case 5:
            case 8:
            case 10:
            case 12: //andi
				//X는 레지스터 값, Y는 상수
				unsigned int X;
				memcpy(X, &isImmediate[0], 5);
				unsigned int Y;
				memcpy(Y, &isImmediate[5], 5);

				unsigned int RX = R[X];	//레지스터의 X위치에서 저장된 값 가져오기
				unsigned int answer = ALU(RX, Y, 8, 0); //c32 == 2, c10 == 0만족하기 위해 C = 8, 제로플래그 0 으로
				return answer;    /////
            case 13:
            case 14:
            case 15:
            case 32:
            case 35:
            case 36:
            case 40:
            case 43:
            default:
				// not found
				break;
        }
    }else{
        // R-Format 인 경우
        switch(fct){
            case 0:
            case 2:
            case 3:
            case 8:
            case 12:
            case 16: 
            case 18: 
            //case 24: 
            case 32: //"add"; 
				
			case 34: //"sub";
            case 36: //"and";
			case 37: 
            case 38: //"xor"; 
            case 39: 
            case 42: 
            default: 
            //not found
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
