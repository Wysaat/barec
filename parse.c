#include "barec.h"

void declarationGencode(declaration *declaration, buffer *buff) {
    list_append(scope->declaration_list, declaration);
    list *ptr;
    for (ptr = declaration->init_declarator_list; ptr; ptr = ptr->next) {
        if (!ptr->content->initializer) {
            if (declaration->dec_spec->storage_class == 1) {  // "auto"
                int *addrp = (int *)malloc(sizeof(int));
                *addrp = scope->stack_addr;
                if (type(declaration->dec_spec->type_specifier) == integral_type_specifier_t)
                    scope->stack_addr += ((integral_type_specifier *)declaration->dec_spec->type_specifier)->size;
                ptr->content->initializer = addrp;
            }
        }
    }
}

void int_exprGencode(void *expression, buffer *buff) {
    int_expr *expr = (int_expr *)expression;
    buff_add(buff, "mov    eax, ");
    buff_addln(buff, expr->value);
}

void identifier_gencode(void *expression, buffer *buff) {
    identifier *expr = (identifier *)expression;
    list *ptr;
    void *addrp = 0;
    for (ptr = scope->declaration_list; ptr; ptr = ptr->next) {
        list *ptr2;
        for (ptr2 = ptr->content->init_declarator_list; ptr2; ptr2 = ptr2->next) {
            if (!strcmp(ptr2->content->declarator->id, expr->value)) {
                addrp = ptr2->content->initializer;
                break;
            }
        }
        if (addrp) {
            if (ptr->dec_spec->storage_class == 1)
        }
    }
}

void expression_gencode(void *expression, buffer *buff) {
    if (type(expression) == int_expr_t)
        int_expr_gencode(expression);
    else if (type(expression) == identifier_t)
        identifier_gencode(expression);
}

void stmt_gencode(void *stmt, buffer *buff) {
    if (type(stmt) == expression_t)
        expression_gencode(stmt);
}