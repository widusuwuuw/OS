#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

static struct stats {
  int ncopyin;
  int ncopyinstr;
} stats;

int
statscopyin(char *buf, int sz) {
  int n;
  n = snprintf(buf, sz, "copyin: %d\n", stats.ncopyin);
  n += snprintf(buf+n, sz, "copyinstr: %d\n", stats.ncopyinstr);
  return n;
}

// New copyin, without the pagetable argument.
int
copyin_new(char *dst, uint64 srcva, uint64 len)
{
  struct proc *p = myproc();
  if (srcva >= p->sz || srcva + len < srcva || srcva + len > p->sz) {
    return -1;
  }
  memmove(dst, (void *)srcva, len);
  stats.ncopyin++;
  return 0;
}

// New copyinstr, without the pagetable argument.
int
copyinstr_new(char *dst, uint64 srcva, uint64 max)
{
  struct proc *p = myproc();
  if (srcva >= p->sz) {
    return -1;
  }

  stats.ncopyinstr++;
  int got_null = 0;
  for(int i = 0; i < max; i++){
    if(srcva + i >= p->sz) {
      break;
    }
    dst[i] = ((char *)srcva)[i];
    if(dst[i] == 0){
      got_null = 1;
      break;
    }
  }

  if(!got_null){
    return -1;
  }
  
  return 0;
}