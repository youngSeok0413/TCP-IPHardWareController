#include "led.h"
#include <stdio.h>
#include <unistd.h>   // read, write, usleep
#include <fcntl.h>    // fcntl, O_NONBLOCK
#include <errno.h>    // errno, EAGAIN, EWOULDBLOCK
#include <stdbool.h>  // bool 타입
#include <string.h>   // memset, 기타 문자열 함수
#include <wiringPi.h> // wiringPi 관련 함수들
#include <softPwm.h>  // softPwm 함수

#define LED 5
#define CDS 6

#define WEAK 50
#define NORMAL 150
#define STRONG 255

void ledMain(int write_fd, int read_fd)
{
    char buffer[2];

    wiringPiSetup();
    // read_fd를 논블로킹 모드로 설정 (필요시)
    int flags = fcntl(read_fd, F_GETFL, 0);
    fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);

    pinMode(CDS, INPUT);
    pinMode(LED, OUTPUT);
    softPwmCreate(LED, 0, 255);

    bool onoff = false;
    int strength = NORMAL;
    unsigned int nowtime = millis();
    unsigned int daynightChecker = millis();
    char prevCmd = 'x';

    while (1)
    {
        if (nowtime - millis() > 200)
        {
            ssize_t n = read(read_fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // 문자열 종료

                if (prevCmd != buffer[0])
                {
                    prevCmd = buffer[0];
                    switch (buffer[0])
                    {
                    case '0':
                        onoff = !onoff;
                        break;

                    case '1':
                        strength = WEAK;
                        onoff = true;
                        break;

                    case '2':
                        strength = NORMAL;
                        onoff = true;
                        break;

                    case '3':
                        strength = STRONG;
                        onoff = true;
                        break;
                    }

                    write(write_fd, "change", sizeof("change"));
                }
            }

            nowtime = millis();
        }

        if (digitalRead(CDS) == HIGH)
        {
            // night
            if (onoff)
            {
                softPwmWrite(LED, 255 - strength);
            }
            else
            {
                softPwmWrite(LED, 255);
            }

            if (millis() - daynightChecker > 10000)
            {
                daynightChecker = millis();
                write(write_fd, "n", sizeof("n"));
            }
        }
        else
        {
            // day : always turn off the light
            softPwmWrite(LED, 255);

            if (millis() - daynightChecker > 10000)
            {
                daynightChecker = millis();
                write(write_fd, "d", sizeof("d"));
            }
        }
    }
}
