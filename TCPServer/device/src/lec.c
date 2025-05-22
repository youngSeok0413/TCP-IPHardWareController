#include "led.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>
#include <softPwmCreate>

#define LED     5
#define CDS     6

#define WEAK    50
#define NORMAL  150
#define STRONG  255

void ledMain(int write_fd, int read_fd) {
    char buffer[2];

    wiringPiSetup();
    pinMode(CDS, INPUT);
    pinMode(LED, OUTPUT);
    softPWMCreate(LED, 0, 255);

    bool onoff = false;
    int strength = 50;

    while (1) {
        ssize_t n = read(read_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';  // 문자열 종료

            if(digitalRead(CDS) == HIGH){
                //night
                digitalWrite(LED, LOW);

                switch(buffer[0]){
                    case '0':
                    onoff ~= onoff;
                    break;
                    
                    case '1':
                    strength = WEAK;
                    break;

                    case '2':
                    strength = NORMAL;
                    break;
                    
                    case '3':
                    strength = STRONG;
                    break;
                }

                if(onoff){
                    softPwmWrite(LED, strength);
                }
                else{
                    softPwmWrite(LED, 0);
                }

                write(write_fd, "d", sizeof("d"));
            }
            else{
                //day : always turn off the light
                softPwmWrite(LED, 0);
                write(write_fd, "d", sizeof("d"));
            }
        }

        sleep(1);  // 주기적으로 응답 (옵션에 따라 조정 가능)
    }
}
