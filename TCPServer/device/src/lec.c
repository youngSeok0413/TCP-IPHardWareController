#include "led.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void ledMain(int write_fd, int read_fd) {
    char buffer[2];

    while (1) {
        ssize_t n = read(read_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';  // 문자열 종료

            printf("LED : %s", buffer);
            
            // 받은 문자열을 그대로 echo
            write(write_fd, buffer, strlen(buffer));
        }

        sleep(1);  // 주기적으로 응답 (옵션에 따라 조정 가능)
    }
}
