#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  backtrace();

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// in kernel/sysproc.c, at the end
uint64
sys_sigalarm(void)
{
  int interval;
  uint64 handler_addr;
  struct proc *p = myproc();

  if (argint(0, &interval) < 0 || argaddr(1, &handler_addr) < 0) {
    return -1;
  }

  p->alarm_interval = interval;
  p->alarm_handler = (void (*)())handler_addr;
  p->ticks_left = interval; // Initialize the countdown
  
  return 0;
}

// in kernel/sysproc.c, at the end
uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();

  // Restore the trapframe from the backup.
  *(p->trapframe) = *(p->saved_trapframe);

  // Reset the alarm countdown.
  p->ticks_left = p->alarm_interval;

  // Mark the alarm as no longer active.
  p->alarm_active = 0;

  return 0;
}