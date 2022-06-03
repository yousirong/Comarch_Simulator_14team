#define _CRT_SECURE_NO_WARNINGS
#define M_SIZE 1000
#define REG_SIZE 32
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/*제어용 전역변수*/
char fileName[100];
const int check = 1;
static FILE *pFile = NULL;
static int continueTask = 1;
static unsigned int R[32], PC; // 모든 레지스터와 PC 연산할때 쓰는 함수
static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];
static unsigned int var = NULL; // 0xAABBCCDD; // MEM 초기화에 활용할 변수

unsigned char *rTypeName(int fct);
unsigned char *J_I_TypeName(int opc, int *isImmediate);
unsigned char *getInstName(int opc, int fct, int *isImmediate);
// char* getOp(int opc);

char *regArr[32] = {
    "$zero",                                                // 0
    "$at",                                                  // 어셈블러가 사용하기 위해 예약
    "$v0", "$v1",                                           //프로시저 결과
    "$a0", "$a1", "$a2", "$a3",                             // 프로시저 매개변수 => 저장됨
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", // 임시값
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", // 피 연산자 => 프로시저 호출할 때 저장됨
    "$t8", "$t9",                                           // 추가 임시 값
    "$k0", "$k1",                                           // 운영체제(커널) 예약
    "$gp",                                                  // 전역포인터 => 저장됨
    "$sp",                                                  // 스택포인터=> 저장됨
    "$s8",                                                  //프레임 포인터=> 저장됨
    "$ra"                                                   // 반환 주소=> 저장됨
};

/*각 format에 따른 구조체 형식이다. 강의자료 참고함.
RI = r-format 구조체
II = i-format 구조체
JI = j-format 구조체*/
union itype
{
    unsigned int I;
    struct rFormat
    {
        unsigned int opcode : 6;
        unsigned int rs : 5;
        unsigned int rt : 5;
        unsigned int rd : 5;
        unsigned int funct : 6;
    } RI;
    struct iFormat
    {
        unsigned int opcode : 6;
        unsigned int rs : 5;
        unsigned int rt : 5;
        unsigned int offset : 16;
    } II;
    struct jFormat
    {
        unsigned int opcode : 6;
        unsigned int jumpAddr : 26;
    } JI;
} IR;
/*시뮬레이터에 사용될 함수 선언*/
void printNotice();
int checkArgument1(int lenCode, char type);
int checkArgument2(int lenCode, char type);
int checkArgument3(int lenCode, int type);

/*로직에 사용할 함수 선언*/
void initializeRegister();                               //레지스터 초기화
void setRegister(unsigned int regNum, unsigned int val); //원하는 레지스터 값을 변경할 수 있는 함수.
void setMemory(char *offset, char *val);                 //원하는 값으로 해당 메모리에 접근하여 값을 변경하는 함수.
void updatePC(unsigned int addr);                        //현재 pc값을 원하는 값으로 변경하는 함수이다.
void loadInitTask();                                     //바이너리 파일을 로드하고 메모리에 ?적재하는 작업을 담당하는 함수.
void showRegister();                                     //인터페이스 'r'실행시 반환되는 함수
void startGoTask();                                      //인터페이스 'g'실행시 반환되는 함수
void startStepTask();                                    //인터페이스 's'실행시 반환되는 함수   → debugging 함수 포함되어있음

void openBinaryFile(char *filePath);       // l 명령어 실행시 filePath를 받아서 바이너리 파일 여는 함수
unsigned int To_BigEndian(unsigned int x); // 빅엔디안 변경 함수 => hex값
unsigned char getOp(int opc);              // opcode 확인 함수
// binary to decimal 한 값을 int값으로 저장함
// unsigned char* getInstName(int opc, int fct, int* isImmediate);   // debugging 함수
void instExecute(int opc, int fct, int *isImmediate);           // instruction 실행함수
int MEM(unsigned int Reg, int Data, int RW_signal, int Signal); // memory access함수
// ALU
int logicOperation(int OP_A, int OP_B, int CIN);
int addSubtract(int OP_A, int OP_B, int CIN);
int shiftOperation(int Data, int OP_B, int CIN);
int checkZero(int Signal);
int checkSetLess(int OP_A, int OP_B);
int ALU(int OP_A, int OP_B, int CIN, int *Z); // R-format 명령어에서 ALU함수 필요

