#include "barec.h"

declaration_specifier *declaration_specifier_init(int storage_class, int type_qualifier, void *type_specifier) {
    declaration_specifier *retptr = (declaration_specifier *)malloc(sizeof(declaration_specifier));
    retptr->storage_class = storage_class;
    retptr->type_qualifier = type_qualifier;
    retptr->type_specifier = type_specifier;
    return retptr;
}

integral_type_specifier *integral_type_specifier_init() {
    integral_type_specifier *retptr = (integral_type_specifier *)malloc(sizeof(integral_type_specifier));
    memset(retptr, 0, sizeof(integral_type_specifier));
    // retptr->type = integral_type_specifier_t;
    return retptr;
}

declaration *declaration_init(declaration_specifier *dec_spec, list *init_declarator_list) {
    declaration *retptr = (declaration *)malloc(sizeof(declaration));
    // retptr->type = declaration_t;
    retptr->dec_spec = dec_spec;
    retptr->init_declarator_list = init_declarator_list;
    return retptr;
}

void declaration_gencode(declaration *declaration, buffer *buff) {
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

void int_expr_gencode(void *expression, buffer *buff) {
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