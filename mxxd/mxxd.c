#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 16
#define FILE_SIZE 30

void txtToBin(char inputFile[], char outputFile[]) {
	char buffer[BUFFER_SIZE];
    FILE *fp_input, *fp_output;
    size_t bytesStore;

	int offset = 0;

    fp_input = fopen(inputFile, "r");  				// 파일 열기
	fp_output = fopen(outputFile, "w"); 			// 파일 쓰기

    while ((bytesStore = fread(buffer, 1, BUFFER_SIZE, fp_input)) > 0) { 	// 파일을 16바이트씩 읽어오기
        fprintf(fp_output, "%08x: ", offset);  								// 오프셋 쓰기
        for (int i = 0; i < bytesStore; i += 2) {
            fprintf(fp_output, "%02x%02x ", buffer[i], buffer[i+1]);  		// 16진수로 쓰기 (16진수를 2자리로 표시)
        }
        fprintf(fp_output, " ");

		// 16바이트가 아닐 경우, 16바이트 기준으로 정렬
		int tempByte = bytesStore;
		if(tempByte != BUFFER_SIZE) {
			while(tempByte != BUFFER_SIZE) {
					fprintf(fp_output, "  ");
					tempByte++;
			}
		}

        for (int i = 0; i < bytesStore; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) {  					// ASCII 문자인 경우
                fprintf(fp_output, "%c", buffer[i]);  						// 해당 문자 쓰기
            } else {
                fprintf(fp_output, ".");  									// 아스키 문자가 아닌 경우 '.' 쓰기
            }
        }
        fprintf(fp_output, "\n");
        offset += bytesStore;  												// 오프셋 증가
    }

	// 파일 닫기
    fclose(fp_input);
	fclose(fp_output);
}

void binToTxt(char order[], char inputFile[], char outputFile[]) {
	char buffer[BUFFER_SIZE];
    FILE *fp;
    size_t bytes_read;

	if(strcmp(order, "-r") == 0) {
		printf("success\n");
	} else {
		printf("error: please try again!\n");
	}
}

int main (int argc, char ** argv)
{
	char buffer[BUFFER_SIZE];
    FILE *fp;
    size_t bytes_read;

	char order[5], inputFile[FILE_SIZE], outputFile[FILE_SIZE];

	if(argc == 4) {
		strcpy(order, argv[1]);
		strcpy(inputFile, argv[2]);
		strcpy(outputFile, argv[3]);
		binToTxt(order, inputFile, outputFile);
	} else {
		strcpy(inputFile, argv[1]);
		strcpy(outputFile, argv[2]);
		txtToBin(inputFile, outputFile);
	}

    return 0;
}