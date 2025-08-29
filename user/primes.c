#include "kernel/types.h"
#include "user/user.h"

// 函数声明：sieve 接收一个整数（文件描述符）
void sieve(int p_read_end);

int main(int argc, char *argv[])
{
  int p[2];
  pipe(p);

  if (fork() == 0) {
    // 子进程
    close(p[1]);
    // 调用 sieve，传入管道的读取端文件描述符 p[0] (这是一个 int)
    sieve(p[0]);
  } else {
    // 父进程
    close(p[0]);

    for (int i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(int));
    }
    
    close(p[1]); 
    wait(0);
  }

  exit(0);
}

// 函数定义：参数名改为 p_read_end，类型是 int，与声明完全匹配
void sieve(int p_read_end)
{
  int prime, num;

  if (read(p_read_end, &prime, sizeof(int)) == 0) {
    close(p_read_end);
    exit(0);
  }
  printf("prime %d\n", prime);

  int right_pipe[2];
  pipe(right_pipe);

  if (fork() == 0) {
    // 子进程
    close(right_pipe[1]);
    close(p_read_end);
    // 递归调用，传入新管道的读取端 right_pipe[0] (这是一个 int)
    sieve(right_pipe[0]);
  } else {
    // 当前进程
    close(right_pipe[0]);

    while (read(p_read_end, &num, sizeof(int)) > 0) {
      if (num % prime != 0) {
        write(right_pipe[1], &num, sizeof(int));
      }
    }
    
    close(p_read_end);
    close(right_pipe[1]);
    wait(0);
  }
  
  exit(0);
}