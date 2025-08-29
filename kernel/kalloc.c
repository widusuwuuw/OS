// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

// in kernel/kalloc.c
// structure for each CPU's memory pool
struct kmem_cpu {
  struct spinlock lock;
  struct run *freelist;
};

// array of per-CPU memory pools
struct {
  struct kmem_cpu cpu[NCPU];
} kmem;

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    initlock(&kmem.cpu[i].lock, "kmem");
  }
}

void
kmeminit()
{
  kinit();
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
// in kernel/kalloc.c
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // Safely get the current CPU's ID
  push_off();
  int cid = cpuid();
  pop_off();
  
  // Acquire the lock for this CPU's freelist and add the page.
  acquire(&kmem.cpu[cid].lock);
  r->next = kmem.cpu[cid].freelist;
  kmem.cpu[cid].freelist = r;
  release(&kmem.cpu[cid].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
// in kernel/kalloc.c
void *
kalloc(void)
{
  struct run *r;

  // Get current CPU ID safely
  push_off();
  int cid = cpuid();
  pop_off();

  // --- Fast path: try to allocate from current CPU's freelist ---
  acquire(&kmem.cpu[cid].lock);
  r = kmem.cpu[cid].freelist;
  if(r){
    kmem.cpu[cid].freelist = r->next;
  }
  release(&kmem.cpu[cid].lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    return (void*)r;
  }

  // --- Slow path: local freelist is empty, try to steal from others ---
  for(int i = 0; i < NCPU; i++){
    if(i == cid) {
      continue; // Skip our own CPU
    }
    
    // Try to acquire the lock of another CPU's freelist
    acquire(&kmem.cpu[i].lock);
    r = kmem.cpu[i].freelist;
    if(r){
      // Found a page to steal!
      kmem.cpu[i].freelist = r->next;
    }
    release(&kmem.cpu[i].lock);

    if(r){
      memset((char*)r, 5, PGSIZE); // fill with junk
      return (void*)r; // Stole a page, return it.
    }
  }
  
  return 0; // Out of memory
}
