#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"
#include "sleeplock.h"
#include "fcntl.h"
#include "file.h"

uint64 sys_mmap(void) {
  uint64 addr;
  int length, prot, flags, fd, offset;

  // 1. 获取用户传入的6个参数
  if (argaddr(0, &addr) < 0 || argint(1, &length) < 0 || argint(2, &prot) < 0 ||
      argint(3, &flags) < 0 || argint(4, &fd) < 0 || argint(5, &offset) < 0) {
    return -1;
  }

  // 2. 获取当前进程和文件
  struct proc *p = myproc();
  struct file *f = p->ofile[fd];

  // 3. 参数校验
  // 文件必须存在。
  // 如果请求了读权限(PROT_READ)，文件必须是可读的。
  // 如果请求了写权限(PROT_WRITE) 并且是共享映射(MAP_SHARED)，文件必须是可写的。
  // (对于私有映射 MAP_PRIVATE，写入时不会影响原文件，所以文件不需要可写)
  if (f == 0 || (f->readable == 0 && (prot & PROT_READ)) ||
      (f->writable == 0 && (prot & PROT_WRITE) && !(flags & MAP_PRIVATE))) {
    return -1;
  }

  // 4. 寻找一个空闲的 vma 结构
  struct vma *v = 0;
  for (int i = 0; i < NVMA; i++) {
    if (p->vma[i].used == 0) {
      v = &p->vma[i];
      break;
    }
  }
  if (v == 0) {
    return -1; // 没有可用的 vma
  }

  // 5. 确定映射的起始地址
  // 如果用户传入的 addr 是 0，则由内核来选择地址。
  // 一个简单的策略是将其放在进程现有内存的末尾。
  if (addr == 0) {
    addr = p->sz;
    p->sz += length; // 扩展进程的逻辑大小，"预留"这块地址空间
  }
  
  // 6. 填充 vma 结构体
  v->used = 1;
  v->addr = addr;
  v->length = length;
  v->prot = prot;
  v->flags = flags;
  v->file = filedup(f); // 增加文件的引用计数，非常重要！
  v->offset = offset;

  // 7. 返回映射的起始地址
  return addr;
}
uint64 sys_munmap(void) {
  uint64 addr;
  int length;

  // 1. 获取参数
  if (argaddr(0, &addr) < 0 || argint(1, &length) < 0)
    return -1;

  // xv6中，地址和长度都要求是页大小的整数倍
  if (addr % PGSIZE != 0 || length % PGSIZE != 0)
    return -1;

  struct proc *p = myproc();
  struct vma *vma = 0;

  // 2. 找到地址 addr 所在的 vma
  for (int i = 0; i < NVMA; i++) {
    if (p->vma[i].used && addr >= p->vma[i].addr && addr < p->vma[i].addr + p->vma[i].length) {
      vma = &p->vma[i];
      break;
    }
  }
  if (vma == 0) {
    return -1; // 没有找到对应的 vma
  }
  
  // 3. 【重要】如果是共享、可写的映射，将数据写回文件
  if ((vma->flags & MAP_SHARED) && (vma->prot & PROT_WRITE)) {
    // 注意：这里的实现是简化版，它直接调用了 filewrite。
    // 在一个更完整的系统中，这里应该只写回被修改过的“脏页”。
    filewrite(vma->file, addr, length);
  }

  // 4. 解除页表映射
  uvmunmap(p->pagetable, addr, length / PGSIZE, 1);

  // 5. 更新 vma 结构
  // 情况一：完全 unmap 整个区域
  if (addr == vma->addr && length == vma->length) {
    fileclose(vma->file); // 减少文件引用计数
    vma->used = 0;
  // 情况二：从区域的开头 unmap 一部分
  } else if (addr == vma->addr) {
    vma->addr += length;
    vma->length -= length;
    vma->offset += length;
  // 情况三：从区域的末尾 unmap 一部分
  } else if (addr + length == vma->addr + vma->length) {
    vma->length -= length;
  // 其他情况（例如从中间unmap）本实验不支持
  } else {
    return -1;
  }
  
  return 0; // 成功
}