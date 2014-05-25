#include <stdlib.h>
#include <string.h>
#include "scope.h"
#include "types.h"

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

int find_addr(scope *local_scope, char *name) {
    val_list *ptr;
    for (ptr = local_scope->vals; ptr; ptr = ptr->next) {
        if (!strcmp(name, ptr->name))
            return ptr->addr;
    }
    puts("f**k");
}

void declare(scope *local_scope, char *token) {
    int addr = local_scope->next_addr;
    int type = local_scope->decl_type;
    if (type == int_t) {
        local_scope->next_addr += 4;
        if (local_scope->vals)
            val_list_append(local_scope->vals, token, addr);
        else
            local_scope->vals = val_list_node(token, addr);
    }
}