#include "barec.h"

void declaration_gencode(declaration *expr)
{
    if (type(expr->storage) == auto_storage_t) {
        auto_storage *storage = expr->storage;
        gencode(storage->address);
    }

    switch (type(expr->type_list->content)) {
        case pointer_t: case array_t: buff_add(text_buff, "mov eax, [eax]"); break;
        case arithmetic_specifier_t:
            switch (((arithmetic_specifier *)expr->type_list->content)->atype) {
                case unsigned_char_t: case char_t:
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
    switch(expr->specifier->atype) {
        case unsigned_char_t: case char_t:
            buff_add(text_buff, "mov ax, ");
            buff_addln(text_buff, expr->value);
            break;
        case unsigned_int_t: case int_t:
            buff_add(text_buff, "mov eax, ");
            buff_addln(text_buff, expr->value);
            break;
        case unsigned_long_long_t: case long_long_t:
            val = atoll(expr->value);
            buff_add(text_buff, "mov eax, ");
            buff_addln(text_buff, itoa(val && 0xffffffff));
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
    else if (type(expr->expr) == struct_ref_t) {
    }
    else if (type(expr->expr) == array_ref_t) {
        gencode(expr->expr);
    }
    else if (type(expr->expr) == indirection_t) {
        indirection *ind = expr->expr;
        gencode(ind->expr);
    }
}

void m_expr_gencode(m_expr *expr)
{
    arithmetic_specifier *specifier = expr->type_list->content;
    switch (specifier->atype) {
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t:
            gencode(expr->left);
            buff_add(text_buff,
                "push eax\n"
                "push edx\n"
                "\n"
                );
            gencode(expr->right);
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
            break;
        case unsigned_int_t:
            gencode(expr->left);
            buff_add(text_buff, "push eax\n");
            gencode(expr->right);
            buff_add(text_buff,
                "pop ebx\n"
                "mul ebx\n"
                );
            break;
        case int_t:
            gencode(expr->left);
            buff_add(text_buff, "push eax\n");
            gencode(expr->right);
            buff_add(text_buff,
                "pop ebx\n"
                "imul ebx\n"
                );
            break;
    }
}

void a_expr_gencode(a_expr *expr)
{
    arithmetic_specifier *specifier = expr->type_list->content;
    switch (specifier->atype) {
        case double_t:
            break;
        case float_t:
            break;
        case unsigned_long_long_t: case long_long_t:
            gencode(expr->left);
            buff_add(text_buff,
                "push eax\n"
                "push edx\n"
                "\n"
                );
            gencode(expr->right);
            buff_add(text_buff,
                "pop ebx\n" /* hi */
                "pop ecx\n" /* lo */
                "\n"
                "adc, eax, ecx\n"
                "jnc t1\n"
                "inc edx\n"
                "t1: add edx, ebx\n"
                "\n"
                );
            break;
        case unsigned_int_t: case int_t:
            gencode(expr->left);
            buff_add(text_buff, "push eax\n");
            gencode(expr->right);
            buff_add(text_buff,
                "pop ebx\n"
                "add eax, ebx\n"
                );
            break;
    }
}

void assignment_gencode(assignment *expr)
{
    addr_gencode(ADDR(expr->expr1));
    buff_add(text_buff, "push eax\n");
    gencode(expr->expr2);
    buff_add(text_buff, "pop ebx\n");
    void *t = get_type_list(expr->expr2)->content;
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

void expression_stmt_gencode(expression *expr)
{
    list *ptr;
    for (ptr = expr->assignment_list->next; ptr; ptr = ptr->next)
        gencode(ptr->content);
}

void gencode(void *expr)
{
    switch (type(expr)) {
        case declaration_t: declaration_gencode(expr); break;
        case arithmetic_t: arithmetic_gencode(expr); break;
        case string_t: string_gencode(expr); break;
        case addr_t: addr_gencode(expr); break;
        case m_expr_t: m_expr_gencode(expr); break;
        case a_expr_t: a_expr_gencode(expr); break;
        case assignment_t: assignment_gencode(expr); break;
        case expression_t: expression_gencode(expr); break;
        case expression_stmt_t: expression_stmt_gencode(expr); break;
    }
}