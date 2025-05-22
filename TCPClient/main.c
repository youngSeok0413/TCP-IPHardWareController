#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>

#define TCP_PORT 5100
#define USERIDSIZ 20
#define MSGSIZ BUFSIZ

/* Global Variables */
char userid[USERIDSIZ];          // 사용자 ID
char send_msg[MSGSIZ];           // 전송 메시지 버퍼
char recv_msg[MSGSIZ];           // 수신 메시지 버퍼

int ssock = -1;                  // 소켓 디스크립터 (-1은 미연결 상태)
struct sockaddr_in servaddr;     // 서버 주소 구조체

/* Function Prototypes */
char prepareMessage();
void cleanup();
void signal_handler(int sig);

int main(int argc, char **argv)
{
    // 시그널 핸들러 등록 (Ctrl+C 등)
    signal(SIGINT, signal_handler);

    if(argc < 2) {
        fprintf(stderr, "Usage : %s <IP_ADDRESS>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 사용자 ID 입력 */
    printf("Write user id: ");
    fgets(userid, USERIDSIZ, stdin);
    userid[strcspn(userid, "\n")] = '\0';  // 개행 문자 제거

    /* 소켓 생성 */
    if((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return EXIT_FAILURE;
    }

    /* 서버 주소 설정 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &(servaddr.sin_addr.s_addr)) <= 0) {
        fprintf(stderr, "올바른 IPv4 주소를 입력하세요 (예: 127.0.0.1)\n");
        cleanup();
        return EXIT_FAILURE;
    }
    servaddr.sin_port = htons(TCP_PORT);

    /* 서버에 연결 */
    if(connect(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        cleanup();
        return EXIT_FAILURE;
    }

    printf("서버에 연결되었습니다. 메시지를 입력하세요.\n");

    /* 메시지 전송 루프 */
    while (1) {
        memset(send_msg, 0, sizeof(send_msg));
        memset(recv_msg, 0, sizeof(recv_msg));

        char command = prepareMessage();

        // 서버에 메시지 전송
        if (send(ssock, send_msg, strlen(send_msg), 0) <= 0) {
            perror("send()");
            break;
        }

        // 'q' 명령어 처리
        /*if (command == 'q') {
            printf("종료 명령을 전송했습니다. 연결을 종료합니다.\n");
            break;
        }*/

        // 서버 응답 수신
        int len = recv(ssock, recv_msg, sizeof(recv_msg) - 1, 0);
        if (len > 0) {
            recv_msg[len] = '\0';
            printf("서버 응답: %s\n", recv_msg);
        } else if (len == 0) {
            printf("서버 연결이 종료되었습니다.\n");
            break;
        } else {
            perror("recv()");
            break;
        }
    }

    cleanup();
    return EXIT_SUCCESS;
}

/* 메시지 구성 함수 */
char prepareMessage() {
    char command;
    char led = 'x', buzzer = 'x';
    int timer = -1;

    printf("명령어를 입력하세요 (c: 제어, q: 종료): ");
    scanf(" %c", &command);

    if (command == 'c') {
        printf("LED 세기 입력 (x, 1, 2, 3 중 하나): ");
        scanf(" %c", &led);

        printf("부저 ON/OFF 입력 (x, 0: OFF, 1: ON): ");
        scanf(" %c", &buzzer);

        printf("타이머 설정 (x, 0 ~ 9 숫자): ");
        scanf(" %d", &timer);

        snprintf(send_msg, sizeof(send_msg), "%s:%c:%c:%c:%d",
                 userid, command, led, buzzer, timer);
        return command;

    } else if (command == 'q') {
        snprintf(send_msg, sizeof(send_msg), "%s:%c:x:x:x", userid, command);
        return command;

    } else {
        // 유효하지 않은 명령어 -> 기본 메시지 전송
        printf("잘못된 명령입니다. 기본 메시지를 서버에 전송합니다.\n");
        snprintf(send_msg, sizeof(send_msg), "%s:c:x:x:x", userid);
        return 'i';  // invalid
    }
}

/* 종료 시 자원 해제 */
void cleanup() {
    if (ssock != -1) {
        close(ssock);
        ssock = -1;
        printf("소켓 연결이 해제되었습니다.\n");
    }
    memset(send_msg, 0, sizeof(send_msg));
    memset(recv_msg, 0, sizeof(recv_msg));
}

/* SIGINT (Ctrl+C) 핸들러 */
void signal_handler(int sig) {
    printf("\nSIGINT 수신: 클라이언트 종료 중...\n");
    cleanup();
    exit(EXIT_SUCCESS);
}
