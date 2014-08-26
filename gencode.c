#include <stdlib.h>
#include "barec.h"

void addr_gencode(addr *expr);

void declaration_gencode(declaration *expr)
{
    addr_gencode(ADDR(expr));

    switch (type(expr->type_list->content)) {
        case pointer_t: case array_t: buff_addln(text_buff, "mov eax, [eax]"); break;
        case arithmetic_specifier_t:
            switch (((arithmetic_specifier *)expr->type_list->content)->atype) {
                case unsigned_char_t: case char_t:
                    buff_addln(text_buff, "mov al, [eax]");
                    break;
                case unsigned_short_t: case short_t:
                    buff_addln(text_buff, "mov ax, [eax]");
                    break;
                case unsigned_int_t: case int_t:
                    buff_addln(text_buff, "mov eax, [eax]");
                    break;
                case unsigned_long_long_t: case long_long_t:
                    buff_addln(text_buff, "mov eax, [eax]");
                    buff_addln(text_buff, "mov edx, [eax+4]");
                    break;
            }
            break;
    }
}

void arithmetic_gencode(arithmetic *expr)
{
    long long val;
    switch(((arithmetic_specifier *)expr->type_list->content)->atype) {
        case unsigned_char_t: case char_t:
            buff_add(text_buff, "mov al, ");
            buff_addln(text_buff, expr->value);
            break;
        case unsigned_short_t: case short_t:
            buff_add(text_buff, "mov ax, ");
            buff_addln(text_buff, expr->value);
            break;
        case unsigned_int_t: case int_t:
            buff_add(text_buff, "mov eax, ");
            buff_addln(text_buff, expr->value);
            break;
        case unsigned_long_long_t: case long_long_t:
            /* CAUTION! atoll gives wrong result if you don't include <stdlib.h> */
            val = atoll(expr->value);
            buff_add(text_buff, "mov eax, ");
            buff_addln(text_buff, itoa(val & 0xffffffff));
            buff_add(text_buff, "mov edx, ");
            buff_addln(text_buff, itoa(val >> 32));
            break;
    }
}

void string_gencode(string *expr)
{
    buff_add(text_buff, "mov eax, ");
    buff_addln(text_buff, itoa(expr->address));
}

void addr_gencode(addr *expr)
{
    if (type(expr->expr) == declaration_t) {
        declaration *node = expr->expr;
        if (type(node->storage) == auto_storage_t) {
            auto_storage *storage = node->storage;
            gencode(storage->address);
            buff_add(text_buff,
                "mov ebx, ebp\n"
                "sub ebx, eax\n"
                "mov eax, ebx\n"
                );
        }
    }
    else if (type(expr->expr) == string_t) {
        string_gencode(expr->expr);
    }
    else if (type(expr->expr) == indirection_t) {
        indirection *ind = expr->expr;
        gencode(ind->expr);
    }
}

void indirection_gencode(indirection *expr)
{
    gencode(expr->expr);
    if (type(expr->type_list->content) == arithmetic_t) {
        arithmetic_specifier *a = expr->type_list->content;
        switch (a->atype) {
            case unsigned_char_t: case char_t:
                buff_add(text_buff, "mov al, [eax]\n");
                return;
            case unsigned_short_t: case short_t:
                buff_add(text_buff, "mov ax, [eax]\n");
                return;
            case unsigned_int_t: case int_t:
                buff_add(text_buff, "mov eax, [eax]\n");
                return;
            case unsigned_long_long_t: case long_long_t:
                buff_add(text_buff,
                    "mov eax, [eax]\n"
                    "mov edx, [eax+4]\n"
                    );
                return;
        }
    }
}

void unary_gencode(unary *expr)
{
    if (!strcmp(expr->op, "+"))
        return;
    else {
        gencode(expr->expr);
        if (type(get_type_list(expr->expr)->content) == arithmetic_specifier_t) {
            enum atypes atype = ((arithmetic_specifier *)get_type_list(expr->expr)->content)->atype;
            switch (atype) {
                case int_t: case unsigned_int_t:
                    buff_add(text_buff, "neg eax\n");
                    break;
            }
        }
    }
}

static inline void cast_gencode_double(list *type_list, void *expr) {
    gencode(expr);
}

static inline void cast_gencode_int(list *type_list, void *expr)
{
    gencode(expr);
    switch (type(type_list->content)) {
        case pointer_t: break;
        case arithmetic_specifier_t: {
            switch (((arithmetic_specifier *)type_list->content)->atype) {
                case long_double_t: case double_t: case float_t:
                    buff_add(text_buff, "fild eax\n");
                    break;
                case unsigned_long_long_t: case long_long_t:
                    buff_add(text_buff, "xor edx, edx\n");
                    break;
                default: break;
            }
        }
    }
}

