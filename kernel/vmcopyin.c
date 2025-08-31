#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// New, safe copyin implementation.
int
copyin_new(char *dst, uint64 srcva, uint64 len)
{
  struct proc *p = myproc();
  if (srcva >= p->sz || srcva + len < srcva || srcva + len > p->sz) {
    return -1;
  }
  memmove(dst, (void *)srcva, len);
  return 0;
}

// New, safe copyinstr implementation.
int
copyinstr_new(char *dst, uint64 srcva, uint64 max)
{
  struct proc *p = myproc();
  if (srcva >= p->sz) {
    return -1;
  }

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
