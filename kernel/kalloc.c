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

struct {
  struct spinlock lock;
  struct run *freelist;

  int ref_counts[PHYSTOP / PGSIZE];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

// in kernel/kalloc.c, replace the existing freerange function
void 
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    // Manually set ref_count to 1 before freeing.
    acquire(&kmem.lock);
    kmem.ref_counts[(uint64)p / PGSIZE] = 1;
    release(&kmem.lock);
    kfree(p);
  }
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

  acquire(&kmem.lock);
  if(kmem.ref_counts[(uint64)pa / PGSIZE] < 1)
    panic("kfree ref count");
  
  kmem.ref_counts[(uint64)pa / PGSIZE] -= 1;
  int ref_count = kmem.ref_counts[(uint64)pa / PGSIZE];
  release(&kmem.lock);

  if(ref_count > 0) {
    return; // Still in use, do not free
  }
  
  // If ref_count is 0, proceed with original free logic
  memset(pa, 1, PGSIZE); // fill with junk
  r = (struct run*)pa;
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    if(kmem.ref_counts[(uint64)r/PGSIZE]!=0)
      panic("kalloc ref");
    kmem.ref_counts[(uint64)r/PGSIZE]=1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// at the end of kernel/kalloc.c
void
inc_ref(uint64 pa)
{
  acquire(&kmem.lock);
  if (pa >= PHYSTOP)
    panic("inc_ref");
  kmem.ref_counts[pa / PGSIZE]++;
  release(&kmem.lock);
}

int
get_ref(uint64 pa)
{
acquire(&kmem.lock);
if (pa >= PHYSTOP)
panic("get_ref");
int ref_count = kmem.ref_counts[pa / PGSIZE];
release(&kmem.lock);
return ref_count;
}