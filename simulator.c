#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <cstdio>

/*제어용 전역변수*/
unsigned int PC, IR;
FILE* pFile;


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

char* regArr[32] = { "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
"$t8","$t9","$k0","$k1","$gp","$sp","$s8","$ra" };

static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];

static unsigned int R[32], PC;

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

    //레지스터 초기화 함수


    while(1){
        //COMAND 변수 초기화
        lenCode = 0; //명령어의 인자 수
        cmdLen = 0; //명령어의 자리수(1~2자리 식별용)
        cmdErr = 0;
        char *cmdArr[10] = {NULL, };
        

        //시뮬레이터 사용법 출력
        printNotice();

    /*명령입력받기*/
        printf("명령어를 입력하세요.\n>>> ");
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
                
                //함수삽입

                //바이너리 파일을 읽지 못한 경우에 대한 에러 처리


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
                printf("프로그램을 종료합니다.\n");
                exit(1);
                break;

        /*정의되지 않은 명령어 오류처리: 명령어 1개짜리*/
            default:
                printf("Error: 올바른 명령어를 입력해주세요.");
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
                printf("Error: 올바른 명령어를 입력해주세요.");
            }

        }
    /*정의되지 않은 명령어 오류처리: 명령어 입력x인 경우 + 정의되지 않은 명령어인 경우*/
        else{
            printf("Error: 올바른 명령어를 입력해주세요.");
        }
        
        printf("\n\n");
    }
}


//시뮬레이터 사용법 출력함수
void printNotice(){
    printf("\t\t\t*명령어 입력형식*\n");
    printf("l 실행파일이름\t\t\t:실행파일이 시뮬레이터 메모리에 올라갑니다.\n");
    printf("j 프로그램 시작 위치 \t\t:입력한 위치에 시뮬레이터 실행을 준비합니다.\n");
    printf("g\t\t\t\t:현재pc위치에서 시뮬레이터가 명령어를 끝까지 처리합니다.\n");
    printf("s\t\t\t\t:명령어 하나를 처리하고 사용자 명령을 밭는 상태로 중지합니다.\n");
    printf("m start end\t\t\t:start~end범위의 메모리 내용을 출력합니다.\n");
    printf("r\t\t\t\t:현재 레지스터의 내용을 출력합니다.\n");
    printf("x\t\t\t\t:시뮬레이터 프로그램을 종료합니다.\n");
    printf("sr register number value\t:특정 레지스터의 값을 설정합니다.\n");
    printf("sm location value\t\t:메모리 특정 주소의 값을 설정합니다.\n");
    printf("---------------------------------------------------------------------------------------------\n");
}

/*올바르지 않은 인자 확인 함수*/
int checkArgument1(int lenCode, char type){ //인자가 1개인 명령어들
    int result = 0;

    if(lenCode >= 2){
        printf("Error: 명령어의 형식을 지켜주세요.\n");

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
        printf("Error: 명령어의 형식을 지켜주세요.\n");
        printf("\tex) l 실행파일의경로와이름");
        break;

    case 'j':
        if(lenCode == 2){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: 명령어의 형식을 지켜주세요.\n");
        printf("\tex) j 프로그램시작위치");
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
        printf("Error: 명령어의 형식을 지켜주세요.\n");
        printf("\tex) m 시작주소 종료주소");
        result = 1;
        break;

/*sr 명령어*/
    case 2:
        if(lenCode == 3){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: 명령어의 형식을 지켜주세요.\n");
        printf("\tex) sr 레지스터번호 지정할값");
        result = 1;
        break;  

/*sm 명령어*/
    case 3:
        if(lenCode == 3){ //오류가 없는 정상적인 상태인 경우
            break;
        }
        printf("Error: 명령어의 형식을 지켜주세요.\n");
        printf("\tex) sm 메모리주소 지정할값");
        result = 1;
        break;                  
    default:
        break;
    }

    return result;
}