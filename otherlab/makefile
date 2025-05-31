CC = gcc
YACC = bison
LEX = flex
CFLAGS = -Wall -g

# 目标可执行文件名称
TARGET = custom_compiler

# 生成的中间文件
YACC_OUTPUT = subc_ast.tab.c subc_ast.tab.h
LEX_OUTPUT = lex.yy.c

# 源文件
YACC_SRC = subc_ast.y
LEX_SRC = subc.l
AST_SRC = ast.c
AST_HDR = ast.h

all: $(TARGET)

$(TARGET): $(YACC_OUTPUT) $(LEX_OUTPUT) ast.o
	$(CC) $(CFLAGS) -o $(TARGET) subc_ast.tab.c lex.yy.c ast.o

ast.o: $(AST_SRC) $(AST_HDR)
	$(CC) $(CFLAGS) -c $(AST_SRC)

$(YACC_OUTPUT): $(YACC_SRC)
	$(YACC) -d -o subc_ast.tab.c $(YACC_SRC)

$(LEX_OUTPUT): $(LEX_SRC)
	$(LEX) $(LEX_SRC)

clean:
	rm -f $(TARGET) $(YACC_OUTPUT) $(LEX_OUTPUT) *.o

# 测试目标
test: $(TARGET)
	./$(TARGET) test_program.txt

# 调试版本
debug: CFLAGS += -DYYDEBUG=1
debug: $(TARGET)

.PHONY: all clean test debug