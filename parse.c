#include <stdlib.h>
#include "barec.h"

int type(void *stmt) {
    return *(int *)stmt;
}

auto_storage *auto_storage_init() {
    auto_storage *retptr = malloc(sizeof(auto_storage));
    retptr->type = auto_storage_t;
    retptr->address = 0;
    return retptr;
}

static_storage *static_storage_init() {
    static_storage *retptr = malloc(sizeof(static_storage));
    retptr->type = static_storage_t;
    retptr->address = 0;
    return retptr;
}

arithmetic_specifier *arithmetic_specifier_init(int atype) {
    arithmetic_specifier *retptr = (arithmetic_specifier *)malloc(sizeof(arithmetic_specifier));
    retptr->type = arithmetic_specifier_t;
    retptr->atype = atype;
    return retptr;
}

struct_specifier *struct_specifier_init(char *id, list *declaration_list) {
    struct_specifier *retptr = (struct_specifier *)malloc(sizeof(struct_specifier));
    retptr->type = struct_specifier_t;
    retptr->id = id;
    retptr->declaration_list = declaration_list;
    return retptr;
}

pointer *pointer_init() {
    pointer *retptr = (pointer *)malloc(sizeof(pointer));
    retptr->type = pointer_t;
    return retptr;
}

array *array_init(void *size) {
    array *retptr = (array *)malloc(sizeof(array));
    retptr->type = array_t,
    retptr->size = size;
    return retptr;
}

declarator *declartor_init(char *id, list *type_list) {
    declarator *retptr = (declarator *)malloc(sizeof(declarator));
    retptr->type = declarator_t;
    retptr->id = id;
    retptr->type_list = type_list;
    return retptr;
}

declaration *declaration_init(char *id, void *storage, list *type_list) {
    declaration *retptr = (declaration *)malloc(sizeof(declaration));
    retptr->type = declaration_t;
    retptr->id = id;
    retptr->storage = storage;
    retptr->type_list = type_list;
    return retptr;
}

arithmetic *ARITHMETIC(char *value, int atype) {
    arithmetic *retptr = (arithmetic *)malloc(sizeof(arithmetic));
    retptr->type = arithmetic_t;
    retptr->value = value;
    retptr->specifier = arithmetic_specifier_init(atype);
    return retptr;
}

string *STRING(int address, char *value) {
    string *retptr = (string *)malloc(sizeof(string));
    retptr->type = string_t;
    retptr->data_position = data_position;
    retptr->value = value;
    return retptr;
}

array_ref *ARRAY_REF(void *primary, void *expr) {
    array_ref *retptr = (array_ref *)malloc(sizeof(array_ref));
    retptr->type = array_ref_t;
    retptr->primary = primary;
    retptr->expr = expr;
    retptr->type_list = get_type_list(primary)->next;
    return retptr;
}

struct_ref *STRUCT_REF(void *primary, char *id) {
    struct_ref *retptr = (struct_ref *)malloc(sizeof(struct_ref));
    retptr->type = struct_ref_t;
    retptr->primary = primary;
    retptr->id = id;

    list *type_list = get_type_list(primary);
    struct_specifier *specifier = (specifier *)type_list->content;
    for (ptr = specifier->declaration_list->next; ptr; ptr = ptr->next) {
        if (!strcmp(ptr->content->id, expr->id)) {
            retptr->type_list = ptr->content->type_list;
            break;
        }
    }

    return retptr;
}

posinc *POSINC(void *primary, int inc) {
    posinc *retptr = (posinc *)malloc(sizeof(posinc));
    retptr->type = posinc_t;
    retptr->primary = primary;
    retptr->inc = inc;
    retptr->type_list = get_type_list(primary);
    return retptr;
}

preinc *PREINC(void *expr, int inc) {
    preinc *retptr = (preinc *)malloc(sizeof(preinc));
    retptr->type = preinc_t,
    retptr->expr = expr;
    retptr->inc = inc;
    retptr->type_list = get_type_list(expr);
    return retptr;
}

addr *ADDR(void *expr) {
    addr *retptr = (addr *)malloc(sizeof(addr));
    retptr->type = addr_t;
    retptr->expr = expr;
    retptr->type_list = list_node();
    retptr->type_list->content = pointer_init();
    retptr->type_list->next = get_type_list(expr);
    return retptr;
}

indirection *INDIRECTION(void *expr) {
    indirection *retptr = (indirection *)malloc(sizeof(indirection));
    retptr->type = indirection_t;
    retptr->expr = expr;
    retptr->type_list = get_type_list(expr)->next;
    return retptr;
}

unary *UNARY(void *expr, char *op) {
    unary *retptr = (unary *)malloc(sizeof(unary));
    retptr->type = unary_t;
    retptr->expr = expr;
    retptr->op = op;
    retptr->type_list = get_type_list(expr);
    return retptr;
}

