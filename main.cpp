#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
// #include <cstdio>
/*제어용 전역변수*/
unsigned int PC, IR;
FILE* pFile = nullptr;
errno_t err;
int stop = 0; //명령어 해석 종료 여부 식별 : syscall 10 작동을 위한 변수


unsigned int invertEndian(unsigned int data)
{
    unsigned char c[4];

    c[3] = (unsigned char)data; data = data >> 8;
    c[2] = (unsigned char)data; data = data >> 8;
    c[1] = (unsigned char)data; data = data >> 8;
    c[0] = (unsigned char)data;

    return *(unsigned int*)c;
}
unsigned int To_BigEndian(unsigned int x)
{
	unsigned int result = (x & 0xFF) << 24;

	result |= ((x >> 8) & 0xFF) << 16;
	result |= ((x >> 16) & 0xFF) << 8;
	result |= ((x >> 24) & 0xFF);

	return result;
}
const int M_SIZE = 1024;
unsigned char MEM[M_SIZE];

unsigned int memoryRead(unsigned int addr) {
    unsigned int data;

    if ((addr & 3) != 0) { // check unaligned access?
        printf("memoryRead(): Unaligned access error!\n");
        addr &= 0xFFFFFFFC; // make aligned address
    }
    /*
    data = (unsigned int)MEM[addr];
    data |= (unsigned int)MEM[addr + 1] << 8;
    data |= (unsigned int)MEM[addr + 2] << 16;
    data |= (unsigned int)MEM[addr + 3] << 24;
    */
    data = *(unsigned int*)&MEM[addr];
    data = invertEndian(data);

    return data;
}

void memoryWrite(unsigned int addr, unsigned int data) {
    if ((addr & 3) != 0) { // check unaligned access?
        printf("memoryWrite(): Unaligned access error!\n");
        addr &= 0xFFFFFFFC; // ignore the least 2 bits
    }
    /*
    MEM[addr + 3] = (unsigned char)data; data >>= 8;
    MEM[addr + 2] = (unsigned char)data; data >>= 8;
    MEM[addr + 1] = (unsigned char)data; data >>= 8;
    MEM[addr] = (unsigned char)data;
    */
    *(unsigned int*)&MEM[addr] = invertEndian(data);

    return;
}

