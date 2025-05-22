# 컴파일러 및 옵션
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
INCLUDES = -Iinclude -Idevice/include

# 디렉토리
SRC_DIR = src
DEVICE_DIR = device/src
BUILD_DIR = build
DEVICE_BUILD = $(BUILD_DIR)/device
CORE_BUILD = $(BUILD_DIR)/core

# 파일 정의
DEVICE_SRC = $(wildcard $(DEVICE_DIR)/*.c)
DEVICE_OBJ = $(patsubst $(DEVICE_DIR)/%.c,$(DEVICE_BUILD)/%.o,$(DEVICE_SRC))

CORE_SRC = $(wildcard $(SRC_DIR)/*.c)
CORE_OBJ = $(patsubst $(SRC_DIR)/%.c,$(CORE_BUILD)/%.o,$(CORE_SRC))

MAIN_OBJ = $(BUILD_DIR)/main.o

# 라이브러리 명
SHARED_LIB = libdevice.so
STATIC_LIB = libcore.a

# 최종 실행 파일
TARGET = tcp_server

# rpath 설정
RPATH = -Wl,-rpath=.

.PHONY: all clean

all: $(TARGET)

# 최종 실행 파일 생성
$(TARGET): $(SHARED_LIB) $(STATIC_LIB) $(MAIN_OBJ)
	$(CC) -o $@ $(MAIN_OBJ) -L. -lcore -L. -ldevice $(RPATH)

# 공유 라이브러리 빌드
$(SHARED_LIB): $(DEVICE_OBJ)
	$(CC) -shared -o $@ $^

# 정적 라이브러리 빌드
$(STATIC_LIB): $(CORE_OBJ)
	ar rcs $@ $^

# 개별 오브젝트 파일 빌드
$(DEVICE_BUILD)/%.o: $(DEVICE_DIR)/%.c
	@mkdir -p $(DEVICE_BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CORE_BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(CORE_BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/main.o: main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) *.o *.so *.a $(TARGET)
