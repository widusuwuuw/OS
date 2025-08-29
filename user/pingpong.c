#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  // p2c: parent to child pipe
  // c2p: child to parent pipe
  int p2c[2];
  int c2p[2];

  pipe(p2c);
  pipe(c2p);

  int pid = fork();

  if(pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // --- 子进程 ---

    // 子进程从 p2c 读，向 c2p 写。
    // 所以关闭 p2c 的写入端和 c2p 的读取端。
    close(p2c[1]);
    close(c2p[0]);

    char buf[1];
    // 从父进程读取一个字节
    if (read(p2c[0], buf, 1) != 1) {
      fprintf(2, "pingpong: child failed to read from parent\n");
      exit(1);
    }
    
    // 打印 "ping" 消息
    printf("%d: received ping\n", getpid());
    
    // 向父进程写回一个字节
    if (write(c2p[1], "B", 1) != 1) { // 写入的内容是什么不重要
      fprintf(2, "pingpong: child failed to write to parent\n");
      exit(1);
    }

    // 关闭剩下的文件描述符
    close(p2c[0]);
    close(c2p[1]);
    
    exit(0); // 子进程结束
  } else {
    // --- 父进程 ---

    // 父进程向 p2c 写，从 c2p 读。
    // 所以关闭 p2c 的读取端和 c2p 的写入端。
    close(p2c[0]);
    close(c2p[1]);

    // 向子进程写入一个字节
    if (write(p2c[1], "A", 1) != 1) { // 写入的内容是什么不重要
      fprintf(2, "pingpong: parent failed to write to child\n");
      exit(1);
    }

    // 等待子进程执行完毕并返回
    wait(0);

    char buf[1];
    // 从子进程读取一个字节
    if (read(c2p[0], buf, 1) != 1) {
      fprintf(2, "pingpong: parent failed to read from child\n");
      exit(1);
    }
    
    // 打印 "pong" 消息
    printf("%d: received pong\n", getpid());

    // 关闭剩下的文件描述符
    close(p2c[1]);
    close(c2p[0]);

    exit(0); // 父进程结束
  }
}
