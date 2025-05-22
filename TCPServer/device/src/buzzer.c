#include "buzzer.h"
#include <stdio.h>
#include <unistd.h>  // read, write, usleep
#include <fcntl.h>   // fcntl, O_NONBLOCK
#include <errno.h>   // errno, EAGAIN, EWOULDBLOCK
#include <stdbool.h> // bool 타입
#include <string.h>  // memset, 기타 문자열 함수
#include <wiringPi.h>
#include <softTone.h>

#define BZR 29 //pin

void buzzerMain(int write_fd, int read_fd)
{
    char buffer[2];

    wiringPiSetup();
    softToneCreate(BZR);

    int flags = fcntl(read_fd, F_GETFL, 0);
    fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);

    bool gopause = false;
    unsigned int nowtime = millis();
    char prevCmd = 'x';
    int index = 0;

    while (1)
    {
        //명령어를 받는 부분
        if (millis() - nowtime > 0)
        {
            ssize_t n = read(read_fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // 문자열 종료
                if(prevCmd != buffer[0]){
                    prevCmd = buffer[0];

                    switch (buffer[0])
                    {
                    case '0':
                        gopause = false;
                        break;

                    case '1':
                        gopause = true;
                        break;
                    }
                }
            }

            nowtime = millis();
        }

        if(gopause){
            if(index >= 0 && index < NUM_NOTES){
                if(melody[index] == REST){
                    softToneWrite(BZR, 0);
                    delay(durations[index]);
                }
                else{
                    softToneWrite(BZR, melody[index]);
                    delay(durations[index]*0.6);
                }

                softToneWrite(BZR, 0);
                delay(durations[index]*0.3);

                index++;
            }else{
                index = 0;
            }
        }
        else{
            index = 0;
        }
    }
}
