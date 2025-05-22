#include "tcpip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <time.h>

#include "parser.h"

#define TIMEOUT_SECONDS 30
#define MAX_CLIENTS 1024

typedef struct {
    int fd;
    time_t last_active;
    char userid[20];
} client_session;

static int listen_fd = -1;
static int epoll_fd = -1;

char authorized_userid[20] = {0, };
static client_session clients[MAX_CLIENTS];

int setup_server_socket(int *out_listen_fd) {
    struct sockaddr_in server_addr;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket()");
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind()");
        close(fd);
        return -1;
    }

    if (listen(fd, SOMAXCONN) < 0) {
        perror("listen()");
        close(fd);
        return -1;
    }

    *out_listen_fd = fd;
    listen_fd = fd;
    return 0;
}

int setup_epoll(int lfd) {
    int efd = epoll_create1(0);
    if (efd == -1) {
        perror("epoll_create1()");
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &ev) < 0) {
        perror("epoll_ctl()");
        close(efd);
        return -1;
    }

    epoll_fd = efd;
    return efd;
}

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/*New Client*/ 
static void add_client_session(int fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd == 0) {
            clients[i].fd = fd;
            clients[i].last_active = time(NULL);
            clients[i].userid[0] = '\0';
            break;
        }
    }
}

static void update_client_activity(int fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd == fd) {
            clients[i].last_active = time(NULL);
            break;
        }
    }
}

static void update_client_userid(int fd, const char *userid) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd == fd && clients[i].userid[0] == '\0') {
            strncpy(clients[i].userid, userid, sizeof(clients[i].userid) - 1);
            clients[i].userid[sizeof(clients[i].userid) - 1] = '\0';
            break;
        }
    }
}

static void remove_client_session(int fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd == fd) {
            clients[i].fd = 0;
            clients[i].last_active = 0;
            clients[i].userid[0] = '\0';
            break;
        }
    }
}

void handle_new_connection(int efd, int lfd) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int client_fd = accept(lfd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0) {
        perror("accept()");
        return;
    }

    printf("클라이언트 연결됨: %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    set_nonblocking(client_fd);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl() - client_fd");
        close(client_fd);
        return;
    }

    add_client_session(client_fd); // 활동 시간 등록
}

/*Action!!*/
void handle_client_data(int efd, int client_fd, char* cntrl) {
    char buffer[BUFSIZE];
    int len = recvStrTCPIP(client_fd, buffer, BUFSIZE);

    if (len <= 0) {
        if (len < 0) perror("recv()");
        else printf("클라이언트 종료됨 (fd: %d)\n", client_fd);

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if(clients[i].fd == client_fd 
                && strcmp(clients[i].userid, authorized_userid) == 0){
                    printf("authorized_userid 초기화됨 (unconnected)\n");
                    memset(authorized_userid, 0, sizeof(authorized_userid));
                    break;
            }
        }

        epoll_ctl(efd, EPOLL_CTL_DEL, client_fd, NULL);
        close(client_fd);
        remove_client_session(client_fd);
        return;
    }

    update_client_activity(client_fd);  // 활동 시간 갱신

    char** tokens; int count = 0;
    split_string(buffer, ':', &tokens, &count);

    const char *userid = tokens[0];

    // userid 저장
    update_client_userid(client_fd, userid);

    if (strlen(authorized_userid) <= 0) {
        strncpy(authorized_userid, userid, sizeof(authorized_userid) - 1);
        authorized_userid[sizeof(authorized_userid) - 1] = '\0';
    }

    if (strcmp(authorized_userid, userid) == 0) {

        cntrl[0] = tokens[2][0];
        cntrl[1] = tokens[3][0];
        cntrl[2] = tokens[4][0];

        printf("state : %c%c%c\r\n", 
            cntrl[0],cntrl[1],cntrl[2]);

        sendStrTCPIP(client_fd, buffer); // Echo
        
        //fork(p - tcp server), (c - device), messageque

    } else {
        const char *msg = "현재 기기 사용 중입니다. 나중에 이용해주세요.\n";
        sendStrTCPIP(client_fd, msg);
    }

    freeTokens(tokens, count);
}


int sendStrTCPIP(int sockfd, const char *msg) {
    size_t len = strlen(msg);
    ssize_t sent = send(sockfd, msg, len, 0);
    return (sent == (ssize_t)len) ? 0 : -1;
}

int recvStrTCPIP(int sockfd, char *buf, size_t bufsize) {
    int n = recv(sockfd, buf, bufsize - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
    }
    return n;
}

void cleanup() {
    if (listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
    if (epoll_fd != -1) {
        close(epoll_fd);
        epoll_fd = -1;
    }
    printf("서버 리소스 해제 완료\n");
}

/*
void signal_handler(int sig) {
    printf("\nSIGINT 수신, 서버 종료 중...\n");
    cleanup();
    exit(EXIT_SUCCESS);
}
*/

void check_timeouts(int efd) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd != 0 && difftime(now, clients[i].last_active) > TIMEOUT_SECONDS) {
            int fd = clients[i].fd;
            printf("타임아웃 발생: 클라이언트(fd: %d, userid: %s) 연결 해제\n", fd, clients[i].userid);

            epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);

            if (strcmp(clients[i].userid, authorized_userid) == 0) {
                printf("authorized_userid 초기화됨 (timeout 발생)\n");
                memset(authorized_userid, 0, sizeof(authorized_userid));
            }

            remove_client_session(fd);
        }
    }
}

