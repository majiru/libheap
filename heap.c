#include <u.h>
#include <libc.h>
#include <heap.h>

void
swap(HeapNode **a, HeapNode **b)
{
	HeapNode *tmp = *a;
	*a = *b;
	*b = tmp;
}

void
splice_heap_node_lists(HeapNode *n1, HeapNode *n2)
{
	if (n1 && n2) {
		n1->next->prev = n2->prev;
		n2->prev->next = n1->next;
		n1->next = n2;
		n2->prev = n1;
	}
}

void
insert_heap_node_in_list(HeapNode *n, HeapNode *l)
{
	n->next = l;
	n->prev = l->prev;
	n->prev->next = n;
	l->prev = n;
}

void
remove_heap_node_from_list(HeapNode *n)
{
	n->next->prev = n->prev;
	n->prev->next = n->next;
}

void
print_heap_node(HeapNode *n, unsigned indent, char *(*print)(const void *v))
{
	HeapNode *nc;

	//print("%c%s%s\n", indent, "", print(n->datum));
	if (!(nc = n->child)) {
		return;
	}

	do {
		print_heap_node(nc, indent + 2, print);
		nc = nc->next;
	} while (nc != n->child);
}

void
print_heap(Heap *h, char *(*print)(const void *v))
{
	HeapNode *n;

	if (h->min) {
		//print("size: %d\n", h->size);
		print("min = ");
		n = h->min;
		do {
			print_heap_node(n, 0, print);
			n = n->next;
		} while (n != h->min);
	} else {
		print("(null)\n");
	}
}

void
print_heap_node_list(HeapNode *n)
{
	HeapNode *hn;

	if (!n) {
		return;
	}

	hn = n;
	do {
		print("%p ", hn->datum);
		hn = hn->next;
	} while (hn != n);
	print("\n");
}

void
heap_init(Heap *h, int (*compare)(const void *key, const void *with), void (*datum_delete)(void *))
{
	h->min = nil;
	h->size = 0;
	h->compare = compare;
	h->datum_delete = datum_delete;
}

void
heap_node_delete(Heap *h, HeapNode *hn)
{
	HeapNode *next;

	hn->prev->next = nil;
	while (hn) {
		if (hn->child) {
			heap_node_delete(h, hn->child);
		} 
		next = hn->next;
		if (h->datum_delete) {
			h->datum_delete(hn->datum);
		}
		free(hn);
		hn = next;
	}
}

void
heap_delete(Heap *h)
{
	if (h->min) {
		heap_node_delete(h, h->min);
	}
	h->min = nil;
	h->size = 0;
	h->compare = nil;
	h->datum_delete = nil;
}

HeapNode*
heap_insert(Heap *h, void *v)
{
	HeapNode *n;

	assert((n = calloc(1, sizeof (*n))));
	n->datum = v;

	if (h->min) {
		insert_heap_node_in_list(n, h->min);
	} else {
		n->next = n->prev = n;
	}
	if (!h->min || (h->compare(v, h->min->datum) < 0)) {
		h->min = n;
	}
	h->size++;

	return n;
}

void*
heap_peek_min(Heap *h)
{
	return h->min ? h->min->datum : nil;
}

static void
heap_link(Heap *h, HeapNode *node, HeapNode *root)
{
	/*  remove_heap_node_from_list(node);*/
	if (root->child) {
		insert_heap_node_in_list(node, root->child);
	} else {
		root->child = node;
		node->next = node->prev = node;
	}
	node->parent = root;
	root->degree++;
	node->mark = 0;
}

static void
heap_consolidate(Heap *h)
{
	uint i;
	HeapNode *x, *y, *n;
	HeapNode *a[64]; /* Need ceil(lg(h->size)), so this is good  *
			                 * to the limit of a 64-bit address space,  *
			                 * and much faster than any lg calculation. */

	memset(a, 0, sizeof (a));

	h->min->prev->next = nil;

	for (x = n = h->min; n; x = n) {
		n = n->next;

		while (a[x->degree]) {
			y = a[x->degree];
			if (h->compare(x->datum, y->datum) > 0) {
			  swap(&x, &y);
			}
			a[x->degree] = nil;
			heap_link(h, y, x);
		}
		a[x->degree] = x;
	}

	for (h->min = nil, i = 0; i < 64; i++) {
		if (a[i]) {
			if (h->min) {
			  insert_heap_node_in_list(a[i], h->min);
			  if (h->compare(a[i]->datum, h->min->datum) < 0) {
			    h->min = a[i];
			  }
			} else {
			  h->min = a[i];
			  a[i]->next = a[i]->prev = a[i];
			}
		}
	}
}

void*
heap_remove_min(Heap *h)
{
	void *v;
	HeapNode *n;

	v = nil;

	if (h->min) {
		v = h->min->datum;
		if (h->size == 1) {
			free(h->min);
			h->min = nil;
		} else {
			if ((n = h->min->child)) {
				for (; n->parent; n = n->next) {
					n->parent = nil;
				}
			}

			splice_heap_node_lists(h->min, h->min->child);

			n = h->min;
			remove_heap_node_from_list(n);
			h->min = n->next;
			free(n);

			heap_consolidate(h);
		}

		h->size--;
	}

	return v;
}

int
heap_combine(Heap *h, Heap *h1, Heap *h2)
{
	if (h1->compare != h2->compare ||
			h1->datum_delete != h2->datum_delete) {
		return 1;
	}

	h->compare = h1->compare;
	h->datum_delete = h1->datum_delete;

	if (!h1->min) {
		h->min = h2->min;
		h->size = h2->size;
	} else if (!h2->min) {
		h->min = h1->min;
		h->size = h1->size;
	} else {
		h->min = ((h->compare(h1->min->datum, h2->min->datum) < 0) ? h1->min : h2->min);
		splice_heap_node_lists(h1->min, h2->min);
	}

	memset(h1, 0, sizeof (*h1));
	memset(h2, 0, sizeof (*h2));

	return 0;
}

static void
heap_cut(Heap *h, HeapNode *n, HeapNode *p)
{
	if (!--p->degree) {
		p->child = nil;
	}
	if (p->child == n) {
		p->child = p->child->next;
	}
	remove_heap_node_from_list(n);
	n->parent = nil;
	n->mark = 0;
	insert_heap_node_in_list(n, h->min);
}

static void
heap_cascading_cut(Heap *h, HeapNode *n)
{
	HeapNode *p;

	if ((p = n->parent)) {
		if (!n->mark) {
			n->mark = 1;
		} else {
			heap_cut(h, n, p);
			heap_cascading_cut(h, n);
		}
	}
}

int
heap_decrease_key(Heap *h, HeapNode *n, void *v)
{
	if (h->compare(n->datum, v) <= 0) {
		return 1;
	}

	if (h->datum_delete) {
		h->datum_delete(n->datum);
	}
	n->datum = v;

	return heap_decrease_key_no_replace(h, n);
}

int
heap_decrease_key_no_replace(Heap *h, HeapNode *n)
{
	/* No tests that the value hasn't actually increased.  Change *
	 * occurs in place, so the check is not possible here.  The   *
	 * user is completely responsible for ensuring that they      *
	 * don't fubar the queue.                                     */

	HeapNode *p;

	p = n->parent;

	if (p && (h->compare(n->datum, p->datum) < 0)) {
		heap_cut(h, n, p);
		heap_cascading_cut(h, p);
	}
	if (h->compare(n->datum, h->min->datum) < 0) {
		h->min = n;
	}

	return 0;
}
