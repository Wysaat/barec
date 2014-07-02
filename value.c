#include "utils.h"
#include "list.h"
#include "parse.h"
#include "types.h"
#include "scan.h"

void *arithmetic_get_value(void *vptr, buffer *buff) {
    arithmetic *expr = (arithmetic *)vptr;
    if (expr->specifier->atype == int_t) {
        buff_add(buff, "mov    eax, ");
        buff_addln(buff, expr->value);
    }
    return expr->specifier;
}

int declaration_get_value(void *vptr, buffer *buff) {
    declaration *node = (declaration *)vptr;
    char *size;
    if (node->declarator->pointers > 0)
        size = "dword";
    else if (type(node->specifier) == int_specifier_t) {
        int_specifier *specifier = (int_specifier *)node->specifier;
        retval = specifier->size;
        switch (specifier->size) {
            case 1:
                size = "byte";
                break;
            case 2:
                size = "word";
                break;
            case 4:
                size = "dword";
                break;
        }
    }
    if (type(node->storage) == auto_storage_t) {
        buff_add(buff, "mov    eax, ");
        buff_add(buff, size);
        buff_add(buff, " [ebp-");
        buff_add(buff, itoa(((auto_storage *)node->storage)->stack_position));
        buff_addln(buff, "]");
    }
    return retval;
}

int indirection_get_value(void *vptr, buffer *buff) {
    indirection *expr = (indirection *)vptr;
    int size = get_value(expr->expr, buff);
    buff_addln(buff, "mov    eax, [eax]");
    indirection *ptr = expr;
    int pointers = 0;
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

int m_expr_get_value(void *vptr, buffer *buff) {
    m_expr *expr = (m_expr *)vptr;
    int size1 = get_value(expr->left);
    if (size1 <= 4)
        buff_addln("push   eax");
    int size2 = get_value(expr->right);
    if (size2 <= 4)
        buff_addln("mov    ebx, eax");
    if (size1 <= 4)
        buff_addln("pop    eax");
    if (!strcmp(expr->op, "*")) {
        int size = size1 > size2 ? size1 : size2;
        if (size == 1)
    }
}

int assignment_get_value(void *vptr, buffer *buff) {
    assignment *expr = (assignment *)vptr;
    int size = get_addr(expr->expr1, buff);
    buff_addln(buff, "push    eax");
    get_value(expr->expr2, buff);
    buff_addln(buff, "pop     ebx");
    if (size == 1)
        buff_addln(buff, "mov    [ebx], al");
    else if (size == 2)
        buff_addln(buff, "mov    [ebx], ax");
    else if (size == 4)
        buff_addln(buff, "mov    [ebx], eax");
    return size;
}

int expression_get_value(void *vptr, buffer *buff) {
    expression *expr = (expression *)vptr;
    list *ptr;
    int size;
    for (ptr = expr->assignment_list->next; ptr; ptr = ptr->next)
        size = get_value(ptr->content, buff);
    return size;
}

int get_value(void *expr, buffer *buff) {
    switch (type(expr)) {
        case integer_t:
            return integer_get_value(expr, buff);
        case declaration_t:
            return declaration_get_value(expr, buff);
        case indirection_t:
            return indirection_get_value(expr, buff);
        case assignment_t:
            return assignment_get_value(expr, buff);
        case expression_t:
            return expression_get_value(expr, buff);
    }
}