void cast_gencode(cast *expr)
{
    void *rtype = get_type_list(expr->expr)->content;
    switch (type(rtype)) {
        case pointer_t: case array_t:
            return cast_gencode_int(expr->type_list, expr->expr);
        case arithmetic_specifier_t: {
            switch (((arithmetic_specifier *)rtype)->atype) {
                case double_t:
                    return cast_gencode_double(expr->type_list, expr->expr);
                case int_t:
                    return cast_gencode_int(expr->type_list, expr->expr);
            }
            break;
        }
    }
}

void binary_gencode_mul(void *left, void *right)
{
    switch (((arithmetic_specifier *)get_type_list(left)->content)->atype) {
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t:
            gencode(left);
            buff_add(text_buff,
                "push eax\n"
                "push edx\n"
                "\n"
                );
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n" /* hi */
                "pop ecx\n" /* lo */
                "\n"
                "mov esi, eax\n"
                "mov eax, edx\n"
                "mul ecx\n"
                "mov edi, eax\n"
                "\n"
                "mov eax, esi\n"
                "mul ebx\n"
                "add edi, eax\n"
                "\n"
                "mul ecx\n"
                "add edx, edi\n"
                "\n"
                );
            break;
        case long_long_t:
            gencode(left);
            buff_add(text_buff,
                "push eax\n"
                "push edx\n"
                "\n"
                "xor ebx, ebx\n"
                "test edx, 0x80000000\n"
                "setz bl\n"
                "push ebx\n"
                "\n"
                );
            gencode(right);
            buff_add(text_buff,
                "xor ecx, ecx\n"
                "test edx, 0x80000000\n"
                "setz cl\n"
                "\n"
                "pop ebx\n"
                "xor bl, cl\n"
                "setz bl\n"
                "mov esi, ebx\n"
                "shl esi, 31\n"
                "or esi, 0x7fffffff\n"
                "\n"
                "pop ebx\n" /* hi */
                "pop ecx\n" /* lo */
                "\n"
                "and edx, 0x7fffffff\n"
                "and ebx, 0x7fffffff\n"
                "\n"
                "mov esi, eax\n"
                "mov eax, edx\n"
                "mul ecx\n"
                "mov edi, eax\n"
                "\n"
                "mov eax, esi\n"
                "mul ebx\n"
                "add edi, eax\n"
                "\n"
                "mul ecx\n"
                "add edx, edi\n"
                "\n"
                "or edx, esi\n"
                "\n"
                );
            break;
        case unsigned_int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n"
                "mul ebx\n"
                );
            break;
        case int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n"
                "imul ebx\n"
                );
            break;
    }
}

void binary_gencode_divi(btype_t btype, void *left, void *right)
{
    switch (((arithmetic_specifier *)get_type_list(left)->content)->atype) {
        case long_double_t:
            break;
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t:
            break;
        case long_long_t:
            break;
        case unsigned_long_t: case unsigned_int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n"
                "xchg eax, ebx\n"
                "xor edx, edx\n"
                "div ebx\n"
                );
            if (btype == mod) buff_add(text_buff, "mov eax, edx\n");
            break;
        case long_t: case int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n"
                "xchg eax, ebx\n"
                "cdq\n" /* sign-extend eax to edx:eax */
                "idiv ebx\n"
                );
            if (btype == mod) buff_add(text_buff, "mov eax, edx\n");
            break;
    }
}

void binary_gencode_add(btype_t btype, void *left, void *right)
{
    enum atypes atype;

    switch (type(get_type_list(left)->content)) {
        case arithmetic_specifier_t:
            atype = ((arithmetic_specifier *)get_type_list(left)->content)->atype;
            break;
        case array_t: case pointer_t: atype = int_t; break;
    }

    switch (atype) {
        case long_double_t:
            break;
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t: case long_long_t:
            gencode(left);
            buff_add(text_buff,
                "push eax\n"
                "push edx\n"
                "\n"
                );
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n" /* hi */
                "pop ecx\n" /* lo */
                "\n"
                );
            btype == add ? buff_add(text_buff,
                                    "adc eax, ecx\n"
                                    "jnc t1\n"
                                    "inc edx\n"
                                    "t1: add edx, ebx\n"
                                    "\n")
                         : buff_add(text_buff,
                                    "sub eax, ecx\n"
                                    "sbb edx, ebx\n"
                                    "\n");
            break;
        case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
            gencode(right);
            buff_add(text_buff, "push eax\n");
            gencode(left);
            buff_add(text_buff, "pop ebx\n");
            btype == add ? buff_add(text_buff, "add eax, ebx\n")
                         : buff_add(text_buff, "sub eax, ebx\n");
            break;
    }
}

