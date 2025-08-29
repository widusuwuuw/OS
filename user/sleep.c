#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  // argc 是命令行参数的数量。
  // 对于命令 "sleep 10"，argc 的值是 2。
  // argv[0] 是 "sleep"，argv[1] 是 "10"。

  // 检查用户是否提供了且只提供了一个参数。
  if(argc != 2){
    // fprintf 可以向指定的文件描述符输出。fd=2 代表标准错误输出。
    fprintf(2, "Usage: sleep <ticks>\n");
    exit(1); // 使用非零状态码退出，表示程序出错。
  }

  // 使用 atoi 函数将字符串参数 (argv[1]) 转换为整数。
  // atoi 函数由 xv6 的用户库提供。
  int ticks = atoi(argv[1]);

  // 调用 sleep 系统调用，内核会暂停当前进程指定的 ticks 数量。
  // ticks 是 xv6 内部的时钟中断计数。
  int ret = sleep(ticks);

  // 如果 sleep 返回非零值，说明可能出错了。
  if(ret != 0) {
    fprintf(2, "sleep: failed\n");
    exit(1);
  }

  // 成功执行完毕，以状态码 0 退出。
  exit(0);
}
