#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

#define LOG_FILE_PATH "/tmp/tcp_server.log"

// 로그 레벨 정의
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL
} LogLevel;

// 로그 초기화 (경로 지정)
int log_init(const char *filepath);

// 로그 기록
void log_message(LogLevel level, const char *format, ...);

// 로그 종료
void log_close();

#endif // LOGGER_H
