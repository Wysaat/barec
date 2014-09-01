#include <stdlib.h>
#include <string.h>
#include "barec.h"

int type(void *stmt) {
    return *(int *)stmt;
}

auto_storage *auto_storage_init(int constant, int iaddress, void *vaddress)
{
    auto_storage *retptr = malloc(sizeof(auto_storage));
    retptr->type = auto_storage_t;
    retptr->constant = constant;
    retptr->iaddress = iaddress;
    retptr->vaddress = vaddress;
    return retptr;
}

static_storage *static_storage_init() {
    static_storage *retptr = malloc(sizeof(static_storage));
    retptr->type = static_storage_t;
    return retptr;
}

extern_storage *extern_storage_init() {
    extern_storage *retptr = malloc(sizeof(extern_storage));
    retptr->type = extern_storage_t;
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

function *function_init(list *parameter_list) {
    function *retptr = (function *)malloc(sizeof(function));
    retptr->type = function_t;
    retptr->parameter_list = parameter_list;
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

parameter *parameter_init(void *storage, list *type_list) {
    parameter *retptr = (parameter *)malloc(sizeof(parameter));
    retptr->type = parameter_t;
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

void *ARRAY_REF(void *primary, void *expr) {
    void *expr2 = BINARY(add, primary, expr);
    return INDIRECTION(expr2);
}

static void storage_add_size(auto_storage *left, struct size *right)
{
    if (left->constant) {
        if (right->constant)
            left->iaddress += right->ival;
        else {
            left->vaddress = BINARY(add, ARITHMETIC(itoa(left->iaddress), int_t), right->vval);
            left->constant = 0;
        }
    }
    else {
        if (right->constant)
            left->vaddress = BINARY(add, left->vaddress, ARITHMETIC(itoa(right->ival), int_t));
        else
            left->vaddress = BINARY(add, left->vaddress, right->vval);
    }
}

auto_storage *auto_storage_add_size_nip(auto_storage *left, struct size *right)
{
    int constant;
    int iaddress = 0;
    void *vaddress = 0;
    if (left->constant) {
        if (right->constant) {
            constant = 1;
            iaddress = left->iaddress + right->ival;
        }
        else {
            constant = 0;
            vaddress = BINARY(add, ARITHMETIC(itoa(left->iaddress), int_t), right->vval);
        }
    }
    else {
        constant = 0;
        if (right->constant)
            vaddress = BINARY(add, left->vaddress, ARITHMETIC(itoa(right->ival), int_t));
        else
            vaddress = BINARY(add, left->vaddress, right->vval);
    }
    return auto_storage_init(constant, iaddress, vaddress);
}

declaration *STRUCT_REF(void *primary, char *id) {
    struct_specifier *specifier = get_type_list(primary)->content;
    if (specifier->declaration_list) {
        list *ptr;
        for (ptr = specifier->declaration_list->next; ptr; ptr = ptr->next) {
            declaration *node = ptr->content;
            if (!strcmp(node->id, id))
                return node;
        }
    }
}

posinc *POSINC(void *primary, int inc) {
    posinc *retptr = (posinc *)malloc(sizeof(posinc));
    retptr->type = posinc_t;
    retptr->primary = primary;
    retptr->inc = inc;
    retptr->type_list = get_type_list(primary);
    return retptr;
}

funcall *FUNCALL(void *primary, list *argument_expression_list)
{
    funcall *retptr = malloc(sizeof(funcall));
    retptr->type = funcall_t;
    retptr->primary = primary;
    retptr->argument_expression_list = argument_expression_list;
    retptr->type_list = get_type_list(primary)->next;
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

void *ADDR(void *expr) {
    list *type_list = get_type_list(expr);
    if (type(type_list->content) == function_t)
        return expr;
    addr *retptr = (addr *)malloc(sizeof(addr));
    retptr->type = addr_t;
    retptr->expr = expr;
    retptr->type_list = list_node();
    retptr->type_list->content = pointer_init();
    retptr->type_list->next = type_list;
    return retptr;
}

void *INDIRECTION(void *expr) {
    list *type_list = get_type_list(expr);
    if (type(type_list->content) == function_t)
        return expr;
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

static inline int arith_size(arithmetic_specifier *ptr)
{
    char *val;
    switch (ptr->atype) {
        case unsigned_char_t: case char_t: return 1;
        case unsigned_short_t: case short_t: return 2;
        case unsigned_int_t: case int_t: return 4;
        case unsigned_long_long_t: case long_long_t: return 8;
        case float_t: return 4;
        case double_t: return 8;
    }
}

struct size *size_init(int constant, int ival, void *vval)
{
    struct size *retptr = (struct size *)malloc(sizeof(struct size));
    retptr->constant = constant;
    retptr->ival = ival;
    retptr->vval = vval;
    return retptr;
}

struct size *size(list *type_list)
{
    int ival = 1;
    void *vval = ARITHMETIC(strdup("1"), int_t);
    int constant = 1;
    list *ptr;
    for (ptr = type_list; ptr->next; ptr = ptr->next) {
        switch (type(ptr->content)) {
            case pointer_t: {
                if (constant)
                    ival *= 4;
                else
                    vval = BINARY(mul, vval, ARITHMETIC(strdup("4"), int_t));
                return size_init(constant, ival, vval);
            }
            case array_t: {
                array *arr = (array *)ptr->content;
                if (constant) {
                    if (type(arr->size) == arithmetic_t) {
                        arithmetic *arith = (arithmetic *)arr->size;
                        arithmetic_specifier *specifier = arith->type_list->content;
                        if (specifier->atype == int_t)
                            ival *= atoi(arith->value);
                    }
                    else {
                        vval = BINARY(mul, ARITHMETIC(itoa(ival), int_t), arr->size);
                        constant = 0;
                    }
                }
                else {
                    vval = BINARY(mul, vval, arr->size);
                }
                break;
            }
        }
    }
    switch (type(ptr->content)) {
        case arithmetic_specifier_t: {
            if (constant)
                ival *= arith_size(ptr->content);
            else
                vval = BINARY(mul, vval, ARITHMETIC(itoa(arith_size(ptr->content)), int_t));
            return size_init(constant, ival, vval);
        }
        case struct_specifier_t: {
            int last_ival = 1;
            void *last_vval = ARITHMETIC(strdup("1"), int_t);
            int last_constant = 1;
            struct_specifier *specifier = (struct_specifier *)ptr->content;
            list *p;
            struct size *asize;
            if (!specifier->declaration_list)
                return 0;
            for (p = specifier->declaration_list->next; p; p = p->next) {
                asize = size(get_type_list(p->content));
                if (last_constant) {
                    if (asize->constant)
                        last_ival += asize->ival;
                    else {
                        last_vval = BINARY(add, ARITHMETIC(itoa(last_ival), int_t), asize->vval);
                        last_constant = 0;
                    }
                }
                else {
                    if (asize->constant)
                        last_vval = BINARY(add, vval, ARITHMETIC(itoa(asize->ival), int_t));
                    else
                        last_vval = BINARY(add, vval, asize->vval);
                }
            }
            if (constant) {
                if (last_constant)
                    ival *= last_ival;
                else {
                    vval = BINARY(mul, ARITHMETIC(itoa(ival), int_t), last_vval);
                    constant = 0;
                }
            }
            else {
                if (last_constant)
                    vval = BINARY(mul, vval, ARITHMETIC(itoa(last_ival), int_t));
                else
                    vval = BINARY(mul, vval, last_vval);
            }
            return size_init(constant, ival, vval);
        }
    }
}

cast *CAST(list *type_list, cast *expr)
{
    cast *retptr = (cast *)malloc(sizeof(cast));
    retptr->type = cast_t;
    retptr->type_list = type_list;
    retptr->expr = expr;
    return retptr;
}

binary *BINARY(btype_t btype, void *left, void *right)
{
    binary *retptr = (binary *)malloc(sizeof(binary));
    retptr->type = binary_t;
    retptr->btype = btype;
    retptr->left = left;
    retptr->right = right;
    switch (btype) {
        case mul: case divi: case mod: case add: case sub:
        case lshift: case rshift: case band: case bxor: case bor:
            retptr->type_list = get_type_list(left);
            break;
        default:
            retptr->type_list = list_node();
            retptr->type_list->content = arithmetic_specifier_init(int_t);
            break;
    }
    return retptr;
}

assignment *ASSIGNMENT(void *expr1, void *expr2)
{
    assignment *retptr = (assignment *)malloc(sizeof(assignment));
    retptr->type = assignment_t;
    retptr->expr1 = expr1;
    retptr->expr2 = expr2;
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

expression_stmt *EXPRESSION_STMT(list *assignment_list)
{
    expression_stmt *retptr = (expression_stmt *)malloc(sizeof(expression_stmt));
    retptr->type = expression_stmt_t;
    retptr->assignment_list = assignment_list;
    return retptr;
}

compound_stmt *COMPOUND_STMT(list *statement_list, namespace_t *namespace)
{
    compound_stmt *retptr = (compound_stmt *)malloc(sizeof(compound_stmt));
    retptr->type = compound_stmt_t;
    retptr->statement_list = statement_list;
    retptr->namespace = namespace;
    return retptr;
}

if_stmt *IF_STMT(void *expr, void *statement1, void *statement2)
{
    if_stmt *retptr = (if_stmt *)malloc(sizeof(if_stmt));
    retptr->type = if_stmt_t;
    retptr->expr = expr;
    retptr->statement1 = statement1;
    retptr->statement2 = statement2;
    return retptr;
}

switch_stmt *SWITCH_STMT(void *expr, void *statement, char *break_tag)
{
    switch_stmt *retptr = malloc(sizeof(switch_stmt));
    retptr->type = switch_stmt_t;
    retptr->expr = expr;
    retptr->statement = statement;
    retptr->break_tag = break_tag;
    return retptr;
}

case_stmt *CASE_STMT(char *tag, char *value, void *statement)
{
    case_stmt *retptr = malloc(sizeof(case_stmt));
    retptr->type = case_stmt_t;
    retptr->tag = tag;
    retptr->value = value;
    retptr->statement = statement;
    return retptr;
}

default_stmt *DEFAULT_STMT(char *tag, void *statement)
{
    default_stmt *retptr = malloc(sizeof(default_stmt));
    retptr->type = default_stmt_t;
    retptr->tag = tag;
    retptr->statement = statement;
    return retptr;
}

id_labeled_stmt *ID_LABELED_STMT(char *tag, char *id, void *statement)
{
    id_labeled_stmt *retptr = malloc(sizeof(id_labeled_stmt));
    retptr->type = id_labeled_stmt_t;
    retptr->tag = tag;
    retptr->id = id;
    retptr->statement = statement;
    return retptr;
}

while_stmt *WHILE_STMT(void *expr, void *statement, int flag, char *continue_tag, char *break_tag)
{
    while_stmt *retptr = malloc(sizeof(while_stmt));
    retptr->type = while_stmt_t;
    retptr->expr = expr;
    retptr->statement = statement;
    retptr->flag = flag;
    retptr->continue_tag = continue_tag;
    retptr->break_tag = break_tag;
    return retptr;
}

for_stmt *FOR_STMT(void *expr1, void *expr2, void *expr3, void *statement, char *continue_tag, char *break_tag)
{
    for_stmt *retptr = malloc(sizeof(for_stmt));
    retptr->type = for_stmt_t;
    retptr->expr1 = expr1;
    retptr->expr2 = expr2;
    retptr->expr3 = expr3;
    retptr->statement = statement;
    retptr->continue_tag = continue_tag;
    retptr->break_tag = break_tag;
    return retptr;
}

goto_stmt *GOTO_STMT(char *id, namespace_t *func_namespace)
{
    goto_stmt *retptr = malloc(sizeof(goto_stmt));
    retptr->type = goto_stmt_t;
    retptr->id = id;
    retptr->func_namespace = func_namespace;
    return retptr;
}

continue_stmt *CONTINUE_STMT(char *tag)
{
    continue_stmt *retptr = malloc(sizeof(continue_stmt));
    retptr->type = continue_stmt_t;
    retptr->tag = tag;
    return retptr;
}

break_stmt *BREAK_STMT(char *tag)
{
    break_stmt *retptr = malloc(sizeof(break_stmt));
    retptr->type = break_stmt_t;
    retptr->tag = tag;
    return retptr;
}

return_stmt *RETURN_STMT(void *expr, char *tag)
{
    return_stmt *retptr = malloc(sizeof(return_stmt));
    retptr->type = return_stmt_t;
    retptr->expr = expr;
    retptr->tag = tag;
    return retptr;
}

arithmetic_specifier *integral_promotion(arithmetic_specifier *s)
{
    enum atypes atype = s->atype;
    switch (atype) {
        case unsigned_char_t: case char_t: case unsigned_short_t: case short_t:
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
        case binary_t:
            return ((binary *)vptr)->type_list;
        case expression_t:
            return ((expression *)vptr)->type_list;
        case func_t:
            return ((func *)vptr)->type_list;
        case funcall_t:
            return ((funcall *)vptr)->type_list;
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
        case binary_t:
            ((binary *)vptr)->type_list = type_list;
            break;
        case expression_t:
            ((expression *)vptr)->type_list = type_list;
            break;
        case func_t:
            ((func *)vptr)->type_list = type_list;
            break;
        case funcall_t:
            ((funcall *)vptr)->type_list = type_list;
            break;
    }
}