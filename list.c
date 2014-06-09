#include "barec.h"

list *list_node() {
    list *retptr = (list *)malloc(sizeof(list));
    memset(retptr, 0, sizeof(list));
    return retptr;
}

void list_append(list *head, void *content) {
    list *node = list_node();
    node->content = content;
    if (!head->next)
        head->next = node;
    else {
        list *ptr = head->next;
        while (ptr->next)
            ptr = ptr->next;
        ptr->next = node;
    }
}