// R-type 명령어 디코딩
void decodeRtype(unsigned int fct)
{
    unsigned int fcth, fctl;

    fctl = fct & 0x7;
    fcth = (fct & 0x38) >> 3;

    if (fcth == 0) {
        if (fctl == 0) printf("sll\n");
        else if (fctl == 2) printf("srl\n");
        else if (fctl == 4) printf("sra\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 1) {
        if (fctl == 0) printf("jr\n");
        else if (fctl == 4) { printf("syscall\n"); stop = 1;  return; }
        else printf("Undefined instruction\n");
    }
    else if (fcth == 2) {
        if (fctl == 0) printf("mfhi\n");
        else if (fctl == 2) printf("mflo\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 3) {
        if (fctl == 0) printf("mul\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 4) {
        if (fctl == 0) printf("add\n");
        else if (fctl == 2) printf("sub\n");
        else if (fctl == 4) printf("and\n");
        else if (fctl == 5) printf("or\n");
        else if (fctl == 6) printf("xor\n");
        else if (fctl == 7) printf("nor\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 5) {
        if (fctl == 2) printf("slt\n");
        else printf("Undefined instruction\n");
    }
    else printf("Undefined instruction\n");
}

//ID
void instructionDecode(void)
{
    unsigned int opc, fct;

    unsigned int opch, opcl;

    opc = IR >> 26;
    fct = IR & 0x3f;

    printf("Opc: %2x, Fct: %2x, Inst: ", opc, fct);

    opcl = opc & 0x7;
    opch = (opc & 0x38) >> 3;

    if (opch == 0) {
        if (opcl == 0) {
            decodeRtype(fct);
        }
        else if (opcl == 1) printf("bltz\n");
        else if (opcl == 2) printf("j\n");
        else if (opcl == 3) printf("jal\n");
        else if (opcl == 4) printf("beq\n");
        else if (opcl == 5) printf("bne\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 1) {
        if (opcl == 0) printf("addi\n");
        else if (opcl == 2) printf("slti\n");
        else if (opcl == 4) printf("andi\n");
        else if (opcl == 5) printf("ori\n");
        else if (opcl == 6) printf("xori\n");
        else if (opcl == 7) printf("lui\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 4) {
        if (opcl == 0) printf("lb\n");
        else if (opcl == 3) printf("lw\n");
        else if (opcl == 4) printf("lbu\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 5) {
        if (opcl == 0) printf("sb\n");
        else if (opcl == 3) printf("sw\n");
        else printf("Undefined instruction\n");
    }
    else printf("Undefined instruction\n");
}

// i j g s m r x sr sm ------------------------------------------------------------------------------------------------------------
// int main(){
//     char cmdLine[50];
//     int lenCode= 0;
//     // 레지스터 초기화 함수---------------------------------------------------------------------------------------------------- 0x000000000   은찬님
    // data 0x400000   stack  0x0000000   0x80000000
//     // general mode 랑 Debug
//     while(1){
//         lenCode= 0;
//         printf("Type cmdline : ");
//         gets(cmdLine);   // include  gets() 함수의 include

//         // 공백 문자를 기준으로 문자열을 자르고 문자열 포인터로 반환
//         char* ptr = strtok(cmdLine," ");

//         char* cmdCode= ptr;

//         if(ptr !=NULL){
//             lenCode = strlen(cmdCode);

//         }
//         if(lenCode==1){
// // 명령어 한글자
//         switch(*cmdLine){
//             case 'l':

//         }
//         }else if(lenCode ==2){
// // 명령어 두글자
//         }
//             4}

// }
//시뮬레이터
void main() {

    //시뮬레이터에서 사용할 변수
    char command[100];
    char fileName[100];
    char buf[32];
    /*char* command = malloc(sizeof(char) * 100);
    char* file_name = malloc(sizeof(char) * 100);*/


    //시뮬레이터 동작
    while (1) {
        //기존에 입력받은 명령어 초기화
        // 명령어 받을 때 유저가 "ㅣ <주소> " 가 아닌 "l 주소"로 받기
        //Get command line;
        printf("\t\t\t*명령어 입력형식*\n");
        printf("l 실행파일이름   \t\t\t:실행파일이 시뮬레이터 메모리에 올라갑니다.\n");
        printf("j 프로그램 시작 위치 \t\t:입력한 위치에 시뮬레이터 실행을 준비합니다.\n");
        printf("g\t\t\t\t:현재pc위치에서 시뮬레이터가 명령어를 끝까지 처리합니다.\n");
        printf("s\t\t\t\t:명령어 하나를 처리하고 사용자 명령을 밭는 상태로 중지합니다.\n");
        printf("m<start><end>\t\t\t:start~end범위의 메모리 내용을 출력합니다.\n");
        printf("r\t\t\t\t:현재 레지스터의 내용을 출력합니다.\n");
        printf("x\t\t\t\t:시뮬레이터 프로그램을 종료합니다.\n");
        printf("sr<register number><value>\t:특정 레지스터의 값을 설정합니다.\n");
        printf("sm<location><value>\t\t:메모리 특정 주소의 값을 설정합니다.\n");
        printf("---------------------------------------------------------------------------------------------\n");

        scanf("%s", command);
        //aa = strtok(cmdline," ");
        //명령어 l
        if (command[0] == 'l') {
            /*입력받은 문자열 해석 후 파일 이름 추출*/
            int input = 0;
            char s = '<';
            char e = '>';
            int count = 0;
            int err = 0;
            //  ptr = strtok(NULL, " ")-----------------------------------------------------------------------------------------------------------------
            // if(ptr == NULL){
            //    에러 arguments가 충분치 않다.
            //     ex)  l  c:\\이준용\\as_ex01_arith.bin\n"
            //}
            for (int i = 1; i < 100; i++) {
                if (command[i] == NULL || command[i] == '\0') {
                    printf("입력한 명령어의 형식을 확인해주세요.\n");
                    err = 1;
                    break;
                }
                //개행문자 > 종료인 경우
                if (command[i] == e){
                    input = 0;
                    fileName[count] = '\0';
                    break;
                }
                //파일이름 기록
                if (input==1) {
                    fileName[count] = command[i];
                    count++;
                }
                //개행문자 < 시작인 경우
                if (command[i] == s) {
                    input = 1;
                }
            }

            //해석과정에서 에러가 있었는 지 검증
            if (err == 1) {
                //다시 명령어를 받을 때 한줄 띄움
                printf("\n");
                continue;
            }

            //파일 열어서 FD 저장
            err = fopen_s(&pFile, fileName, "rb");
            if (err) {
                printf(" '%s' 파일을 열 수 없습니다.\n", fileName);
                pFile = NULL;
                continue;
            }

            //프로그램&데이터 메모리 번지 설정

            //PC&SP 초기값 설정


            //현재 명령의 실행결과 출력

        }

        //명령어 j
        else if (command[0] == 'j') {
            /*입력받은 문자열 해석 후 주소 값 추출*/
            char address_point[100];

            int input = 0;
            char s = '<';
            char e = '>';
            int count = 0;
            int err = 0;

            for (int i = 1; i < 100; i++) {
                if (command[i] == NULL || command[i] == '\0') {
                    printf("입력한 명령어의 형식을 확인해주세요.\n");
                    err = 1;
                    break;
                }
                //개행문자 > 종료인 경우
                if (command[i] == e) {
                    input = 0;
                    address_point[count] = '\0';
                    break;
                }
                //주소 포인터 위치 기록
                if (input == 1) {
                    address_point[count] = command[i];
                    count++;
                }
                //개행문자 < 시작인 경우
                if (command[i] == s) {
                    input = 1;
                }
            }

            //해석과정에서 에러가 있었는 지 검증
            if (err == 1) {
                //다시 명령어를 받을 때 한줄 띄움
                printf("\n");
                continue;
            }

            //PC를 해당 주소 값으로 변경
            PC = (unsigned int)address_point; //<- 이부분 수정 필요

            //현재 명령의 실행결과 출력
            printf("현재 PC값: %d\n", PC);

        }

        //명령어 g
        else if (command[0] == 'g') {

            //기존에 load된 데이터가 있는 지 확인
            if (&pFile == nullptr || pFile == NULL) {
                printf("명령어를 load한 뒤 사용해주세요.\n\n");
                continue;
            }


            //현재 PC위치 받아오기



            //명령어를 끝까지 처리
            int i;
            unsigned int data;
            unsigned int addr;
            unsigned int buf[32];

            unsigned int iCount;	// # of instructions
            unsigned int dCount;	// # of data

            int num;
            //명령어를 buf에 32개씩 buf에 저장하며 파일 전체의 명령어 읽어오기
            while ((num = fread(&buf, sizeof(char), 32, pFile)) != 0) {


                // read instruction and data numbers
                for (int j = 0; j < 32; j++) {
                    iCount = invertEndian(buf[j]);
                    dCount = invertEndian(buf[j]);
                    //fread(&data, sizeof(data), 1, buf);
                    //iCount = invertEndian(data);
                    //fread(&data, sizeof(data), 1, buf);
                    //dCount = invertEndian(data);


                    printf("Number of Instructions: %d, Number of Data: %d\n", iCount, dCount);

                    // Load to memory
                    addr = 0;
                    for (i = 0; i < (int)iCount; i++) {
                        fread(&data, sizeof(unsigned int), 1, pFile);
                        data = invertEndian(data);
                        memoryWrite(addr, data);
                        addr += 4;
                    }

                    // Decoding
                    unsigned int PC = 0; // program counter
                    for (i = 0; i < (int)iCount; i++) {
                        IR = memoryRead(PC); // instruction fetch
                        PC += 4;
                        instructionDecode(); // instruction decode
                    }

                    //syscall 10인 경우 명령어 실행 종료
                    if (stop == 1) {
                        fclose(pFile);
                        continue;
                    }
                    fclose(pFile);
                }
            }
        }

        //명령어가 2자리인 경우(sr, sm)
        else if (command[1] != NULL) {

            //명령어 sr
            if (command[0] == 's' && command[1] == 'r') {
                /*입력받은 문자열 해석 후 register number와 value 값 추출*/
                char register_number[100];
                char register_value[100];

                int input = 0;
                char s = '<';
                char e = '>';
                int count = 0;
                int check = 0;
                int err = 0;

                for (int i = 2; i < 100; i++) {
                    if (command[i] == NULL || command[i] == '\0') {
                        printf("입력한 명령어의 형식을 확인해주세요.\n");
                        err = 1;
                        break;
                    }

                    //start_address분리
                    if (check == 0) {
                        //개행문자 > 종료인 경우
                        if (command[i] == e) {
                            input = 0;
                            register_number[count] = '\0';
                            check = 1;
                            count = 0;
                        }
                        //주소 포인터 위치 기록
                        if (input == 1) {
                            register_number[count] = command[i];
                            count++;
                        }
                        //개행문자 < 시작인 경우
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                    else {
                        //개행문자 > 종료인 경우
                        if (command[i] == e) {
                            input = 0;
                            register_value[count] = '\0';
                            break;
                        }
                        //주소 포인터 위치 기록
                        if (input == 1) {
                            register_value[count] = command[i];
                            count++;
                        }
                        //개행문자 < 시작인 경우
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                }


                //해석과정에서 에러가 있었는 지 검증
                if (err == 1) {
                    //다시 명령어를 받을 때 한줄 띄움
                    printf("\n");
                    continue;
                }

                //테스트(성공)
                printf("start_address: %s, end_address: %s\n", register_number, register_value);
                //register함수 실행

                //결과 값 출력
            }

            //명령어 sm
            if (command[0] == 's' && command[1] == 'm') {
                /*입력받은 문자열 해석 후 location와 value 값 추출*/
                char location[100];
                char location_value[100];

                int input = 0;
                char s = '<';
                char e = '>';
                int count = 0;
                int check = 0;
                int err = 0;

                for (int i = 2; i < 100; i++) {

                    if (command[i] == NULL || command[i] == '\0') {
                        printf("입력한 명령어의 형식을 확인해주세요.\n");
                        err = 1;
                        break;
                    }

                    //start_address분리
                    if (check == 0) {
                        //개행문자 > 종료인 경우
                        if (command[i] == e) {
                            input = 0;
                            location[count] = '\0';
                            check = 1;
                            count = 0;
                        }
                        //주소 포인터 위치 기록
                        if (input == 1) {
                            location[count] = command[i];
                            count++;
                        }
                        //개행문자 < 시작인 경우
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                    else {
                        //개행문자 > 종료인 경우
                        if (command[i] == e) {
                            input = 0;
                            location_value[count] = '\0';
                            break;
                        }
                        //주소 포인터 위치 기록
                        if (input == 1) {
                            location_value[count] = command[i];
                            count++;
                        }
                        //개행문자 < 시작인 경우
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                }

                //해석과정에서 에러가 있었는 지 검증
                if (err == 1) {
                    //다시 명령어를 받을 때 한줄 띄움
                    printf("\n");
                    continue;
                }


                //테스트(성공)
                printf("start_address: %s, end_address: %s\n", location, location_value);

                //memory함수 실행

                //결과 값 출력
            }
        }


        //명령어 s
        else if (command[0] == 's') {
            //기존에 load된 데이터가 있는 지 확인
            if (&pFile == nullptr || pFile == NULL) {
                printf("명령어를 load한 뒤 사용해주세요.\n\n");
                continue;
            }

            //현재 PC위치 받아오기

            //현재 명령어 1개만 처리
            int i;
            unsigned int data;
            unsigned int addr;
            unsigned int buf[32];

            unsigned int iCount;	// # of instructions
            unsigned int dCount;	// # of data

            int num;

            //명령어를 buf에 32개씩 buf에 1번만 저장
            if ((num = fread(&buf, sizeof(char), 32, pFile)) != 0) {

                // read instruction and data numbers
                for (int j = 0; j < 32; j++) {
                    iCount = invertEndian(buf[j]);
                    dCount = invertEndian(buf[j]);
                    //fread(&data, sizeof(data), 1, buf);
                    //iCount = invertEndian(data);
                    //fread(&data, sizeof(data), 1, buf);
                    //dCount = invertEndian(data);


                    printf("Number of Instructions: %d, Number of Data: %d\n", iCount, dCount);

                    // Load to memory
                    addr = 0;
                    for (i = 0; i < (int)iCount; i++) {
                        fread(&data, sizeof(unsigned int), 1, pFile);
                        data = invertEndian(data);
                        memoryWrite(addr, data);
                        addr += 4;
                    }

                    // Decoding
                    unsigned int PC = 0; // program counter
                    for (i = 0; i < (int)iCount; i++) {
                        IR = memoryRead(PC); // instruction fetch
                        PC += 4;
                        instructionDecode(); // instruction decode
                    }

                    //syscall 10인 경우 명령어 실행 종료
                    if (stop == 1) {
                        fclose(pFile);
                        continue;
                    }
                    fclose(pFile);
                }
            }

            //명령어에 의해 변경된 register, memory값 받아서 출력



        }

        //명령어 m
        else if (command[0] == 'm') {
            //입력받은 명령어 해석 후 start와 end 값 저장

              /*입력받은 문자열 해석 후 주소 값 추출*/
              char start_address[100];
              char end_address[100];

              int input = 0;
              char s = '<';
              char e = '>';
              int count = 0;
              int check = 0;

              for (int i = 1; i < 100; i++) {
                  if (command[i] == NULL) {
                      printf("입력한 명령어의 형식을 확인해주세요.\n");
                      continue;
                  }

                  //start_address분리
                  if (check == 0) {
                      //개행문자 > 종료인 경우
                      if (command[i] == e) {
                          input = 0;
                          start_address[count] = '\0';
                          check = 1;
                          count = 0;
                      }
                      //주소 포인터 위치 기록
                      if (input == 1) {
                          start_address[count] = command[i];
                          count++;
                      }
                      //개행문자 < 시작인 경우
                      if (command[i] == s) {
                          input = 1;
                      }
                  }

                  else {
                      //개행문자 > 종료인 경우
                      if (command[i] == e) {
                          input = 0;
                          end_address[count] = '\0';
                          break;
                      }
                      //주소 포인터 위치 기록
                      if (input == 1) {
                          end_address[count] = command[i];
                          count++;
                      }
                      //개행문자 < 시작인 경우
                      if (command[i] == s) {
                          input = 1;
                      }
                  }

              }

            //테스트(성공)
            printf("start_address: %s, end_address: %s\n", start_address, end_address);

            //unsigned int unsigned_start_address = (unsigned int)start_address;
            //unsigned int unsigned_end_address = (unsigned int)end_address;
            //printf("start_address: %u, end_address: %u\n", unsigned_start_address, unsigned_end_address);

            //memory함수 접근


            //결과 값 출력


        }

        //명령어 r
        else if (command[0] == 'r') {
            //register함수 접근

            //결과 값 출력

        }

        //명령어 x
        else if (command[0] == 'x') {
            /*저장을 따로 해야하나 ?*/

            //시뮬레이터 동작 종료

            printf("시뮬레이터를 종료합니다.\n");
            break;
        }

        //정의되지 않은 명령어
        else {
        printf("올바른 명령어를 입력해주세요.\n");
        }

        //다시 명령어를 받을 때 1줄 띄우기
        printf("\n");
    }

}
