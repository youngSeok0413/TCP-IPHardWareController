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

# 소스/오브젝트 파일
DEVICE_SRC = $(wildcard $(DEVICE_DIR)/*.c)
DEVICE_OBJ = $(patsubst $(DEVICE_DIR)/%.c,$(DEVICE_BUILD)/%.o,$(DEVICE_SRC))

CORE_SRC = $(wildcard $(SRC_DIR)/*.c)
CORE_OBJ = $(patsubst $(SRC_DIR)/%.c,$(CORE_BUILD)/%.o,$(CORE_SRC))

MAIN_OBJ = $(BUILD_DIR)/main.o

# 라이브러리 명
SHARED_LIB = libdevice.so
STATIC_LIB = libcore.a

# 실행 타겟
TARGET = tcp_server

# WiringPi
WIRINGPI_LIB = -lwiringPi

# 런타임 rpath는 현재 디렉토리에 있는 .so를 찾도록 설정
RPATH = -Wl,-rpath='$$ORIGIN'

.PHONY: all clean

all: $(TARGET)

# 최종 실행 파일 생성
$(TARGET): $(MAIN_OBJ) $(STATIC_LIB) $(SHARED_LIB)
	@echo "🔗 Linking $(TARGET)..."
	$(CC) -o $@ $(MAIN_OBJ) -L. -lcore -ldevice $(RPATH)

# 공유 라이브러리 빌드
$(SHARED_LIB): $(DEVICE_OBJ)
	@echo "🔧 Building shared library: $@"
	$(CC) -shared -o $@ $^ $(WIRINGPI_LIB)

# 정적 라이브러리 빌드
$(STATIC_LIB): $(CORE_OBJ)
	@echo "📦 Building static library: $@"
	ar rcs $@ $^

# 디바이스 오브젝트 빌드
$(DEVICE_BUILD)/%.o: $(DEVICE_DIR)/%.c
	@mkdir -p $(DEVICE_BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 코어 오브젝트 빌드
$(CORE_BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(CORE_BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 메인 오브젝트 빌드
$(BUILD_DIR)/main.o: main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 정리
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) *.o *.so *.a $(TARGET)

.PHONY: kill
kill:
	@echo "🛑 Stopping tcp_server process..."
	@pids=$$(ps -eo pid,comm,args | grep '[t]cp_server' | grep -v grep | awk '$$2=="tcp_server" { print $$1 }'); \
	if [ -n "$$pids" ]; then \
		echo "Found process ID(s): $$pids"; \
		sudo kill -SIGINT $$pids; \
		sleep 1; \
		sudo kill -9 $$pids 2>/dev/null || true; \
	else \
		echo "No tcp_server process found."; \
	fi

#ps aux | grep tcp_server
#sudo killall tcp_server