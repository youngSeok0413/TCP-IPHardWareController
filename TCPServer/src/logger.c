#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static FILE *log_file = NULL;

const char* level_to_string(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:    return "DEBUG";
        case LOG_LEVEL_INFO:     return "INFO";
        case LOG_LEVEL_WARNING:  return "WARNING";
        case LOG_LEVEL_ERROR:    return "ERROR";
        case LOG_LEVEL_CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

int log_init(const char *filepath) {
    log_file = fopen(filepath, "a");
    if (!log_file) {
        return -1; // 파일 열기 실패
    }
    setvbuf(log_file, NULL, _IOLBF, 0); // 라인 버퍼링
    return 0;
}

void log_message(LogLevel level, const char *format, ...) {
    if (!log_file) return;

    // 시간 포맷 생성
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    // 가변 인자 처리
    va_list args;
    va_start(args, format);
    fprintf(log_file, "[%s] [%s] ", time_buf, level_to_string(level));
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    va_end(args);
}

void log_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
