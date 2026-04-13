#include "pmm.h"
#include "mmu_defs.h"
#include "libk/includes/string.h"
#include "libk/includes/stdio.h"

static pa_t* head;

void print_head() {
  kprintf("Head address is %p\n", head);
}
void pmm_init(pa_t* start, pa_t* end) {
  head = start;
  while (start != end) {
    pa_t* current_page = start; 
    pa_t* next_page = current_page + (PAGE_SIZE / sizeof(pa_t));
    if (next_page == end) {
      *(pa_t*) PA_TO_KVA(current_page) = 0;
    } else {
      *(pa_t*) PA_TO_KVA(current_page) = (pa_t)next_page;
    }
    start = next_page;
  }
}

pa_t* pmm_alloc() {
  if (head == NULL) {
    return NULL;
  } 
  pa_t next_page = *(pa_t*) PA_TO_KVA(head);
  pa_t* current_page = head;
  head = (pa_t*)next_page;
  memset((void*) PA_TO_KVA(current_page), 0, PAGE_SIZE);
  return current_page;
}

pa_t* pmm_alloc_pages(size_t count) {
  if (count <= 0 || !head) return NULL;
  if (count == 1) pmm_alloc();

  pa_t* prev = NULL;
  pa_t* start = head;
  pa_t* current = start;
  uint32 found = 1;

  while (found < count) {
      pa_t* next = (pa_t*)(*(pa_t*)PA_TO_KVA(current));
      if (!next) return NULL;

      if ((pa_t)next == (pa_t)current + PAGE_SIZE) {
          found++;
          current = next;
      } else {
          prev = current;
          start = next;
          current = next;
          found = 1;
      }
  }

  pa_t* after = (pa_t*)(*(pa_t*)PA_TO_KVA(current));
  if (prev) {
      *(pa_t*)PA_TO_KVA(prev) = (pa_t)after;
  } else {
      head = after;
  }

  // zero all pages
  for (uint32 i = 0; i < count; i++) {
      memset((void*)PA_TO_KVA((pa_t*)((pa_t)start + (i * PAGE_SIZE))), 0, PAGE_SIZE);
  }
  return start;
}

void pmm_free(pa_t* address) {
  if (address == NULL) {
    return;
  }
  *(pa_t*) PA_TO_KVA(address) = (pa_t)head;
  head = address;
}