size *SIZE(void *expr) {
    size *retptr = (size *)malloc(sizeof(size));
    retptr->type = sizeof_t;
    retptr->expr = expr;
    retptr->type_list = list_node();
    retptr->type_list->content = arithmetic_specifier_init(int_t);
    return retptr;
}

cast *CAST(list *type_list, cast *expr)
{
    cast *retptr = (cast *)malloc(sizeof(cast));
    retptr->type = cast_t;
    retptr->expr = expr;
    return retptr;
}

m_expr *M_EXPR(char *op, void *expr1, void *expr2)
{
    m_expr *retptr = (m_expr *)malloc(sizeof(m_expr));
    retptr->op = op;
    retptr->left = expr1;
    retptr->right = expr2;
    retptr->type_list = list_node();
    arithmetic_specifier *lspecifier = get_type_list(expr1)->content;
    arithmetic_specifier *rspecifier = get_type_list(expr2)->content;
    arithmetic_specifier *specifier  = arithmetic_convertion(lspecifier, rspecifier);
    retptr->type_list->content = specifier;
    if (lspecifier != specifier)
        retptr->left = CAST(retptr->type_list, expr1);
    if (rspecifier != specifier)
        retptr->right = CAST(retptr->type_list, expr2);
    if (ltype->a)
    return retptr;
}

a_expr *A_EXPR(char *op, void *expr1, void *expr2)
{
    a_expr *retptr = (a_expr *)malloc(sizeof(a_expr));
    retptr->op = op;
    retptr->type_list = list_node();
    retptr->left = expr1;
    retptr->right = expr2;
    retptr->type_list = list_node();
    list *ltype_list = get_type_list(expr1);
    list *rtype_list = get_type_list(expr2);
    if (type(ltype_list->content) == pointer_t || type(ltype_list->content) == array_t) {
        return A_EXPR("+", expr1, M_EXPR("*", SIZE(INDIRECTION(expr1)), expr2));
    }
    if (type(rtype_list->content) == pointer_t || type(rtype_list->content) == array_t) {
        return A_EXPR("+", expr2, M_EXPR("*", SIZE(INDIRECTION(expr2)), expr1));
    }
    arithmetic_specifier *lspecifier = get_type_list(expr1)->content;
    arithmetic_specifier *rspecifier = get_type_list(expr2)->content;
    arithmetic_specifier *specifier  = arithmetic_convertion(lspecifier, rspecifier);
    retptr->type_list->content = specifier;
    if (lspecifier != specifier)
        retptr->left = CAST(retptr->type_list, expr1);
    if (rspecifier != specifier)
        retptr->right = CAST(retptr->type_list, expr2);
    return retptr;
}

arithmetic_specifier *arithmetic_convertion(arithmetic_specifier *ls, arithmetic_specifier *rs)
{
    enum atypes left = ls->atype, right = rs->atype;
    if (left->atype == long_double_t || right == long_double_t)
        res = long_double_t;
    else if (left == double_t || right == double_t)
        res = double_t;
    else if (left == float_t || right == float_t)
        res = float_t;
    else if (left == unsigned_long_long_t || right == unsinged_long_long_t)
        rres = unsigned_long_long_t;
    else if (left == long_long_t || right == long_long_t)
        res = long_long_t;
    else if (left == unisgned_int_t || right == unsigned_int_t)
        res = unsigned_int_t;
    else /* integral promotion */
        res = int_t;
    return arithmetic_specifier_init(res);
}

list *get_type_list(void *vptr) 
{
    list *ptr;
    struct_specifier *specifier;
    switch (type(vptr)) {
        case arithmetic_t:
            ptr = list_node();
            ptr->content = ((arithmetic *)vptr)->specifier;
            return ptr;
        case string_t:
            ptr = list_node();
            ptr->content = array_init(ARITHMETIC(itoa(strlen(((string *)vptr)->value))));
            ptr->content->next = arithmetic_specifier_init(char_t);
            return ptr;
        case declaration_t:
            return ((declaration *)vptr)->type_list;
        case array_ref_t:
            return ((array_ref *)vptr)->type_list;
        case struct_ref_t:
            return ((struct_ref *)vptr)->type_list;
        case posinc_t:
            return ((posinc *)vptr)->type_list;
        case preinc_t:
            return ((preinc *)vptr)->type_list;
        case addr_t:
            return ((addr *)vptr)->type_list;
        case indirection_t:
            return ((indirection *)vptr)->type_list;
        case unary_t:
            return ((unary *)vptr)->type_list;
        case sizeof_t:
            return ((size *)vptr)->type_list;
    }
}