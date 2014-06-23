#include "barec.h"
#include "parse.h"

int type(void *stmt) {
    return *(int *)stmt;
}

void assignmentGenCode(void *vptr, buffer *buff) {
    assignment *expr = (assignment *)vptr;
    buff_add(buff, get_addr(expr->expr1));
    buff_addln(buff, "mov    ebx, eax");
    buff_add(buff, get_value(expr->expr2));
    buff_addln(buff, "mov    [ebx], eax");
}

void expression_stmtGenCode(void *vptr, buffer *buff) {
    expression_stmt *stmt = (expression_stmt *)vptr;
    list *ptr;
    for (ptr = stmt->assignment_list->next; ptr; ptr = ptr->next)
        assignmentGenCode(ptr->content, buff);
}

char *integer_get_value(void *vptr) {
    integer *expr = (integer *)vptr;
    buffer *buff = buff_init();
    buff_add(buff, "mov    eax, ");
    buff_addln(buff, expr->value);
    return buff_puts(buff);
}

char *identifier_get_value(void *vptr) {
    declaration_node *node = (declaration_node *)vptr;
    buffer *buff = buff_init();
    if (type(node->specifier) == int_specifier_t) {
        int_specifier *specifier = (int_specifier *)node->specifier;
        if (type(specifier->storage) == auto_storage_t) {
            auto_storage *storage = (auto_storage *)specifier->storage;
            buff_add(buff, "mov    eax, ");
            buff_add(buff, "[ebp-");
            buff_add(buff, itoa(storage->stack_position));
            buff_addln(buff, "]");
            return buff_puts(buff);
        }
    }
}

char *identifier_get_addr(void *vptr) {
    declaration_node *node = (declaration_node *)vptr;
    buffer *buff = buff_init();
    if (type(node->specifier) == int_specifier_t) {
        int_specifier *specifier = (int_specifier *)node->specifier;
        if (type(specifier->storage) == auto_storage_t) {
            auto_storage *storage = (auto_storage *)specifier->storage;
            buff_add(buff, "mov    eax, ");
            buff_add(buff, "ebp-");
            buff_addln(buff, itoa(storage->stack_position));
            return buff_puts(buff);
        }
    }
}

char *array_ref_get_addr(void *vptr) {
    array_ref *expr = (array_ref *)vptr; 
    buffer *buff = buff_init();
    buff_add(buff, get_addr(expr->primary));
    buff_addln(buff, "mov    eax, ebx");
    buff_add(buff, get_value(expr->expr2));
    buff_addln(buff, "add    eax, ebx");
    return buff_puts(buff);
}

char *array_ref_get_value(void *vptr) {
    array_ref *expr = (array_ref *)vptr;
    buffer *buff = buff_init();
    buff_add(buff, array_ref_get_addr(vptr));
    buff_addln(buff, "mov    eax, [eax]");
    return buff_puts(buff);
}

char *expression_get_value(void *vptr) {
    expression *expr = (expression *)vptr;
    buffer *buff = buff_init();
    list *ptr;
    for (ptr = expr->assignment_list->next; ptr; ptr = ptr->next)
        buff_add(buff, get_value(ptr->content));
    return buff_puts(buff);
}

void identifierGenCode(void *vptr, buffer *buff) {
    buff_add(buff, identifier_get_value(vptr));
}

void integerGenCode(void *vptr, buffer *buff) {
    buff_add(buff, integer_get_value(vptr));
}

void array_refGenCode(void *vptr, buffer *buff) {
    buff_add(buff, array_ref_get_value(vptr));
}

void genCode(void *ptr, buffer *buff) {
    switch (type(ptr)) {
        case expression_stmt_t:
            expression_stmtGenCode(ptr, buff);
            break;
        case assignment_t:
            assignmentGenCode(ptr, buff);
            break;
        case integer_t:
            integerGenCode(ptr, buff);
            break;
        case identifier_t:
            identifierGenCode(ptr, buff);
            break;
        case array_ref_t:
            array_refGenCode(ptr, buff);
            break;
    }
}

char *get_addr(void *expr) {
    switch (type(expr)) {
        case identifier_t:
            return identifier_get_addr(expr);
        case array_ref_t:
            return array_ref_get_addr(expr);
    }
}

char *get_value(void *expr) {
    switch (type(expr)) {
        case identifier_t:
            return identifier_get_value(expr);
        case integer_t:
            return integer_get_value(expr);
        case array_ref_t:
            return array_ref_get_value(expr);
        case expression_t:
            return expression_get_value(expr);
    }
}