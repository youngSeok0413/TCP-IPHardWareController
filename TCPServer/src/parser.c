#include "parser.h"

// 문자열을 구분자로 나누고 결과를 이중 포인터에 할당
int split_string(const char* str, char delimiter, char*** result, int* count) {
    int tokens = 0;
    const char* temp = str;

    // 1. 토큰 개수 세기
    while (*temp) {
        if (*temp == delimiter) tokens++;
        temp++;
    }
    tokens++; // 마지막 토큰 포함

    *result = (char**)malloc(tokens * sizeof(char*));
    if (*result == NULL) {
        return -1; // 메모리 할당 실패
    }

    int start = 0;
    int end = 0;
    int tokenIndex = 0;
    int len = strlen(str);

    while (end <= len) {
        if (str[end] == delimiter || str[end] == '\0') {
            int tokenLen = end - start;

            // 동적으로 정확한 길이만큼 메모리 할당
            char* token = (char*)malloc((tokenLen + 1) * sizeof(char));
            if (token == NULL) {
                // 오류 발생 시 메모리 해제
                for (int i = 0; i < tokenIndex; i++) {
                    free((*result)[i]);
                }
                free(*result);
                return -1;
            }

            // 문자열 복사 및 null 종료
            memcpy(token, str + start, tokenLen);
            token[tokenLen] = '\0';

            (*result)[tokenIndex++] = token;
            start = end + 1;
        }
        end++;
    }

    *count = tokens;
    return 0; // 성공
}

void freeTokens(char** s_array, const int size){
    for(int i = 0; i < size; i++){
        free(s_array[i]);
    }
    free(s_array);
}