#ifndef LIST_H
#define LIST_H

#include "barec.h"

typedef struct list list;
typedef struct buffer buffer;

/* the list has an empty head */
struct list {
	void *content;
	list *prev;
	list *next;
};

typedef struct buffer {
    char *content;
    buffer *prev;
    buffer *next;
} buffer;

list *list_node();
void list_append(list *, void *);
buffer *buff_init();
void buff_add(buffer *buff, char *string);
void buff_addln(buffer *buff, char *string);
char *buff_puts(buffer *buff);

#endif /* LIST_H */