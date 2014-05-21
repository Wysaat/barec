#include <stdlib.h>
#include <string.h>
#include "scope.h"

scope *scope_init() {
    scope *retptr = (scope *)malloc(sizeof(scope));
    memset(retptr, 0, sizeof(scope));
}

val_list *val_list_node(char *name, int addr) {
    val_list *retptr = (val_list *)malloc(sizeof(val_list));
    memset(retptr, 0, sizeof(val_list));
    retptr->name = name;
    retptr->addr = addr;
    return retptr;
}

void val_list_append(val_list *head, char *name, int addr) {
    val_list *ptr = head;
    while (ptr->next)
        ptr = ptr->next;
    ptr->next = val_list_node(name, addr);
    ptr->next->index = ptr->index + 1;
}