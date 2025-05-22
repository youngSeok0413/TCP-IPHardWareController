# 컴파일러 및 옵션
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
LDFLAGS =

# 디렉토리 설정
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
BIN = tcp_client

# 소스 파일
SRCS = main.c $(wildcard $(SRC_DIR)/*.c)

# 오브젝트 파일 (build 디렉토리 내에 생성)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))

# 기본 목표
all: $(BIN)

# 실행 파일 빌드 규칙
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# build 디렉토리에 object 파일 생성
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# build 디렉토리가 없으면 생성
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 정리
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN)