int main()
{
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

    while (1)
    {
        // COMAND 변수 초기화
        lenCode = 0; //명령어의 인자 수
        cmdLen = 1;  //명령어의 자리수(1~2자리 식별용)
        cmdErr = 0;
        
        /*명령입력받기*/
        printf("Enter a command.\n>>> ");
        gets(cmdLine);
        // 공백 문자를 기준으로 문자열을 자르고 포인터 반환
        char *ptr = strtok(cmdLine, " ");

        // 명령어 코드
        char *cmdArr = ptr;
        if (ptr != NULL)
        { // 명령어가 null이 아니면 반환
            // 명령어 코드 글자 수
            cmdLen = strlen(cmdArr);
        }
        if (cmdLen == 1)
        { //명령어가 한글자일 때
            switch (*cmdArr)
            {
                /*l 명령어*/
            case 'l':
                // load program
                // ptr은 Filename 문자열을 가리킨다.
                ptr = strtok(NULL, " ");
                char *filePath = NULL;
                if (ptr == NULL)
                {
                    printf("Error: Not enough arguments.\n");
                    printf("ex) l CIN:\\pub\\as_ex01_arith.bin\n");
                }
                else
                {
                    filePath = ptr;
                    openBinaryFile(filePath);
                }

                break;
                /*j 명령어*/
            case 'j':
                // jump, 입력한 위치에서 simulator 실행을 준비한다.
                // ptr은 프로그램 시작 위치 문자열을 가리킨다.
                ptr = strtok(NULL, " ");
                char *newAddr = NULL;
                if (ptr == NULL)
                {
                    printf("Error: Not enough arguments.\n");
                    printf("ex) j  0x400000\n");
                }
                else
                {
                    newAddr = ptr;
                    updatePC(strtol(newAddr, NULL, 16));
                }
                break;

                /*g 명령어*/
            case 'g':
                if (checkArgument1(lenCode, 'g') == 1) //명령어 유효성검사
                    break;

                /*Go program, 현재PC위치에서 simulator가 명령어를 끝까지 처리한다.
                이때 사용되는 함수는 startGoTask()이다.*/
                if (pFile != NULL)
                {
                    startGoTask();
                }
                else
                {
                    printf("Error: Load Binary File in advance!\n");
                }
                break;

                /*s 명령어*/
            case 's':
            {
                if (checkArgument1(lenCode, 's') == 1) //명령어 유효성검사
                    break;

                /* Step, 명령어에 의하여 변경된 레지스터, 메모리 정보를 출력한다.
                이때 사용되는 함수는 startStepTask()이다.*/
                if (pFile != NULL)
                {
                    startStepTask();
                    showRegister();
                }
                else
                {
                    printf("Error: Load Binary File in advance!\n");
                }
                break;
            }
                /*m 명령어*/
            case 'm':
                // View memory
                // ptr은 start 문자열을 가리킨다., startAddr~endAddr범위의 메모리 내용 출력
                ptr = strtok(NULL, " ");

                if (ptr == NULL)
                {
                    printf("Error: Not enough arguments.\n");
                    printf("ex) m 0x10000000 0x1000FFFF\n");
                    printf("ex) Instruction -> m 0x400000 0x400010\n");
                    printf("ex) DATA -> m 0x10000000 0x10000010\n");
                }
                else
                {
                    char *start = ptr;

                    // ptr은 end 문자열을 가리킨다.
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL)
                    {
                        printf("Error: Not enough arguments.\n");
                        printf("ex) m 0x10000000 0x1000FFFF\n");
                        printf("ex) Instruction -> m 0x400000 0x400010\n");
                        printf("ex) DATA -> m 0x10000000 0x10000010\n");
                    }
                    else
                    {
                        char *end = ptr;
                        unsigned int startAddr = strtol(start, NULL, 16);
                        unsigned int endAddr = strtol(end, NULL, 16);
                        for (unsigned int i = startAddr; i <= endAddr; i = i + 4)
                        {
                            printf("[0x%08x] => 0x%x\n", i, MEM(i, var, 0, 2));
                        }
                    }
                }
                break;
                /*r 명령어*/
            case 'r':
                if (checkArgument1(lenCode, 'r') == 1) //명령어 유효성검사
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
                printf("Error: Enter a valid command.\n");
                break;
            }
        }
        else if (cmdLen == 2)
        {
            if (strcmp(cmdArr, "sr") == 0)
            {
                // 특정 레지스터의 값 설정
                char *regNum = NULL;
                char *regVal = NULL;
                // ptr은 register number 문자열을 가리킨다.
                ptr = strtok(NULL, " ");

                if (ptr == NULL)
                {
                    printf("Error: Not enough arguments.\n");
                    printf("ex) sr 1 20\n");
                    printf("ex) sr 19 20\n");
                }
                else
                {
                    regNum = ptr;
                    // ptr은 value 문자열을 가리킨다.
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL)
                    {
                        printf("Error: Not enough arguments.\n");
                        printf("ex) sr 1 20\n");
                        printf("ex) sr 19 20\n");
                    }
                    else
                    {
                        regVal = ptr;
                        setRegister(atoi(regNum), strtol(regVal, NULL, 16));
                    }
                }
            }
            else if (strcmp(cmdArr, "sm") == 0)
            {
                // 메모리 특정 주소의 값 설정
                // ptr은 start 문자열을 가리킨다.
                ptr = strtok(NULL, " ");

                if (ptr == NULL)
                {
                    printf("Error: Not enough arguments.\n");
                    printf("ex) sm 0x10000000 20\n");
                }
                else
                {
                    printf("OK\n");
                    // char *memAddr = ptr;
                    unsigned int memAddr = strtol(ptr, NULL, 16);
                    // ptr은 end 문자열을 가리킨다.
                    ptr = strtok(NULL, " ");
                    if (ptr == NULL)
                    {
                        printf("Error: Not enough arguments.\n");
                        printf("ex) sm 0x10000000 20\n");
                    }
                    else
                    {
                        printf("OK\n");
                        // char *memVal = ptr;
                        unsigned int memVal = strtol(ptr, NULL, 16);
                        MEM(memAddr, memVal, 1, 2);
                    }
                }
            }
            else
            {
                printf("Error: Invalid command arguments.\n");
            }
        }
        else
        {
            printf("Error: Enter a valid command.\n");
        }

        printf("\n\n");
    }
    return 0;
}

