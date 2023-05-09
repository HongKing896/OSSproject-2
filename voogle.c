#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VALUE 100
#define MAX 1024

FILE * fp_niv ;

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
// 어떤 형태인지 판별
int search_type(char token[]) {
    int token_len = strlen(token);

    // token*
    if(token[token_len - 1] == '*') {
        return 0;
    }

    // -token
    if(token_len > 1 && token[0] == '-') {
        return 1;
    }

    char col = ':';
    char* pcol = strchr(token, col);
    if(pcol != NULL) {
        if(token[0] == 'b')
            return 2;
        else if(token[0] == 'c')
            return 3;
    }
    
    char quo = '\"';
    char* pquo = strchr(token, quo);
    if(pquo != NULL)
        return 4;
    
    return 5;  // token
}
// token
int basic_token(char* b_verse, char* token) {
    int len = strlen(token);
    // 토큰 길이가 0
    if(len <= 0) 
        return 0;

    // 대소문자 구분 없애기
    char low_token[len+1];
    for(int i=0; i<len; i++)
        low_token[i] = tolower(token[i]);
    low_token[len] = '\0';

    // 주어진 문자열에서 각 단어와 비교
    char *word = strtok(b_verse, " ");
    int word_len;
    while(word != NULL) {
        word_len = strlen(word);
        char low_word[word_len+1];
        for(int i=0; i<word_len; i++)
            low_word[i] = tolower(word[i]);
        low_word[word_len] = '\0';

        if(strcmp(low_token, low_word) == 0) {
            if(word_len == len)
                return 1; // 일치하는 단어가 있으면 참 반환
            else
                return 0;
        }
        word = strtok(NULL, " ");
    }

    return 0; // 접두사와 일치하는 단어가 없으면 거짓 반환
}

// token*
int star_token(char* b_verse, char *token) {
    int len = strlen(token) - 1;
    // 토큰 길이가 0이거나 '*'만 있는 경우
    if(len <= 0) 
        return 0;

    // 대소문자 구분 없이 접두사 추출
    char prefix[len + 1];
    for(int i=0; i<len; i++)
        prefix[i] = tolower(token[i]);
    prefix[len] = '\0';

    // 주어진 문자열에서 각 단어와 비교
    char *word = strtok(b_verse, " ");
    while(word != NULL) {
        if(strncasecmp(word, prefix, len) == 0) {
            return 1; // 접두사와 일치하는 단어가 있으면 참 반환
        }
        word = strtok(NULL, " ");
    }

    return 0; // 접두사와 일치하는 단어가 없으면 거짓 반환
}

// -token
int stick_token(char b_verse[], char* token) {
    // printf("check!!! %s\n", b_verse);
    // printf("처음 token -> %s\n", token);
    int len = strlen(token) - 1;
    if(len <= 0) 
        return 1;

    char token_cpy[len + 1];
    for(int i=0; i<len; i++)
        token_cpy[i] = tolower(token[i+1]);
    token_cpy[len] = '\0';

    int word_len;
    // 주어진 문자열에서 각 단어와 비교
    char *word = strtok(b_verse, " ");
    while(word != NULL) {
        word_len = strlen(word);
        char low_word[word_len+1];
        for(int i=0; i<word_len; i++)
            low_word[i] = tolower(word[i]);
        low_word[word_len] = '\0';
        // printf("word: %s ----------- token_cpy: %s\n", low_word, token_cpy);
        if(strcmp(word, token_cpy) == 0) {
            if(word_len == len)
                return 0;
        }
        word = strtok(NULL, " ");
    }
    return 1; // no token
}

int book_token(char* b_verse, char* token) {
    char col[] = ":";
    char* pcol = strstr(token, col);
    int len = strlen(pcol);
    // 토큰 길이가 0
    if(len <= 0) 
        return 0;

    // ':' 삭제 후 book name 추출
    char book_copy[len];
    for(int i=0; i<len; i++)
        book_copy[i] = pcol[i+1];
    book_copy[len] = '\0';

    // 주어진 문자열에서 각 단어와 비교
    char *word = strtok(b_verse, " ");

    if(strcmp(book_copy, word) == 0)
        return 1; 

    return 0; // 일치하는 단어가 없으면 거짓 반환
}

