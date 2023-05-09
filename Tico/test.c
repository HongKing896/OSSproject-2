#include <stdio.h>

int main() {
    int arr[10];
    int freq[10] = {0};
    int maxFreq = -1, maxIndex = -1;

    // 10개의 정수 입력받기
    for(int i = 0; i < 10; i++) {
        printf("정수를 입력하세요: ");
        scanf("%d", &arr[i]);
    }

    // 각 숫자의 빈도수 계산하기
    for(int i = 0; i < 10; i++) {
        int count = 1;
        for(int j = i+1; j < 10; j++) {
            if(arr[i] == arr[j]) {
                count++;
            }
        }
        freq[i] = count;
    }

    // 가장 많이 나온 숫자 찾기
    for(int i = 0; i < 10; i++) {
        if(freq[i] > maxFreq) {
            maxFreq = freq[i];
            maxIndex = i;
        }
    }

    printf("가장 많이 나온 숫자: %d\n", arr[maxIndex]);

    return 0;
}