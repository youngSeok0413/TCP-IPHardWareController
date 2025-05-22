#include "tcpip.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "device/include/led.h"
#include "device/include/buzzer.h"
#include "device/include/sevenseg.h"

#define NUM_DEVICES 3

char g_devNextStat[3] = {'x', 'x', 'x'};
char g_devNextStat_buffer[3] = {'x', 'x', 'x'};

int parent_to_child[NUM_DEVICES][2];
int child_to_parent[NUM_DEVICES][2];
pid_t child_pids[NUM_DEVICES];

void signal_handler(int signo) {
    for (int i = 0; i < NUM_DEVICES; ++i) {
        kill(child_pids[i], SIGTERM);
    }

    for (int i = 0; i < NUM_DEVICES; ++i) {
        waitpid(child_pids[i], NULL, 0);
    }

    cleanup();
    exit(EXIT_SUCCESS);
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void launch_child(int index, void (*child_func)(int, int), int epoll_fd) {
    if (pipe(parent_to_child[index]) < 0 || pipe(child_to_parent[index]) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 자식 프로세스
        close(parent_to_child[index][1]); // 부모 → 자식 쓰기 닫기
        close(child_to_parent[index][0]); // 자식 → 부모 읽기 닫기

        dup2(parent_to_child[index][0], STDIN_FILENO);  // 부모 → 자식
        dup2(child_to_parent[index][1], STDOUT_FILENO); // 자식 → 부모

        child_func(STDOUT_FILENO, STDIN_FILENO);
        exit(0); // 절대 도달하지 않음
    } else {
        // 부모 프로세스
        close(parent_to_child[index][0]); // 자식 읽기 닫기
        close(child_to_parent[index][1]); // 자식 쓰기 닫기

        set_nonblocking(child_to_parent[index][0]);
        struct epoll_event ev = {
            .events = EPOLLIN,
            .data.fd = child_to_parent[index][0]
        };
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, child_to_parent[index][0], &ev) == -1) {
            perror("epoll_ctl (pipe)");
            exit(EXIT_FAILURE);
        }

        child_pids[index] = pid;
    }
}

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("1차 fork 실패");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // 부모 종료
        exit(EXIT_SUCCESS);
    }

    // 세션 리더가 되어 터미널과 분리
    if (setsid() < 0) {
        perror("setsid 실패");
        exit(EXIT_FAILURE);
    }

    // 두 번째 fork
    pid = fork();
    if (pid < 0) {
        perror("2차 fork 실패");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // 두 번째 부모 프로세스 종료
        exit(EXIT_SUCCESS);
    }

    // 파일 권한 마스크 설정
    umask(0);

    // 루트 디렉터리로 이동
    if (chdir("/") < 0) {
        perror("chdir 실패");
        exit(EXIT_FAILURE);
    }

    // 표준 입출력 닫기
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // /dev/null로 리디렉션 (선택 사항)
    open("/dev/null", O_RDONLY); // stdin
    open("/dev/null", O_WRONLY); // stdout
    open("/dev/null", O_RDWR);   // stderr
}

int main() {

    //daemonize(); 데몬화 시 makefile 확인할 것

    signal(SIGINT, signal_handler);

    int listen_fd;
    if (setup_server_socket(&listen_fd) < 0) {
        fprintf(stderr, "서버 소켓 설정 실패\n");
        return EXIT_FAILURE;
    }

    int epoll_fd = setup_epoll(listen_fd);
    if (epoll_fd < 0) {
        fprintf(stderr, "epoll 설정 실패\n");
        cleanup();
        return EXIT_FAILURE;
    }

    // 자식 프로세스 시작 (epoll 등록 포함)
    launch_child(0, ledMain, epoll_fd);
    launch_child(1, buzzerMain, epoll_fd);
    launch_child(2, segMain, epoll_fd);

    printf("서버 시작됨: 포트 %d에서 대기 중...\n", SERVER_PORT);

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        check_timeouts(epoll_fd);

        if (nfds == -1) {
            perror("epoll_wait()");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                handle_new_connection(epoll_fd, listen_fd);
            } else {
                int is_child_pipe = 0;
                for (int j = 0; j < NUM_DEVICES; ++j) {
                    if (fd == child_to_parent[j][0]) {
                        char buf[128];
                        ssize_t len = read(fd, buf, sizeof(buf) - 1);
                        if (len > 0) {
                            buf[len] = '\0';
                            printf("자식[%d] 출력: %s, %d \n", j, buf, strlen(buf));
                        }
                        is_child_pipe = 1;
                        break;
                    }
                }

                // 자식에게 현재 상태 전달
                for (int j = 0; j < NUM_DEVICES; ++j) {
                    write(parent_to_child[j][1], &g_devNextStat[j], 1);
                }

                if (!is_child_pipe) {
                    handle_client_data(epoll_fd, fd, g_devNextStat_buffer);
                    for(int i = 0; i < 3; i++){
                        g_devNextStat[i] = g_devNextStat_buffer[i];
                    }
                }
            }
        }
    }

    signal_handler(SIGINT);
    return EXIT_SUCCESS;
}
