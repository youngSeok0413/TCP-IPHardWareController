#include "sevenseg.h"
#include <stdio.h>
#include <unistd.h>  // read, write, usleep
#include <fcntl.h>   // fcntl, O_NONBLOCK
#include <errno.h>   // errno, EAGAIN, EWOULDBLOCK
#include <stdbool.h> // bool 타입
#include <string.h>  // memset, 기타 문자열 함수

#include <wiringPi.h>
#include <softTone.h>

#include "logger.h"

#define BZR 29 // pin

void segMain(int write_fd, int read_fd)
{
    char buffer[2];

    wiringPiSetup();
    softToneCreate(BZR);
    for (int i = 0; i < 4; i++)
    {
        pinMode(gpiopins[i], OUTPUT); /* 모든 Pin의 출력 설정 */
    }

    int flags = fcntl(read_fd, F_GETFL, 0);
    fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);

    unsigned int nowtime = millis();
    unsigned int cntdowntime = millis();
    int countdown = -1;
    char prevCmd = 'x';

    while (1)
    {
        if (millis() - nowtime > 200)
        {
            ssize_t n = read(read_fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // 문자열 종료
                if (prevCmd != buffer[0])
                {
                    prevCmd = buffer[0];
                    if (buffer[0] != 'x')
                    {
                        countdown = buffer[0] - '0';
                        log_init(LOG_FILE_PATH);
                        log_message(LOG_LEVEL_INFO, "TIMER:%d", countdown);
                        log_close();
                        fndControl(countdown);
                    } 
                    else{
                        countdown = -1;
                    }
                }
            }

            nowtime = millis();
        }

        if (countdown > -1)
        {
            if (millis() - cntdowntime > 1000)
            {
                fndControl(countdown--);
                cntdowntime = millis();
            }

            if (countdown == 0)
            {
                fndControl(countdown--);

                softToneWrite(BZR, 2637);
                delay(250);
                softToneWrite(BZR, 2349);
                delay(250);
                softToneWrite(BZR, 0);
            }
        }
    }
}

void fndControl(int num)
{
    for (int i = 0; i < 4; i++)
    { /* FND 초기화 */
        digitalWrite(gpiopins[i], HIGH);
    }

    for (int i = 0; i < 4; i++)
    {
        digitalWrite(gpiopins[i], number[num][i] ? HIGH : LOW); /* FND에 값을 출력 */
    }
}