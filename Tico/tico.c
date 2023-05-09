#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEMORY_SIZE 256

FILE * fp_niv ;
int count = 0;

char * read_a_line ()
{
	static char buf[BUFSIZ] ;
	static int buf_n = 0 ;
	static int curr = 0 ;

	if (feof(fp_niv) && curr == buf_n - 1)
		return 0x0 ;

	char * s = 0x0 ;
	size_t s_len = 0 ;
	do {
		int end = curr ;
		while (!(end >= buf_n || !iscntrl(buf[end]))) {
			end++ ;
		}
		if (curr < end && s != 0x0) {
			curr = end ;
			break ;
		}
		curr = end ;
		while (!(end >= buf_n || iscntrl(buf[end]))) {
			end++ ;
		}
		if (curr < end) {
			if (s == 0x0) {
				s = strndup(buf + curr, end - curr) ;
				s_len = end - curr ;
			}
			else {
				s = realloc(s, s_len + end - curr + 1) ;
				s = strncat(s, buf + curr, end - curr) ;
				s_len = s_len + end - curr ;
			}
		}
		if (end < buf_n) {
			curr = end + 1 ;
			break ;
		}

		buf_n = fread(buf, 1, sizeof(buf), fp_niv) ;
		curr = 0 ;
	} while (buf_n > 0) ;
	return s ;
}
typedef enum { READ, WRITE, ASSIGN, MOVE, LOAD, STORE, 
			   ADD, MINUS, MULT, MOD, EQ, LESS, JUMP, 
			   JUMPIF, TERM 
			 } Type;

typedef struct { 
  Type operator;
  unsigned char operand1;  
  unsigned char operand2;  
  unsigned char operand3;  
} Instruction;

typedef union { 
  Instruction inst;
  unsigned char num; 
} memory;

// store function
void store(char* line, memory* m) {
	// // debug
	// count++;
	// printf("현재 %d번째!!\n\n", count);



	Instruction instruct;
	// 초기화
    for (int i = 0; i < MEMORY_SIZE; i++) {
        instruct.operator = 0;
        instruct.operand1 = 0;
        instruct.operand2 = 0;
        instruct.operand3 = 0;
    }



	// debug
	// printf("part1\n\n");



    // address 구하기
    char *word = strtok(line, " ");
	int w_len = strlen(word);
	char col = ':';
    char* pcol = strchr(word, col);
	int address_len = w_len - strlen(pcol);
	char cp_address[address_len];

	for(int i=0; i<address_len; i++)
		cp_address[i] = word[i];

	cp_address[address_len] = '\0';
	int address = atoi(cp_address);



	// debug
	// printf("part2\n\n");



	// Instruction 구하기
	char quato = '\"';
	char* pquato;
	word = strtok(NULL, " ");
	if(word != NULL)
		pquato = strchr(word, quato);




	// debug
	// printf("#[%s]\n", pquato);




	if(pquato != NULL) {						// 숫자 data만 존재할 경우
		// printf("=========== quato world ===========\n\n");
		int quato_len = strlen(pquato) - 2;
		char ch_temp[quato_len];
		for(int i=0; i<quato_len; i++)
			ch_temp[i] = pquato[i+1];
		ch_temp[quato_len] = '\0';
		int data = atoi(ch_temp);

		// 예외 처리 -> 공백일 경우 파일 끝까지 가져오는 문제 발생
		char* enterIssue;
		enterIssue = strchr(pquato, col);
		if(enterIssue != NULL)
			m[address].num = 0;
		else // memory에 데이터 입력
			m[address].num = data;
	} else {									// type data로 존재할 경우
		// printf("=========== type world ===========\n\n");
		if (strcmp(word, "READ") == 0) {
			instruct.operator = READ;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "WRITE") == 0) {
			instruct.operator = WRITE;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "ASSIGN") == 0) {
			instruct.operator = ASSIGN;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			char* str = strtok(NULL, " \n");
			instruct.operand1 = (int)(str[0]);
		} else if (strcmp(word, "MOVE") == 0) {
			instruct.operator = MOVE;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "LOAD") == 0) {
			instruct.operator = LOAD;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "STORE") == 0) {
			instruct.operator = STORE;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "ADD") == 0) {
			instruct.operator = ADD;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
			instruct.operand3 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "MINUS") == 0) {
			instruct.operator = MINUS;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
			instruct.operand3 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "MULT") == 0) {
			instruct.operator = MULT;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
			instruct.operand3 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "MOD") == 0) {
			instruct.operator = MOD;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
			instruct.operand3 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "EQ") == 0) {
			instruct.operator = EQ;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
			instruct.operand3 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "LESS") == 0) {
			instruct.operator = LESS;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
			instruct.operand3 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "JUMP") == 0) {
			instruct.operator = JUMP;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
		} else if (strcmp(word, "JUMPIF") == 0) {
			instruct.operator = JUMPIF;
			instruct.operand1 = atoi(strtok(NULL, " \n"));
			instruct.operand2 = atoi(strtok(NULL, " \n"));
		} else
			instruct.operator = TERM;
		// memory에 구조체 데이터 입력
		m[address].inst = instruct;
	}
}

