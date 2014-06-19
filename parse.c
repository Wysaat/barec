#include "barec.h"
#include "parse.h"

int type(void *stmt) {
    return *(int *)stmt;
}

void assignmentGenCode(void *vptr, buffer *buff) {
    assignment *expr = (assignment *)vptr;
    buff_add(buff, "mov    ");
    buff_add(buff, get_addr(expr->expr1));
    buff_add(buff, ", ");
    buff_addln(buff, get_value(expr->expr2));
}

void expression_stmtGenCode(void *vptr, buffer *buff) {
    expression_stmt *stmt = (expression_stmt *)vptr;
    list *ptr;
    for (ptr = stmt->assignment_list->next; ptr; ptr = ptr->next)
        assignmentGenCode(ptr->content, buff);
}

char *integer_get_value(void *vptr) {
    integer *expr = (integer *)vptr;
    return expr->value;
}

char *identifier_get_value(void *vptr) {
    declaration_node *node = (declaration_node *)vptr;
    buffer *buff = buff_init();
    if (type(node->specifier) == int_specifier_t) {
        int_specifier *specifier = (int_specifier *)node->specifier;
        if (specifier->storage == 1) {  // auto
            buff_add(buff, "[ebp-");
            buff_add(buff, itoa(stack_pointer));
            stack_pointer += specifier->size;
            buff_add(buff, "]");
            return buff_puts(buff);
        }
    }
}

char *identifier_get_addr(void *vptr) {
    declaration_node *node = (declaration_node *)vptr;
    buffer *buff = buff_init();
    if (type(node->specifier) == int_specifier_t) {
        int_specifier *specifier = (int_specifier *)node->specifier;
        if (specifier->storage == 1) {  // auto
            buff_add(buff, "ebp-");
            buff_add(buff, itoa(stack_pointer));
            stack_pointer += specifier->size;
            return buff_puts(buff);
        }
    }
}

void identifierGenCode(void *vptr, buffer *buff) {
    buff_add(buff, "mov    eax, ");
    buff_addln(buff, identifier_get_value(vptr));
}

void integerGenCode(void *vptr, buffer *buff) {
    buff_add(buff, "mov    eax, ");
    buff_addln(buff, integer_get_value(vptr));
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
    }
}

char *get_addr(void *expr) {
    switch (type(expr)) {
        case identifier_t:
            return identifier_get_addr(expr);
    }
}

char *get_value(void *expr) {
    switch (type(expr)) {
        case identifier_t:
            return identifier_get_value(expr);
            break;
        case integer_t:
            return integer_get_value(expr);
            break;
    }
}