void binary_gencode_shift(btype_t btype, void *left, void *right)
{
    arithmetic_specifier *specifier = get_type_list(left)->content;
    switch (specifier->atype) {
        case unsigned_long_long_t: case long_long_t:
            break;
        case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff,
                "mov cl, al\n"
                "pop eax\n"
                );
            btype == lshift ? buff_add(text_buff, "shl eax, cl\n")
                            : buff_add(text_buff, "shr eax, cl\n");
            break;
    }
}

/*
 * The terms “above” and “below” are associated with the CF flag and refer to the relation between
 * two unsigned integer values. The terms “greater” and “less” are associated with the SF and OF
 * flags and refer to the relation between two signed integer values.
 */

void binary_gencode_relational(btype_t btype, void *left, void *right)
{
    enum atypes atype;

    switch (type(get_type_list(left)->content)) {
        case arithmetic_specifier_t:
            atype = ((arithmetic_specifier *)get_type_list(left)->content)->atype;
            break;
        case array_t: case pointer_t: atype = int_t; break;
    }

    switch (atype) {
        case long_double_t:
            break;
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t:
            break;
        case long_long_t:
            break;
        case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff,
                "pop ebx\n"
                "cmp ebx, eax\n"
                );
            break;
        case unsigned_short_t: case short_t:
            gencode(left);
            buff_add(text_buff, "push ax\n");
            gencode(right);
            buff_add(text_buff,
                "pop bx\n"
                "cmp bx, ax\n"
                );
            break;
        case unsigned_char_t: case char_t:
            gencode(left);
            buff_add(text_buff, "push ax\n");
            gencode(right);
            buff_add(text_buff,
                "pop bx\n"
                "cmp bx, al\n"
                );
            break;
    }

    switch (atype) {
        case long_double_t: case double_t: case float_t:
        case unsigned_long_long_t: case unsigned_long_t: case unsigned_int_t:
        case unsigned_short_t: case unsigned_char_t:
            btype == lt ? buff_add(text_buff, "setb al\n") :
                     gt ? buff_add(text_buff, "seta al\n") :
                     le ? buff_add(text_buff, "setbe al\n") :
                     ge ? buff_add(text_buff, "setae al\n") :
                     eq ? buff_add(text_buff, "sete al\n") :
                          buff_add(text_buff, "setne al\n");
            buff_add(text_buff, "movzx eax al\n");
            break;
        case long_long_t: case long_t: case int_t: case short_t: case char_t:
            btype == lt ? buff_add(text_buff, "setl al\n") :
                     gt ? buff_add(text_buff, "setg al\n") :
                     le ? buff_add(text_buff, "setle al\n") :
                     ge ? buff_add(text_buff, "setge al\n") :
                     eq ? buff_add(text_buff, "sete al\n") :
                          buff_add(text_buff, "setne al\n");
            buff_add(text_buff, "movzx eax al\n");
            break;
    }
}

void binary_gencode_bitwise(btype_t btype, void *left, void *right)
{
    arithmetic_specifier *specifier = get_type_list(left)->content;
    switch (specifier->atype) {
        case unsigned_long_long_t: case long_long_t:
            break;
        case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
            gencode(left);
            buff_add(text_buff, "push eax\n");
            gencode(right);
            buff_add(text_buff, "pop ebx\n");
            btype == band ? buff_add(text_buff, "and eax, ebx\n") :
                     bxor ? buff_add(text_buff, "xor eax, ebx\n") :
                            buff_add(text_buff, "or eax, ebx\n");
            break;
    }
}

void binary_gencode_logical(btype_t btype, void *left, void *right)
{
    enum atypes atype;

    switch (type(get_type_list(left)->content)) {
        case arithmetic_specifier_t:
            atype = ((arithmetic_specifier *)get_type_list(left)->content)->atype;
            break;
        case array_t: case pointer_t: atype = int_t; break;
    }

    switch (atype) {
        case long_double_t:
            break;
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t: case long_long_t:
            break;
        case unsigned_long_t: case long_t: case unsigned_int_t: case int_t: {
            gencode(left);
            buff_add(text_buff, "cmp eax, 0\n");
            char *t1 = get_tab();
            btype == land ? buff_add(text_buff, "je ") :
                           buff_add(text_buff, "jne ");
            buff_addln(text_buff, t1);
            gencode(right);
            buff_add(text_buff, "cmp eax, 0\n");
            btype == land ? buff_add(text_buff, "je ") :
                           buff_add(text_buff, "jne ");
            buff_addln(text_buff, t1);
            btype == land ? buff_add(text_buff, "mov eax, 1\n") :
                           buff_add(text_buff, "mov eax, 0\n");
            char *t2 = get_tab();
            buff_add(text_buff, "jmp "); buff_addln(text_buff, t2);
            buff_add(text_buff, t1); buff_add(text_buff, ":\n");
            btype == land ? buff_add(text_buff, "mov eax, 0\n") :
                           buff_add(text_buff, "mov eax, 1\n");
            buff_add(text_buff, t2); buff_add(text_buff, ":\n");
            break;
        }
    }
}