int ch_token(char* b_verse, char* token) {
    char col = ':';
    char* pcol = strchr(token, col);
    int len = strlen(pcol);
    // 토큰 길이가 0
    if(len <= 0) 
        return 0;

    // ':' 삭제 후 chapter 추출
    char ch_copy[len];
    for(int i=0; i<len; i++)
        ch_copy[i] = pcol[i+1];
    ch_copy[len] = '\0';

    // chapter 형 변환(char -> int)
    int ch_num = atoi(ch_copy);

    // b_verse에서 chapter 추출
    char *word = strtok(b_verse, " ");
    word = strtok(NULL, " ");
    char word_copy[len];
    for(int i=0; i<len; i++)
        word_copy[i] = word[i];

    // word_copy 형 변환(char -> int)
    int word_num = atoi(word_copy);

    if(ch_num == word_num)
        return 1; 

    return 0; // 일치하는 단어가 없으면 거짓 반환
}

int quo_token(char* b_verse, char* token) {
    char * temp = 0x0;
    temp = (char*) calloc(BUFSIZ, sizeof(char));
    temp = strtok(token, "\"");

    if(strstr(b_verse, temp))
        return 1;
    else 
        return 0;
}

int main (int argc, char ** argv)
{
    // type별 내용 담을 변수 선언
    char basic[VALUE] = "";  // 일반
    char star[VALUE] = "";   // *
    char stick[VALUE] = "";  // -
    char book[VALUE] = "";   // book:code
    char ch[VALUE] = "";     // chapter:num   
    char quo[VALUE] = "";     // " "  

    // 합치기 위한 공간

    char inputString[BUFSIZ];
	char* searchTokens[BUFSIZ];
	char* finalTokens[BUFSIZ];

    printf("======== Seach bar ========\n");
    printf("Search Keywords: ");
	fgets(inputString, BUFSIZ, stdin);
	//printf("inputString length: %lu\n", strlen(inputString));
	// if no input is received (only enter or directly ctrl+D, input is invalid -> print nothing & exit)
	if(strlen(inputString) <= 1){
		exit(-1);
	}
	inputString[strlen(inputString) -1] = '\0';
	//printf("inputString length: %lu\n", strlen(inputString));

	//tokenize inputString. 
	int count = 0;
	char* cur;
	while(1){
		if (count == 0) cur = strtok(inputString, " ");
		else cur = strtok(NULL, " ");

		if (cur == NULL) break;
		else searchTokens[count] = cur;

		count++;
	}

	for(int i = 0; i < count; i++){
		printf("(%s) ",searchTokens[i]);
	}
	printf("\n");

	//printf("break\n");

	int quotes = 0; //for indicating if double quotes were encountered. 
	int idxFin = 0; //index for final version of the tokens list
	int countDQ; //counting double quotes in a string.
	for(int i = 0; i < count; i++){
		//printf("break2\n");
		if(strchr(searchTokens[i], '\"')){
			countDQ = 0;
			//printf("break3\n");
			for(int j = 0; j < strlen(searchTokens[i]); j++){
				if( searchTokens[i][j] == '\"') countDQ++;
			}
			if(countDQ == 2) {
				//printf("break3.1\n");
				finalTokens[idxFin] = (char*)malloc(BUFSIZ * sizeof(char));
				strcpy(finalTokens[idxFin], searchTokens[i]);
				idxFin++;
			}
			else if (countDQ == 1){
				if(quotes == 0) {
					quotes = 1;
					if(strlen(searchTokens[i]) == 1){
						//printf("break3.2.1\n");
						finalTokens[idxFin] = (char*)malloc(BUFSIZ * sizeof(char));
						strcat(finalTokens[idxFin], searchTokens[i]);
						//strcat(finalTokens[idxFin], " ");
					}
					else {
						//printf("break3.2.2\n");
						finalTokens[idxFin] = (char*)malloc(BUFSIZ * sizeof(char));
						strcat(finalTokens[idxFin], searchTokens[i]);
					}
				}
				else if(quotes == 1){
					quotes = 0;
					if(strlen(searchTokens[i]) == 1){
						//printf("break3.3.1\n");
						strcat(finalTokens[idxFin], " ");
						strcat(finalTokens[idxFin], searchTokens[i]);
					}
					else {
						//printf("break3.3.2\n");
						strcat(finalTokens[idxFin], " ");
						strcat(finalTokens[idxFin], searchTokens[i]);
					}
					idxFin++;
				}
			}
		}
		else{
			//printf("break4\n");
			if(quotes == 1){
				strcat(finalTokens[idxFin], " ");
				strcat(finalTokens[idxFin], searchTokens[i]);
				//strcat(finalTokens[idxFin], " "); //space 추가에 대한 키워드도 따져야한다.... 
			}
			else if(quotes == 0){
				//printf("break5\n");
				finalTokens[idxFin] = (char*)malloc(BUFSIZ * sizeof(char));
				strcpy(finalTokens[idxFin], searchTokens[i]);
				//printf("break6\n");
				idxFin++;
			}
		}
	}

	// if more than 8 keywords, invalid!
	if(idxFin > 8){
		exit(-1);
	}
	
	printf("Final Tokens: \n");
	for(int i = 0; i < idxFin; i++){
		int type = search_type(finalTokens[i]);
        // printf("안녕%s\n", token);

        if(type == 5) {
            strcpy(basic, finalTokens[i]);
            printf("General String -> %s\n", finalTokens[i]);
        } else if(type == 0) {
            strcpy(star, finalTokens[i]);
            printf("\'*\' prefix String -> %s\n", finalTokens[i]);
        } else if(type == 1) {
            strcpy(stick, finalTokens[i]);
            printf("\'-\' has no String -> %s\n", finalTokens[i]);
        } else if(type == 2) {
            strcpy(book, finalTokens[i]);
            printf("\'book:\' book String -> %s\n", finalTokens[i]);
        } else if(type == 3) {
            strcpy(ch, finalTokens[i]);
            printf("\'chapter:\' chapter String -> %s\n", finalTokens[i]);
        } else if(type == 4) {
            strcpy(quo, finalTokens[i]);
            printf("\'double quote:\' identical String -> %s\n", finalTokens[i]);
        }
	}
	printf("\n");


    // 합치기 공간 끝

    fp_niv = fopen("NIV.txt", "r") ;
    
	char * s = 0x0;

    while((s = read_a_line())) {
        int ba_check = 0, sr_check = 0, sk_check = 0, bo_check = 0, ch_check = 0, quo_check = 0;
        char s_temp[MAX];
        char basic_temp[VALUE];
        char star_temp[VALUE];
        char stick_temp[VALUE];
        char book_temp[VALUE];
        char ch_temp[VALUE];
        char quo_temp[VALUE];
        strcpy(basic_temp, basic);
        strcpy(star_temp, star);
        strcpy(stick_temp, stick);
        strcpy(book_temp, book);
        strcpy(ch_temp, ch);
        strcpy(quo_temp, quo);
        // basic func
        if(strlen(basic) != 0) {
            strcpy(s_temp, s);
            ba_check = basic_token(s_temp, basic_temp);
        } else
            ba_check = 1;
        // * func
        if(strlen(star) != 0) {
            strcpy(s_temp, s);
            sr_check = star_token(s_temp, star_temp);
        } else
            sr_check = 1;
        // - func
        if(strlen(stick) != 0) {
            strcpy(s_temp, s);
            sk_check = stick_token(s_temp, stick_temp);
        } else
            sk_check = 1;
        // book: func
        if(strlen(book) != 0) {
            strcpy(s_temp, s);
            bo_check = book_token(s_temp, book_temp);
        } else
            bo_check = 1;
        // chapter: func
        if(strlen(ch) != 0) {
            strcpy(s_temp, s);
            ch_check = ch_token(s_temp, ch_temp);
        } else
            ch_check = 1;
         if(strlen(quo) != 0) {
            strcpy(s_temp, s);
            quo_check = quo_token(s_temp, quo_temp);
        } else
            quo_check = 1;
        
        if(ba_check == 1 && sr_check == 1 && sk_check == 1 && bo_check == 1 && ch_check == 1 && quo_check == 1) {
            // printf("====================\n");
            printf("%s\n\n", s);
        }
		free(s);
	}

    fclose(fp_niv);
}