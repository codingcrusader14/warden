#include "pmm.h"
#include "vmm.h"
#include "libk/includes/string.h"

static pa_t* head;

void pmm_init(pa_t* start, pa_t* end) {
  head = start;
  while (start != end) {
    pa_t* current_page = start; 
    pa_t* next_page = current_page + (PAGE_SIZE / sizeof(pa_t));
    if (next_page == end) {
      *current_page = 0;
    } else {
      *current_page = (pa_t)next_page;
    }
    start = next_page;
  }
}

pa_t* pmm_alloc() {
  if (head == NULL) {
    return NULL;
  } 
  pa_t next_page = *head;
  pa_t* current_page = head;
  head = (pa_t*)next_page;
  return current_page;
}

void pmm_free(pa_t* address) {
  if (address == NULL) {
    return;
  }
  memset(address, 0, PAGE_SIZE);
  *address = (pa_t)head;
  head = address;
}