/*정의되지 않은 명령어 오류처리: 명령어 입력x인 경우 + 정의되지 않은 명령어인 경우*/

//인터페이스 's'실행시 반환되는 함수
void startStepTask()
{
    printf("current value : %x\n", MEM(PC, NULL, 0, 2));
    unsigned instBinary = MEM(PC, NULL, 0, 2);
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
        if (IR.RI.opcode == 0 && IR.RI.funct == 12)
        {
            // syscall 명령어 case
            printf("%s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL));
            continueTask = 0;
        }
        else if (IR.RI.opcode == 0 && IR.RI.funct == 8)
        {
            // jr 명령어 case
            printf("%s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rs]);
        }
        else
        {
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

        instExecute(IR.II.opcode, 0, &isImmediate); // void instExecute(int opc, int fct, int* isImmediate) opcode일 때 fct는 0으로 통일

        // offset인지 immediate value 인지에 따른 결과 출력 변화
        printf("%s", getInstName(IR.II.opcode, 0, &isImmediate));
        if (isImmediate == 1)
        {
            printf(" %s %s %d\n\n", regArr[IR.II.rt], regArr[IR.II.rs], IR.II.offset);
        }
        else
        {
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
void startGoTask()
{
    while (continueTask)
    { // continue Task ==1임 syscall만나면 ==0되면서 종료
        /* Instruction Fetch */
        printf("current value : %x\n", MEM(PC, NULL, 0, 2)); // 현재 메모리 값
        unsigned instBinary = MEM(PC, NULL, 0, 2);
        PC = PC + 4; // pc값 4증가
        /* Instruction Decode */
        // 명령어 타입(R, I, J) 체크 및
        // 명령어 type에 따라 분기하여 추출
        switch (getOp((instBinary >> 26) & 0x3F))
        {         // getOp함수에서 opcode 읽어서 어떤타입인지 알아냄
        case 'R': // case가 왜 R인지 getOp함수 찾아보기
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

            instExecute(IR.RI.opcode, IR.RI.funct, NULL); // 해당 명령어 실행
            //               6bits       6bits

            // 명령어 구분에 따른 결과 출력 변화 (For Debugging) // 명령어 이름 출력하기(특수한 명령어들에 대한 내용)
            if (IR.RI.opcode == 0 && IR.RI.funct == 12)
            {
                // syscall 명령어 case
                printf("%s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL));
                continueTask = 0; // syscall 명령어 만나면 0되면서 while문 종료
            }
            else if (IR.RI.opcode == 0 && IR.RI.funct == 8)
            {
                // jr 명령어 case
                printf("%s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rs]);
            } // jr 명령어 만나면 주소값 강제 변경
            else
            {
                printf("%s %s %s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rd], regArr[IR.RI.rs], regArr[IR.RI.rt]);
            } // 아니면 rd rs rt 평소대로 연산

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

            instExecute(IR.II.opcode, NULL, &isImmediate); // 해당 명령어 실행
            //            6bits                 0 or 1
            // offset인지 immediate value 인지에 따른 결과 출력 변화 (For Debugging)
            printf("%s", getInstName(IR.II.opcode, NULL, &isImmediate));
            if (isImmediate == 1)
            { // 1 인경우 addi와 같은 immediate value값이 있는 명령어임
                printf(" %s %s %d\n\n", regArr[IR.II.rt], regArr[IR.II.rs], IR.II.offset);
            }
            else
            { // 0인경우 평범한 연산 itype 명령어
                printf(" %s %d(%s)\n\n", regArr[IR.II.rt], IR.II.offset, regArr[IR.II.rs]);
            }

            break;
        case 'J':
            // J-Format 기준, opcode 추출
            IR.JI.opcode = (instBinary >> 26) & 0x3F;
            // jump target address 추출
            IR.JI.jumpAddr = instBinary & 0x3FFFFFF;

            instExecute(IR.JI.opcode, NULL, NULL);
            //             6bits

            // 결과 출력 (For Debugging)
            printf("%s %d\n\n", getInstName(IR.JI.opcode, NULL, NULL), IR.JI.jumpAddr);
            //  점프명령어 어디 주소로 jump 했는지 출력
            break;
        default:
            break;
        }
    }
}
//인터페이스 'r'실행시 반환되는 함수
void showRegister()
{
    // 16진수로 출력
    // 현재 레지스터 값 모두 출력
    printf("[REGISTER]\n");
    for (int i = 0; i < REG_SIZE; i++)
    {
        printf("$%d=\t0x%x\n", i, R[i]);
    }
    printf("PC=\t0x%x\n", PC);
}
//시뮬레이터 사용법 출력함수
void printNotice()
{
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
unsigned char getOp(int opc)
{
    char format;
    // R-Format instruction
    if (opc == 0)
    {
        format = 'R';
    }
    // J-Format instruction
    else if ((opc == 2) || (opc == 3))
    {
        format = 'J';
    }
    // I-Format instruction
    else
    {
        format = 'I';
    }
    return format;
}
/*올바르지 않은 인자 확인 함수*/
int checkArgument1(int lenCode, char type)
{ //인자가 1개인 명령어들
    int result = 0;

    if (lenCode >= 2)
    {
        // printf("Error: Keep the format of the command.\n");

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

int checkArgument2(int lenCode, char type)
{ //인자가 2개인 명령어들
    int result = 0;

    switch (type)
    {
    case 'l':
        if (lenCode == 2)
        { //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) l Path of Executable File Name\n");
        break;

    case 'j':
        if (lenCode == 2)
        { //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) j Program Start Location\n");
        result = 1;
        break;

    default:
        break;
    }

    return result;
}

int checkArgument3(int lenCode, int type)
{ //인자가 3개인 명령어들
    int result = 0;

    switch (type)
    {
        /*m 명령어*/
    case 1:
        if (lenCode == 3)
        { //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) m startAddress endAddress\n");
        result = 1;
        break;

        /*sr 명령어*/
    case 2:
        if (lenCode == 3)
        { //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) sr register number value\n");
        result = 1;
        break;

        /*sm 명령어*/
    case 3:
        if (lenCode == 3)
        { //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) sm location value\n");
        result = 1;
        break;
    default:
        break;
    }

    return result;
}

//     l filePath
unsigned char *getInstName(int opc, int fct, int *isImmediate)
{ // 디버깅 함수에 쓰임
    // 명령어 출력해주는 함수임
    // 디버깅 함수는 명령어 출력해주고 어떻게 연산되었는지 어떻게 바뀌었는지 int값이 뭔지
    // 주소가 어떻게 되었는지 디버깅함.
    // int val = instruction->inst;
    // int opc = val >> 26;
    // int fct = val & 0x3f;
    if (opc != 0)
    {
        switch (opc)
        {
        case 1:
            return "bltz";
        case 2: // j
            return "j";
        case 3: // jal;
            return "jal";
        case 4:
            *isImmediate = 1;
            return "beq";
        case 5:
            *isImmediate = 1;
            return "bne";
        case 6:
            return "blez";
        case 7:
            return "bgtz";
        case 8:
            *isImmediate = 1;
            return "addi";
        case 9:
            return "addiu";
        case 10:
            *isImmediate = 1;
            return "slti";
        case 11:
            return "sltiu";
        case 12:
            *isImmediate = 1;
            return "andi";
        case 13:
            *isImmediate = 1;
            return "ori";
        case 14:
            *isImmediate = 1;
            return "xori";
        case 15:
            *isImmediate = 1;
            return "lui";
        case 24:
            return "mul";
        case 32:
            return "lb";
        case 33:
            return "lh";
        case 35:
            return "lw";
        case 36:
            return "lbu";
        case 37:
            return "lhu";
        case 40:
            return "sb";
        case 41:
            return "sh";
        case 43:
            return "sw";
        default:
            return "ERROR";
        }
    }
    else
    {
        // R-format인 경우
        switch (fct)
        {
        case 0:
            return "sll";
        case 2:
            return "srl";
        case 3:
            return "sra";
        case 4:
            return "sllv";
        case 6:
            return "srlv";
        case 7:
            return "srav";
        case 8:
            return "jr";
        case 9:
            return "jalr";
        case 12:
            return "syscall";
        case 16:
            return "mfhi";
        case 17:
            return "mthi";
        case 18:
            return "mflo";
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
            return "add";
        case 33:
            return "addu";
        case 34:
            return "sub";
        case 35:
            return "subu";
        case 36:
            return "and";
        case 37:
            return "or";
        case 38:
            return "xor";
        case 39:
            return "nor";
        case 42:
            return "slt";
        case 43:
            return "sltu";
        default:
            return "ERROR";
        }
    }
}
// 바이너리 파일 여는 함수   -> l명령어
// 레지스터 초기화
void initializeRegister()
{
    for (int i = 0; i < REG_SIZE; i++)
    {
        // 32bit
        R[i] = 0x00000000;
    }
    // PC 초기값 설정
    PC = 0x00400000;
    // SP 초기값 설정
    R[29] = 0x80000000;
}
// 바이너리 파일 여는 함수
void openBinaryFile(char *filePath)
{
    // File Validation TEST

    // FILE* testFile = NULL;
    //--------------------------------------------------------이부분 고치기 file 못읽음
    FILE *testFile = fopen(filePath, "rb"); // 파일 경로 받아서 testFile구조체로 열기
    if (testFile == NULL)
    { // NULL이면 에러 반환
        printf("Cannot open file\n");
        return 1;
    }
    unsigned int data;
    unsigned int data1 = 0xAABBCCDD;
    if (fread(&data, sizeof(data1), 1, testFile) != 1) // 한줄 읽었는데 내용 없으면
        exit(1);                                       // 반환
    fclose(testFile);                                  //반환 하고 종료 닫음

    // Load Real File
    fopen_s(&pFile, filePath, "rb");                           // 있다면 위에서 진짜 읽음
    printf("The Binary File Has Been Loaded Successfully.\n"); // 읽기 성공

    // Load Init Task (메모리 적재)
    loadInitTask();
}
/*To_BigEndian = 데이터가 있을때 큰 단위가 앞으로 나오게 만드는 함수.
이진수에서는 상위비트로 갈 수록 값이 커지기 때문에 앞쪽으로 갈 수록 단위가 커진다.*/
unsigned int To_BigEndian(unsigned int REG)
{ // binary 파일 읽어서 mips에서는 빅엔디안 시켜서 메모리 출력함
    unsigned int result = (REG & 0xFF) << 24;

    result |= ((REG >> 8) & 0xFF) << 16;
    result |= ((REG >> 16) & 0xFF) << 8;
    result |= ((REG >> 24) & 0xFF);

    return result;
}

/*Instruction Fetch단계 =>loadInintTask() = 바이너리 파일을 load하고 메모리에 적재하는 작업을 담당하는 함수*/
void loadInitTask()
{
    updatePC(0x400000);          // PC주소값 초기화
    setRegister(29, 0x80000000); // $29번 레지스터 초기화

    // printf("\n%s\n", loadedFilePath);
    unsigned int data;
    unsigned int data1 = 0xAABBCCDD;
    unsigned int numInst; // number Instruction
    unsigned int numData; // number Data

    // Read the number of Instructions
    fread(&numInst, sizeof(data1), 1, pFile); // 한줄씩 읽어서 big_endian
    numInst = To_BigEndian(numInst);
    // Read the number of Datas
    fread(&numData, sizeof(data1), 1, pFile); // 한줄씩 읽어서 big_endian
    numData = To_BigEndian(numData);

    printf("size of Instructions : %d\n", numInst); //  명령어 몇줄인지 출력
    printf("size of Datas : %d\n", numData);        // data 양 출력
                                                    // 메모리 주소 구조 초기화
    unsigned int memAddr = 0x00400000;              // memory address 초기화
    unsigned int dataAddr = 0x10000000;             // data address 초기화

    for (int i = 0; i < numInst; i++)
    {
        if (fread(&data, sizeof(data1), 1, pFile) != 1) // 한줄씩 읽어서
            exit(1);
        // 명령어 메모리 적재
        data = To_BigEndian(data);            // big_endian시켜서 data에 저장
        printf("Instruction = %08x\n", data); // 명령어 출력

        MEM(memAddr, data, 1, 2); // 해당 메모리 MEM주소에 저장
        memAddr = memAddr + 4;    // 메모리 4씩 증가 PC 주소값 4씩 증가랑 같은 원리
    }

    for (int i = 0; i < numData; i++)
    { // MEM데이터도 위와 같은 원리
        if (fread(&data, sizeof(data1), 1, pFile) != 1)
            exit(1);
        data = To_BigEndian(data); // data big_endian시켜서 data에 저장
        // 데이터 메모리 적재
        printf("Data = %08x\n", data); // 해당 data 뭔지 출력

        MEM(dataAddr, data, 1, 2); // MEM구조에 data 저장
        dataAddr = dataAddr + 4;   // MEM에 data 저장 어떻게 되는지는 메모리 구조 찾아보기
    }
}

//현재 pc값을 원하는 값으로 변경하는 함수이다.
void updatePC(unsigned int addr)
{
    PC = addr; // 점프 명령어에 쓰임
}
//원하는 레지스터 값을 변경할 수 있는 함수.
void setRegister(unsigned int regNum, unsigned int val)
{
    R[regNum] = val; // 레지스터값 강제 변경 하는 명령어에 쓰임
}

//원하는 값으로 해당 메모리에 접근하여 값을 변경하는 함수.
void setMemory(char *offset, char *val)
{
    R[atoi(offset)] = strtol(val, NULL, 16); // 메모리 강제 접근해서 변경하는하는 함수
    // operand / offset 16bits => Immediate operand or address offset
}

// mips memory allocation
// Memory Access 함수 int MEM(unsigned int Reg, int Data, int RW_signal, int Signal);
// RW_signal read할지 write할지 제어 control  S는 byte인지 halfword인지 word인지
int MEM(unsigned int Reg, int Data, int RW_signal, int Signal)
{
    unsigned int sel, offset;
    unsigned char *pM;      // pointer MEM
    sel = Reg >> 20;        // 32-12 21bit 6op 5rs 5rt 5rd
    offset = Reg & 0xFFFFF; // 16bit짜리 상수지정

    if (sel == 0x004)
        pM = progMEM; // Program memory
    else if (sel == 0x100)
        pM = dataMEM; // Data memory
    else if (sel == 0x7FF)
        pM = stakMEM; // Stack
    else
    {
        printf("No memory in executable file\n");
        // 에러 케이스 테스트를 위해 전체 프로그램을 종료하지 않고
        // 함수만 종료한다
        return 1;
    }

    if (Signal == 0)
    {
        // Byte = 8bits
        if (RW_signal == 0)
        {
            // Read
            return pM[offset];
        }
        else if (RW_signal == 1)
        {
            // Write
            pM[offset] = Data;
            return 1;
        }
        else
        {
            printf("RW_signal input error\n");
            return 1;
            // exit(1);
        }
    }
    else if (Signal == 1)
    {
        // Half word  = 2bytes
        if (offset % 2 != 0) // Half-word-aligned Check
        {
            printf("Not an half-word-aligned address input!\n");
            return 1;
            // exit(1);
        }
        if (RW_signal == 0)
        {
            // Read
            int result = (pM[offset] << 8) + (pM[offset + 1]);
            return result;
        }
        else if (RW_signal == 1)
        {
            // Write
            pM[offset] = (Data >> 8) & 0xFF;
            pM[offset + 1] = Data & 0xFF;
        }
        else
        {
            printf("RW_signal input error\n");
            return 1;
            // exit(1)
        }
    }
    else if (Signal == 2)
    {
        // Word  = 4bytes
        if (offset % 4 != 0) // Word-aligned Check
        {
            printf("Not an word-aligned address input!\n");
            return 1;
            // exit(1)
        }
        if (RW_signal == 0)
        {
            // Read
            int result = (pM[offset] << 24) + (pM[offset + 1] << 16) + (pM[offset + 2] << 8) + (pM[offset + 3]);
            return result;
        }
        else if (RW_signal == 1)
        {
            // Write
            pM[offset] = (Data >> 24) & 0xFF;
            pM[offset + 1] = (Data >> 16) & 0xFF;
            pM[offset + 2] = (Data >> 8) & 0xFF;
            pM[offset + 3] = Data & 0xFF;
        }
        else
        {
            printf("RW_signal input error\n");
            return 1;
            // exit(1)
        }
    }
    else
    { // S가 유효하지 않은 값일 경우 오류처리
        printf("Size input error\n");
        return 1;
        // exit(1)
    }
}
/*R-format + (I-format 또는 J-format)라는 2가지 format으로 나누었다.
switch문을 사용해 case마다 명령어 처리했다.
각 instruction은 MIPS simulator 강의자료 참고함*/
void instExecute(int opc, int fct, int *isImmediate)
{
    // zero flag 선언
    int Z;
    // Z = 0;

    int sub;

    if (opc != 0)
    {
        // I-Format 또는 J-Format 인 경우
        switch (opc)
        {
        case 1: // bltz
            // int Z;
            if (ALU(R[IR.II.rs], 0, 4, &Z) == 1)
            {
                updatePC(PC + IR.II.offset * 4); // PC = PC + 4 + 4 * offset
            }
            break;
        case 2:
            // j
            updatePC((PC & 0xF8000000) + (IR.JI.jumpAddr << 2)); // Loop로 이동
            break;
        case 3:
            // jal
            setRegister(31, PC + 4);                             // $ra = PC + 4
            updatePC((PC & 0xF8000000) + (IR.JI.jumpAddr << 2)); // Loop로 이동
            break;

        case 4:
            // beq
            // int Z;
            if (ALU(R[IR.II.rs], R[IR.II.rt], 8, &Z) == 0)
            {
                updatePC(PC + IR.II.offset * 4); // PC = PC + 4 + 4 * offset
            }
            *isImmediate = 1;
            break;

        case 5:
            // bne
            // int Z;
            if (ALU(R[IR.II.rs], R[IR.II.rt], 8, &Z) != 0)
            {
                updatePC(PC + IR.II.offset * 4); // PC = PC + 4 + 4 * offset
            }
            *isImmediate = 1;
            break;

        case 8:
            // addi
            // int Z;
            R[IR.II.rt] = ALU(R[IR.II.rs], IR.II.offset, 8, &Z);
            *isImmediate = 1;
            break;
      
        case 10:
            // slti
            R[IR.II.rt] = ALU(R[IR.II.rs], IR.II.offset, 4, &Z);
            *isImmediate = 1;
            break;

        case 12:
            // andi
            R[IR.II.rt] = ALU(R[IR.II.rs], IR.II.offset, 12, &Z);
            *isImmediate = 1;
            break;
        case 13:
            // ori
            R[IR.II.rt] = ALU(R[IR.II.rs], IR.II.offset, 13, &Z);
            *isImmediate = 1;
            break;
        case 14:
            // xori
            R[IR.II.rt] = ALU(R[IR.II.rs], IR.II.offset, 14, &Z);
            *isImmediate = 1;
            break;
        case 15:
            // lui
            R[IR.II.rt] = IR.II.offset << 16;
            *isImmediate = 1;
            break;
        case 32:
            // lb
            R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, NULL, 0, 0);
            break;
        case 35:
            // lw
            R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, NULL, 0, 2);
            break;
        case 36:
            // lbu
            R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, NULL, 0, 0);
            break;
        case 40:
            // sb
            MEM(R[IR.II.rs] + IR.II.offset, R[IR.II.rt], 1, 0);
            break;
        case 43:
            // sw
            MEM(R[IR.II.rs] + IR.II.offset, R[IR.II.rt], 1, 2);
            break;
        default:
            // not found
            break;
        }
    }
    else
    {
        // R-Format 인 경우 다음과 같이계산
        //레지스터[rd] =ALU(1번째 인자rs , 2번째 인자rt, ALU연산제어신호, zero 플래그 )
        switch (fct)
        {
        case 0:
        {
            // sll
            // int Z;
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 1, &Z);
            break;
        }
        case 2:
        {
            // srl
            // int Z;

            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 2, &Z);
            break;
        }
        case 3:
        {
            // sra
            // int Z;

            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 3, &Z);
            break;
        }
        case 8:
            // jr
            updatePC(R[31]); // go to $ra
            break;
        case 12:
            // syscall
            continueTask = 0; // 12 syscall명령어 만나면 종료
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
        case 32:
        {
            // add
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 8, &Z);
            break;
        }
        case 34:
        {
            // sub
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 9, &Z);
            break;
        }
        case 36:
        {
            // and
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 12, &Z);
            break;
        }
        case 37:
        {
            // or
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 13, &Z);
            break;
        }
        case 38:
        {
            // xor
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 14, &Z);
            break;
        }
        case 39:
        {
            // nor
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 15, &Z);
            break;
        }
        case 42:
        {
            // slt
            R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 4, &Z);
            break;
        }
        default:
            // NOT FOUND!
            break;
        }
    }
}

// ex ) add $t1, $t2, $t3
int ALU(int OP_A, int OP_B, int CARRY, int *Z)
{
    // OP_A = 4-bit input number
    // OP_B = 4-bit input number
    // CIN = carry into LSB position  ALU에서 어떤 명령어를 사용할지 제어신호

    // Z = Zero Flag 0 or 1
    // Zero Flag:  This bit is updated as a result of all operations.
    // If the result of an operation is zero, then Z is asserted.
    // If the result of an operation is not zero, then Z is 0.
    int ALU_CON_input, CARRY_INT;
    int res;

    ALU_CON_input = (CARRY >> 2) & 3;
    CARRY_INT = CARRY & 3;
    if (ALU_CON_input == 0)
    {
        // shift
        res = shiftOperation(OP_A, OP_B, CARRY_INT); // ALU control input {0,1,2,3}  -> {0,1,2,3}  >> 2 == 0 -> 0 & 3 == 0
    }
    else if (ALU_CON_input == 1)
    { // ALU control input 4 -> (4 >>2) == 1 => 1 & 3 == 1(001)
        // set less
        res = checkSetLess(OP_A, OP_B);
    }
    else if (ALU_CON_input == 2)
    { // ALU control input 8 -> 8>>2 == 2 -> 2 & 3 == 2(010)
        // addsubtract
        res = addSubtract(OP_A, OP_B, CARRY_INT); // addSubtract함수에서 0은 add 1은 subtract
        *Z = checkZero(res);                      // 0 or 1
    }
    else
    {
        // logic      //ALU control input  15 -> (15>>2) & 3 == 3
        res = logicOperation(OP_A, OP_B, CARRY_INT);
    }
    return res;
    // res output  -> result output
}
int logicOperation(int OP_A, int OP_B, int CIN)
{
    if (CIN < 0 || CIN > 3)
    {
        printf("error in logic operation\n");
        exit(1);
    }
    if (CIN == 0)
    { // ALU control 0000
        // AND
        return OP_A & OP_B;
    }
    else if (CIN == 1)
    { // ALU control 0001
        // OR
        return OP_A | OP_B;
    }
    else if (CIN == 2)
    { // ALU control 0010
        // XOR
        return OP_A ^ OP_B;
    }
    else
    { // ALU control 1100
        // NOR
        return ~(OP_A | OP_B);
    }
}

int addSubtract(int OP_A, int OP_B, int CIN)
{
    int res;
    if (CIN < 0 || CIN > 1)
    {
        printf("error in add/subtract operation\n");
        exit(1);
    }
    if (CIN == 0)
    {
        // add
        res = OP_A + OP_B;
    }
    else
    {
        // subtract
        res = OP_A - OP_B;
    }
    return res;
}

// Data is 5 bit shift amount
int shiftOperation(int Data, int OP_B, int CIN)
{
    int res;
    if (CIN < 0 || CIN > 3)
    {
        printf("error in shift operation\n");
        exit(1);
    }
    if (CIN == 0)
    {
        // No shift : 그대로 반환
        res = Data;
    }
    else if (CIN == 1)
    {
        // Logical left
        res = Data << OP_B;
    }
    else if (CIN == 2)
    {
        // Logical right
        res = Data >> OP_B;
    }
    else
    {
        // Arith right
        res = Data >> OP_B;
    }
    return res;
}

// 이함수는 add 또는 subtract 수행 시만
// 사용하여 Z값을 설정한다.
int checkZero(int Signal)
{
    int res = 0;
    // check if Signal is zero,
    // and return 1 if it is zero
    // else return 0
    if (Signal == 0)
    {
        res = 1;
    }
    return res;
}

int checkSetLess(int OP_A, int OP_B)
{
    int res;

    // check if OP_A < OP_B,
    // and return 1 if it is true
    // else return 0
    if (OP_B > OP_A)
    {
        res = 1;
    }
    else
    {
        res = 0;
    }
    return res;
}