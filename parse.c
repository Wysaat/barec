#include <stdlib.h>
#include <string.h>
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
    retptr->type_list = list_node();
    retptr->type_list->content = arithmetic_specifier_init(atype);
    return retptr;
}

string *STRING(int address, char *value) {
    string *retptr = (string *)malloc(sizeof(string));
    retptr->type = string_t;
    retptr->address = address;
    retptr->value = value;
    retptr->type_list = list_init(pointer_init());
    retptr->type_list->next = list_init(arithmetic_specifier_init(char_t));
    return retptr;
}

indirection *ARRAY_REF(void *primary, void *expr) {
    void *expr2 = A_EXPR("+", primary, expr);
    return INDIRECTION(expr2);
}

struct_ref *STRUCT_REF(void *primary, char *id) {
    struct_ref *retptr = (struct_ref *)malloc(sizeof(struct_ref));
    retptr->type = struct_ref_t;
    retptr->primary = primary;
    retptr->id = id;

    list *type_list = get_type_list(primary), *ptr;
    struct_specifier *specifier = type_list->content;
    for (ptr = specifier->declaration_list->next; ptr; ptr = ptr->next) {
        if (!strcmp(((declaration *)ptr->content)->id, id)) {
            retptr->type_list = ((declaration *)ptr->content)->type_list;
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
    retptr->type_list = integral_promotion2(get_type_list(expr));
    return retptr;
}

static inline arithmetic *arith_size(arithmetic_specifier *ptr)
{
    char *val;
    switch (ptr->atype) {
        case unsigned_char_t: case char_t: val = "1"; break;
        case unsigned_short_t: case short_t: val = "2"; break;
        case unsigned_int_t: case int_t: val = "4"; break;
        case unsigned_long_long_t: case long_long_t: val = "8"; break;
        case float_t: val = "4"; break;
        case double_t: val = "8"; break;
    }
    return ARITHMETIC(strdup(val), int_t);
}

void *SIZE(list *type_list)
{
    list *ptr;
    void *retptr = ARITHMETIC(strdup("1"), int_t);
    for (ptr = type_list; ptr->next; ptr = ptr->next) {
        switch (type(ptr->content)) {
            case pointer_t:
                return M_EXPR("*", retptr, ARITHMETIC(strdup("4"), int_t));
            case array_t:
                retptr = M_EXPR("*", retptr, ((array *)ptr->content)->size);
                break;
        }
    }
    return M_EXPR("*", retptr, arith_size(ptr->content));
}

void *SIZE2(void *expr)
{
    return SIZE(get_type_list(expr));
}

cast *CAST(list *type_list, cast *expr)
{
    cast *retptr = (cast *)malloc(sizeof(cast));
    retptr->type = cast_t;
    retptr->type_list = type_list;
    retptr->expr = expr;
    return retptr;
}

m_expr *M_EXPR(char *op, void *expr1, void *expr2)
{
    m_expr *retptr = (m_expr *)malloc(sizeof(m_expr));
    retptr->type = m_expr_t;
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
    return retptr;
}

a_expr *A_EXPR(char *op, void *expr1, void *expr2)
{
    a_expr *retptr = (a_expr *)malloc(sizeof(a_expr));
    retptr->type = a_expr_t;
    retptr->op = op;
    retptr->left = expr1;
    retptr->right = expr2;
    retptr->type_list = list_node();
    list *ltype_list = get_type_list(expr1);
    list *rtype_list = get_type_list(expr2);
    list *int_type_list = list_init(arithmetic_specifier_init(int_t));
    if (type(ltype_list->content) == pointer_t || type(ltype_list->content) == array_t) {
        a_expr *retptr = A_EXPR("+", CAST(int_type_list, expr1), M_EXPR("*", SIZE2(INDIRECTION(expr1)), expr2));
        retptr->type_list = ltype_list;
        return retptr;
    }
    if (type(rtype_list->content) == pointer_t || type(rtype_list->content) == array_t) {
        a_expr *retptr = A_EXPR("+", CAST(int_type_list, expr2), M_EXPR("*", SIZE2(INDIRECTION(expr2)), expr1));
        retptr->type_list = rtype_list;
        return retptr;
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

expression *EXPRESSION(list *assignment_list, list *type_list)
{
    expression *retptr = (expression *)malloc(sizeof(expression));
    retptr->type = expression_t;
    retptr->assignment_list = assignment_list;
    retptr->type_list = type_list;
    return retptr;
}

arithmetic_specifier *arithmetic_convertion(arithmetic_specifier *ls, arithmetic_specifier *rs)
{
    enum atypes left = ls->atype, right = rs->atype, res;
    if (left == long_double_t || right == long_double_t)
        res = long_double_t;
    else if (left == double_t || right == double_t)
        res = double_t;
    else if (left == float_t || right == float_t)
        res = float_t;
    else if (left == unsigned_long_long_t || right == unsigned_long_long_t)
        res = unsigned_long_long_t;
    else if (left == long_long_t || right == long_long_t)
        res = long_long_t;
    else if (left == unsigned_int_t || right == unsigned_int_t)
        res = unsigned_int_t;
    else /* integral promotion */
        res = int_t;
    return arithmetic_specifier_init(res);
}

arithmetic_specifier *integral_promotion(arithmetic_specifier *s)
{
    enum atypes atype = s->atype;
    switch (atype) {
        case unsigned_char_t: case char_t: case unsigned_short_t: case_short_t:
            return arithmetic_specifier_init(int_t);
        default:
            return s;
    }
}

list *integral_promotion2(list *type_list)
{
    return list_init(integral_promotion(type_list->content));
}

list *get_type_list(void *vptr) 
{
    list *ptr;
    struct_specifier *specifier;
    switch (type(vptr)) {
        case arithmetic_t:
            return ((arithmetic *)vptr)->type_list;
        case string_t:
            return ((string *)vptr)->type_list;
        case declaration_t:
            return ((declaration *)vptr)->type_list;
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
        case cast_t:
            return ((cast *)vptr)->type_list;
        case m_expr_t:
            return ((m_expr *)vptr)->type_list;
        case a_expr_t:
            return ((a_expr *)vptr)->type_list;
        case expression_t:
            return ((expression *)vptr)->type_list;
    }
}

void set_type_list(list *type_list, void *vptr)
{
    switch (type(vptr)) {
        case arithmetic_t:
            ((arithmetic *)vptr)->type_list = type_list;
            break;
        case string_t:
            ((string *)vptr)->type_list = type_list;
            break;
        case declaration_t:
            ((declaration *)vptr)->type_list = type_list;
            break;
        case struct_ref_t:
            ((struct_ref *)vptr)->type_list = type_list;
            break;
        case posinc_t:
            ((posinc *)vptr)->type_list = type_list;
            break;
        case preinc_t:
            ((preinc *)vptr)->type_list = type_list;
            break;
        case addr_t:
            ((addr *)vptr)->type_list = type_list;
            break;
        case indirection_t:
            ((indirection *)vptr)->type_list = type_list;
            break;
        case unary_t:
            ((unary *)vptr)->type_list = type_list;
            break;
        case cast_t:
            ((cast *)vptr)->type_list = type_list;
            break;
        case m_expr_t:
            ((m_expr *)vptr)->type_list = type_list;
            break;
        case a_expr_t:
            ((a_expr *)vptr)->type_list = type_list;
            break;
        case expression_t:
            ((expression *)vptr)->type_list = type_list;
            break;
    }
}