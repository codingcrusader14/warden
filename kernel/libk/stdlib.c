#include "includes/stdlib.h"
#include "../../kernel/mmu_defs.h"
#include "../../kernel/pmm.h"
#include <stdint.h>

static block_header_t *head;

void *kmalloc(size_t size) {
  if (head == NULL) { // no free pages
    pa_t *page = pmm_alloc();
    if (!page) { // page allocation failed, or no physical pages left
      return NULL;
    }
    block_header_t *header = (block_header_t *)PA_TO_KVA(page);
    header->size = (PAGE_SIZE - sizeof(block_header_t));
    header->free = true;
    header->next = NULL;
    head = header;
  }

  block_header_t *prev = NULL;
  block_header_t *current = head;
  void *allocated_chunk = NULL;
  uint32 total_size = size + sizeof(block_header_t);

  while (current) {
    if (size <= current->size) { // found suitable block, first fit algorithm
      block_header_t *memory = current;
      uint32 remainder = current->size - size;
      memory->size = size;
      memory->free = false;

      if (remainder <
          sizeof(
              block_header_t)) { // no suitable next block, use all memory chunk
        if (current == head) {
          head = current->next;
        } else {
          prev->next = current->next;
        }
        allocated_chunk = (void *)((uint8 *)memory + sizeof(block_header_t));
        memory->size = size + remainder;
        return allocated_chunk;
      }

      block_header_t *next_free_block =
          (block_header_t *)((uint8 *)current + total_size);
      next_free_block->size = remainder - sizeof(block_header_t);
      next_free_block->free = true;
      next_free_block->next = current->next;
      memory->next = NULL;
      if (current == head) { // if current page at head, split page takes place
        head = next_free_block;
      } else {
        prev->next = next_free_block;
      }

      allocated_chunk = (void *)((uint8 *)memory + sizeof(block_header_t));
      return allocated_chunk;
    }
    prev = current;
    current = current->next;
  }

  uint32 needed = size + sizeof(block_header_t);
  uint32 num_pages = (needed + PAGE_SIZE - 1) / PAGE_SIZE;
  pa_t* pages = pmm_alloc_pages(num_pages);
  if (!pages) return NULL;

  block_header_t *new_header = (block_header_t *)PA_TO_KVA(pages);
  new_header->size = ((num_pages * PAGE_SIZE) - sizeof(block_header_t));
  new_header->free = true;
  new_header->next = head;
  head = new_header;
  return kmalloc(size);
}

void kfree(void *ptr) {
  if (!ptr) {
    return;
  }

  block_header_t *memory_block = (block_header_t *)((uint8 *)ptr - sizeof(block_header_t));
  memory_block->free = true;
  bool inserted = false;

  if (!head || memory_block < head) {
    memory_block->next = head;
    head = memory_block;
    inserted = true; // dont iterate since memory was already added to head
  }

  block_header_t *current = head;
  while (current && !inserted) {
    if (memory_block > current &&  (current->next == NULL || memory_block < current->next)) { // check if memory is greater than current and less than current->next
      block_header_t *temp = current->next;
      current->next = memory_block;
      memory_block->next = temp;
      break;
    }
    current = current->next;
  }

  block_header_t *forward_block = (block_header_t *)((uint8 *)memory_block + sizeof(block_header_t) + memory_block->size);
  while (memory_block->next) { // forward coalescing
    if (forward_block != memory_block->next) {
      break;
    }

    uint32 new_size = memory_block->size + sizeof(block_header_t) + memory_block->next->size;
    memory_block->next = memory_block->next->next;
    memory_block->size = new_size;
    forward_block = (block_header_t *)((uint8 *)memory_block + sizeof(block_header_t) + memory_block->size);
  }

  if (inserted) return; // if block is head no need to backward coalesce return

  block_header_t *backward_block = (block_header_t *)((uint8 *)current + (sizeof(block_header_t) + current->size));

  if (backward_block == memory_block) { // merge backward
    uint32 new_size = current->size + sizeof(block_header_t) + memory_block->size;
    current->size = new_size;
    current->next = memory_block->next;
  }
}