void binary_gencode(binary *expr)
{
    void *left = expr->left, *right = expr->right;
    switch (expr->btype) {
        case mul: binary_gencode_mul(left, right); break;
        case divi: binary_gencode_divi(divi, left, right); break;
        case mod: binary_gencode_divi(mod, left, right); break;
        case add: binary_gencode_add(add, left, right); break;
        case sub: binary_gencode_add(sub, left, right); break;
        case lshift: binary_gencode_shift(lshift, left, right); break;
        case rshift: binary_gencode_shift(rshift, left, right); break;
        case lt: binary_gencode_relational(lt, left, right); break;
        case gt: binary_gencode_relational(gt, left, right); break;
        case le: binary_gencode_relational(le, left, right); break;
        case ge: binary_gencode_relational(ge, left, right); break;
        case eq: binary_gencode_relational(eq, left, right); break;
        case neq: binary_gencode_relational(neq, left, right); break;
        case band: binary_gencode_bitwise(band, left, right); break;
        case bxor: binary_gencode_bitwise(bxor, left, right); break;
        case bor: binary_gencode_bitwise(bor, left, right); break;
        case land: binary_gencode_logical(land, left, right); break;
        case lor: binary_gencode_logical(lor, left, right); break;
    }
}

void assignment_gencode(assignment *expr)
{
    addr_gencode(ADDR(expr->expr1));
    buff_add(text_buff, "push eax\n");
    gencode(CAST(get_type_list(expr->expr1), expr->expr2));
    buff_add(text_buff, "pop ebx\n");
    void *t = get_type_list(expr->expr1)->content;
    switch (type(t)) {
        case pointer_t: case array_t:
            buff_add(text_buff, "mov [ebx], eax\n");
            break;
        case arithmetic_specifier_t:
            switch (((arithmetic_specifier *)t)->atype) {
                case unsigned_char_t: case char_t:
                    buff_add(text_buff, "mov [ebx], al\n");
                    break;
                case unsigned_short_t: case short_t:
                    buff_add(text_buff, "mov [ebx], ax\n");
                    break;
                case unsigned_int_t: case int_t:
                    buff_add(text_buff, "mov [ebx], eax\n");
                    break;
                case unsigned_long_long_t: case long_long_t:
                    buff_add(text_buff,
                        "mov [ebx], eax\n"
                        "mov [ebx+4], edx\n"
                        );
                    break;
            }
            break;
    }
}

void expression_gencode(expression *expr)
{
    list *ptr;
    for (ptr = expr->assignment_list->next; ptr; ptr = ptr->next)
        gencode(ptr->content);
}

void expression_stmt_gencode(expression_stmt *expr)
{
    list *ptr;
    for (ptr = expr->assignment_list->next; ptr; ptr = ptr->next)
        gencode(ptr->content);
}

void compound_stmt_gencode(compound_stmt *expr)
{
    list *ptr;
    for (ptr = expr->statement_list->next; ptr; ptr = ptr->next)
        gencode(ptr->content);
}

void gencode(void *expr)
{
    switch (type(expr)) {
        case declaration_t: declaration_gencode(expr); break;
        case arithmetic_t: arithmetic_gencode(expr); break;
        case string_t: string_gencode(expr); break;
        case addr_t: addr_gencode(expr); break;
        case indirection_t: indirection_gencode(expr); break;
        case unary_t: unary_gencode(expr); break;
        case cast_t: cast_gencode(expr); break;
        case binary_t: binary_gencode(expr); break;
        // case m_expr_t: m_expr_gencode(expr); break;
        // case a_expr_t: a_expr_gencode(expr); break;
        case assignment_t: assignment_gencode(expr); break;
        case expression_t: expression_gencode(expr); break;
        case expression_stmt_t: expression_stmt_gencode(expr); break;
        case compound_stmt_t: compound_stmt_gencode(expr); break;
    }
}