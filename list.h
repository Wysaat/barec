#ifndef LIST_H
#define LIST_H

#include "barec.h"

typedef struct list list;

/* the list has an empty head */
struct list {
	void *content;
	list *prev;
	list *next;
};

list *list_node();
void list_append(list *, void *);

#endif /* LIST_H */