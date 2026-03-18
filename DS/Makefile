# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++20 -Wall -Iinclude

# 目标程序名
TARGET = main

# 源文件
SRC = $(wildcard src/*.cpp)

# 目标文件（.cpp → .o）
OBJ = $(patsubst src/%.cpp, build/%.o, $(SRC))

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

# 编译规则（核心）
build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 运行
run: all
	./$(TARGET)

# 清理
clean:
	rm -rf build $(TARGET)

# 伪目标（防止冲突）
.PHONY: all clean run