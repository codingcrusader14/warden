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

void pmm_free(pa_t* address) {
  if (address == NULL) {
    return;
  }
  *(pa_t*) PA_TO_KVA(address) = (pa_t)head;
  head = address;
}
