#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
  int p1[2], p2[2];
  pipe(p1);
  pipe(p2);
  char buf[8];

  if(fork() == 0){
    // Child
    read(p1[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);
    write(p2[1], "pong", 4);
  } else {
    // Parent
    write(p1[1], "ping", 4);
    read(p2[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);
  }
  exit(0);
}
