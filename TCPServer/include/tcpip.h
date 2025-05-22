#ifndef TCPIP_H
#define TCPIP_H

#include <netinet/in.h>

#define SERVER_PORT 5100
#define BUFSIZE 1024
#define MAX_EVENTS 64

/* 서버 초기화 */
int setup_server_socket(int *listen_fd);

/* epoll 초기화 */
int setup_epoll(int listen_fd);

/* 클라이언트 연결 처리 */
void handle_new_connection(int epoll_fd, int listen_fd);

/* 클라이언트 메시지 처리 */
void handle_client_data(int epoll_fd, int client_fd, char* cntrl);

/* 문자열 송신 함수 */
int sendStrTCPIP(int sockfd, const char *msg);

/* 문자열 수신 함수 */
int recvStrTCPIP(int sockfd, char *buf, size_t bufsize);

/* 자원 정리 */
void cleanup();

/* 시그널 핸들러 */
//void signal_handler(int sig);

/*시간 체크*/
void check_timeouts(int epoll_fd);

#endif // TCPIP_H
