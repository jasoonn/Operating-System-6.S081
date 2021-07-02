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
  int mapCounts[PHYSTOP/PGSIZE+1];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  acquire(&kmem.lock);
  for (int i=0;i<PHYSTOP/PGSIZE+1;i++){
    kmem.mapCounts[i] = 1;
  }
  release(&kmem.lock);
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
void
kfree(void *pa)
{
  int is = 1;
  acquire(&kmem.lock);
  kmem.mapCounts[(uint64)pa/4096] -= 1;
  is = kmem.mapCounts[(uint64)pa/4096];
  release(&kmem.lock);
  // if (is<0) {
  //   printf("%p %d", pa, kmem.mapCounts[(uint64)pa/4096]);
  //   panic("kfreeee");
  // }
  if (is==0) {
    struct run *r;

    if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
      panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
}

void
addOne(uint64 pa)
{
  acquire(&kmem.lock);
  kmem.mapCounts[pa/PGSIZE] += 1;
  release(&kmem.lock);
}

int 
getCount(uint64 pa)
{
  int ans;
  acquire(&kmem.lock);
  ans = kmem.mapCounts[pa/PGSIZE];
  release(&kmem.lock);
  return ans;
}

int
minusOne(uint64 pa)
{
  acquire(&kmem.lock);
  if (kmem.mapCounts[pa/PGSIZE]==1){
    release(&kmem.lock);
    return 1;
  }
  kmem.mapCounts[pa/PGSIZE] -= 1;
  release(&kmem.lock);
  return 0;
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
    kmem.mapCounts[(uint64)r/PGSIZE] = 1;
  }
    
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
