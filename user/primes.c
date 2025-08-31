#include "kernel/types.h"
#include "user/user.h"

void sieve(int p_read_end);

int main(int argc, char *argv[])
{
  int p[2];
  pipe(p);

  if (fork() == 0) {
    close(p[1]);
    sieve(p[0]);
  } else {
    close(p[0]);
    for (int i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(int));
    }
    close(p[1]); 
    wait(0);
  }
  exit(0);
}

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
    close(right_pipe[1]);
    close(p_read_end);
    sieve(right_pipe[0]);
  } else {
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
