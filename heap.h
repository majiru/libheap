/*
* Code by: Dr. Jeremy Sheaffer
* Port: Jacob Moody
*/

#ifndef _HEAP_H_
#define _HEAP_H_ 1
#if defined(__cplusplus)
extern "C" {
#endif

AUTOLIB(heap)
#pragma lib "libheap.a"
typedef struct HeapNode HeapNode;

struct HeapNode {
  HeapNode *next;
  HeapNode *prev;
  HeapNode *parent;
  HeapNode *child;
  void *datum;
  u32int degree;
  u32int mark;
};

typedef
struct Heap {
  HeapNode *min;
  u32int size;
  int (*compare)(const void *key, const void *with);
  void (*datum_delete)(void *);
} Heap;

void heap_init(Heap *h, int (*compare)(const void *key, const void *with), void (*datum_delete)(void *));
void heap_delete(Heap *h);
HeapNode *heap_insert(Heap *h, void *v);
void *heap_peek_min(Heap *h);
void *heap_remove_min(Heap *h);
int heap_combine(Heap *h, Heap *h1, Heap *h2);
int heap_decrease_key(Heap *h, HeapNode *n, void *v);
int heap_decrease_key_no_replace(Heap *h, HeapNode *n);

#if defined(__cplusplus)
}
#endif
#endif