void showResult(memory m[]) {
	for(int i=0; i<MEMORY_SIZE; i++) {
		printf("%d: num -> %d\n",i, m[i].num);
		printf("%d: operator -> %u || operand1 -> %u || operand2 -> %d || operand3 -> %d\n\n",i, m[i].inst.operator, m[i].inst.operand1, m[i].inst.operand2, m[i].inst.operand3);
	}
	printf("\n\n\n\n\n");
	int t_check = 0, j_check = 0, j_address = 0, index = 0, type_num = TERM, exit = 0, i = 0;

    while(1) {
		// j_check를 통해 jump 요소인지 판별, // t_check를 통해 type에 따르 명령어 수행인지 아닌지 판별
		if(j_check == 1) {
			printf("JUMP checker!!!! \n");
			i = j_address;
			j_check = 0;
			j_address = 0;
		}
		t_check = m[i].inst.operator;

		printf("%d번째 주소[시작] operator -> %d  ||  여기서 값 -> %d\n", i, t_check, m[m[i].inst.operand1].num);

		if(t_check <= type_num) {
			switch (t_check) {
				case READ:
					printf("Enter a number: ");
					scanf("%d", &index);
					printf("\n");
					m[m[i].inst.operand1].num = index;
					break;
				case WRITE:
					printf("======RESULT======\n");

					printf("Value -> %d\n\n", m[m[i].inst.operand1].num);
					break;
				case ASSIGN:
					m[m[i].inst.operand1].num = m[i].inst.operand2;
					break;
				case MOVE:
					m[m[i].inst.operand1].num = m[m[i].inst.operand2].num;
					break;
				case LOAD:
					m[m[i].inst.operand1].num = m[m[i].inst.operand2].num;
					break;
				case STORE:
					m[m[i].inst.operand2].num = m[m[i].inst.operand1].num;
					break;
				case ADD:
					m[m[i].inst.operand1].num =  m[m[i].inst.operand2].num +  m[m[i].inst.operand3].num;
					break;
				case MINUS:
					m[m[i].inst.operand1].num =  m[m[i].inst.operand2].num -  m[m[i].inst.operand3].num;
					break;
				case MULT:
					m[m[i].inst.operand1].num = m[m[i].inst.operand2].num * m[m[i].inst.operand3].num;
					break;
				case MOD:
					m[m[i].inst.operand1].num = m[m[i].inst.operand2].num % m[m[i].inst.operand3].num;
					break;
				case EQ:
					if(m[m[i].inst.operand2].num == m[m[i].inst.operand3].num)
						m[m[i].inst.operand1].num = 1;
					else
						m[m[i].inst.operand1].num = 0;
					break;
				case LESS:
					if(m[m[i].inst.operand2].num < m[m[i].inst.operand3].num)
						m[m[i].inst.operand1].num = 1;
					else
						m[m[i].inst.operand1].num = 0;
					break;
				case JUMP:
					j_check = 1;
					j_address = m[i].inst.operand1;
					break;
				case JUMPIF:
					if (m[m[i].inst.operand2].num != 0) {
						j_check = 1;
						j_address = m[i].inst.operand1;
						printf("주소 체크!!! -> %d\n", j_address);
					}
					break;
				case TERM:
					printf("Program terminated :)\n");
					exit = 1;
					break;
			}
		}
		i++;
		if(exit == 1)
			break;
	}
}

int main (int argc, char ** argv)
{
	memory mem[MEMORY_SIZE];
	for(int i=0; i<MEMORY_SIZE; i++) {
		mem[i].num = 0;
		mem[i].inst.operator = 20;
		mem[i].inst.operand1 = 0;
		mem[i].inst.operand2 = 0;
		mem[i].inst.operand3 = 0;
	}

	fp_niv = fopen(argv[1], "r") ;

	char * s = 0x0 ;
	if(fp_niv == NULL)
		  printf("\"%s\" file does not exist.\n", argv[1]);
	else {
		while ((s = read_a_line())) {
		store(s, mem);
		free(s) ;
		}
		showResult(mem);
	}
	
	fclose(fp_niv) ;
}