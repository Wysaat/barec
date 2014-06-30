#include "utils.h"
#include "list.h"
#include "parse.h"
#include "types.h"
#include "scan.h"

int declaration_get_addr(void *vptr, buffer *buff) {
    declaration *node = (declaration *)vptr;
    if (type(node->storage) == auto_storage_t) {
        buff_add(buff, "mov    eax, ");
        buff_add(buff, "ebp-");
        buff_addln(buff, itoa(((auto_storage *)node->storage)->stack_position));
    }
    if (node->declarator->pointers > 0)
        return 4;
    else if (type(node->specifier) == int_specifier_t)
        return ((int_specifier *)node->specifier)->size;
}

int indirection_get_addr(void *vptr, buffer *buff) {
    indirection *expr = (indirection *)vptr;
    get_value(expr->expr, buff);
    while (type(ptr) == indirection_t) {
        ptr = ptr->expr;
        pointers++;
    }
    if (type(ptr) == declaration_t) {
        declaration *node = (declaration *)ptr;
        if (node->pointers > pointers)
            return 4;
        else if (type(node->specifier) == int_specifier_t)
            return ((int_specifier *)node->specifier)->size;
    }
}

int get_addr(void *expr, buffer *buff) {
    switch (type(expr)) {
        case declaration_t:
            return declaration_get_addr(expr, buff);
        case indirection_t:
            return indirection_get_addr(expr, buff);
    }
}
