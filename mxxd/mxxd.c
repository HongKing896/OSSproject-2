#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 16
#define FILE_SIZE 30
#define MAX_LINE 1024

void txtToBin(char order[], char inputFile[], char outputFile[]) {
	char buffer[MAX_LINE];
    size_t bytes_read;
	size_t offset = 0;
	
	FILE *fp_input = fopen(inputFile, "r");
	if (fp_input == NULL) {
		printf("Error opening input file.\n");
		exit(0);
	}

	FILE *fp_output = tmpfile();
	if (fp_output == NULL) {
		printf("Error creating temporary file.\n");
		fclose(fp_input);
		exit(0);
	}
	
	while (fgets(buffer, MAX_LINE, fp_input) != NULL) { // Read File 1 line
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }

		char *word = strtok(buffer, " ");
		while(word != NULL) {
			word = strtok(NULL, " ");
			int valid = 1;

			for (int i = 0; i < strlen(word); i++) {
				if (!isxdigit(word[i])) {
					valid = 0;
					break;
				}
			}
			if(valid == 0)
				break;
			
			for (int i = 0; i < strlen(word); i += 2) {
				unsigned char byte;
				if (sscanf(&word[i], "%02hhx", &byte) != 1) {
					printf("Syntax error in input file.\n");
					fclose(fp_input);
					fclose(fp_output);
					exit(0);
				}
				fputc(byte, fp_output);
			}
		}
    }

    // rewind and write output file
    rewind(fp_output);
    FILE *fp_final = fopen(outputFile, "wb");
    if (fp_final == NULL) {
        printf("Error opening output file.\n");
        fclose(fp_input);
        fclose(fp_output);
		exit(0);
    }
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp_output)) > 0) {
        fwrite(buffer, 1, bytes_read, fp_final);
    }
    fclose(fp_input);
    fclose(fp_output);
    fclose(fp_final);
}

void binToTxt(char inputFile[], char outputFile[]) {
	char buffer[BUFFER_SIZE];
    FILE *fp_input, *fp_output;
    size_t bytesStore;

	int offset = 0;

    fp_input = fopen(inputFile, "r");  				// Read File
	fp_output = fopen(outputFile, "w"); 			// Write File

    while ((bytesStore = fread(buffer, 1, BUFFER_SIZE, fp_input)) > 0) { 	// 파일을 16바이트씩 읽어오기
        fprintf(fp_output, "%08x: ", offset);  								// 오프셋 쓰기
        for (int i = 0; i < bytesStore; i += 2) {
            fprintf(fp_output, "%02x%02x ", buffer[i], buffer[i+1]);  		// 16진수로 쓰기 (16진수를 2자리로 표시)
        }
        fprintf(fp_output, " ");

		// If it's not 16 bytes, align it to the 16-byte boundary.
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
	// Close File
    fclose(fp_input);
	fclose(fp_output);
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
		if(strcmp(order, "-r") != 0) {
			printf("error: please try again!\n");
			return 1;
	} else
		txtToBin(order, inputFile, outputFile);
	} else {
		strcpy(inputFile, argv[1]);
		strcpy(outputFile, argv[2]);
		binToTxt(inputFile, outputFile);
	}

    return 0;
}