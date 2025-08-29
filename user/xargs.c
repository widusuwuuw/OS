#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    // 准备要传递给 exec 的参数数组
    // new_argv[0] 是要执行的命令
    // new_argv[1]... 是固定参数
    // 最后一个位置留给从标准输入读取的参数
    char *new_argv[MAXARG];
    
    // 检查参数数量
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }
    
    // 将 xargs 的参数（除了 "xargs" 本身）复制到新参数数组
    for (int i = 1; i < argc; i++) {
        new_argv[i - 1] = argv[i];
    }

    char line_buf[512]; // 用于存储从标准输入读取的一行
    char *p = line_buf;

    // 循环读取标准输入的每一个字符
    while (read(0, p, 1) > 0) {
        if (*p == '\n') {
            // 一行读取完毕
            *p = '\0'; // 替换换行符为字符串结束符
            
            // 将读取到的行作为最后一个参数
            new_argv[argc - 1] = line_buf;
            new_argv[argc] = 0; // exec 的参数数组必须以空指针结尾

            if (fork() == 0) {
                // 子进程
                exec(new_argv[0], new_argv);
                fprintf(2, "xargs: exec %s failed\n", new_argv[0]);
                exit(1);
            } else {
                // 父进程
                wait(0);
            }
            // 重置行缓冲区指针，准备读取下一行
            p = line_buf; 
        } else {
            // 继续读取下一个字符
            p++;
        }
    }
    
    // 处理最后一行（如果文件末尾没有换行符）
    if (p > line_buf) {
        *p = '\0';
        new_argv[argc - 1] = line_buf;
        new_argv[argc] = 0;
        if (fork() == 0) {
            exec(new_argv[0], new_argv);
            fprintf(2, "xargs: exec %s failed\n", new_argv[0]);
            exit(1);
        } else {
            wait(0);
        }
    }

    exit(0);
}
