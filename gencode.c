#include <stdlib.h>
#include <string.h>

#include "barec.h"

void addr_gencode(addr *expr);
static char *bss_addr(int addr);
static char *data_addr(int addr);

void declaration_gencode(declaration *expr)
{
    gencode(ADDR(expr));
    switch (type(expr->type_list->content)) {
        case pointer_t: buff_add(text_buff, "mov eax, [eax]\n"); break;
        case array_t: break;
        case arithmetic_specifier_t:
            switch (((arithmetic_specifier *)expr->type_list->content)->atype) {
                case unsigned_char_t: case char_t:
                    buff_add(text_buff, "mov al, [eax]\n");
                    break;
                case unsigned_short_t: case short_t:
                    buff_add(text_buff, "mov ax, [eax]\n");
                    break;
                case unsigned_int_t: case int_t:
                    buff_add(text_buff, "mov eax, [eax]\n");
                    break;
                case unsigned_long_t: case long_t:
                    buff_add(text_buff, "mov eax, [eax]\n");
                    break;
                case unsigned_long_long_t: case long_long_t:
                    buff_add(text_buff, "mov ecx, eax\n");
                    buff_add(text_buff, "mov eax, [ecx]\n");
                    buff_add(text_buff, "mov edx, [ecx+4]\n");
                    break;
                case float_t:
                    buff_add(text_buff,
                        "ffree st0\n"
                        "fld dword [eax]\n"
                        );
                    break;
                case double_t:
                    buff_add(text_buff,
                        "ffree st0\n"
                        "fld qword [eax]\n"
                        );
                    break;
            }
            break;
        case enum_type:
            buff_add(text_buff, "mov eax, [eax]\n");
            break;
        case struct_specifier_t: case union_specifier_t:
            // do nothing
            break;
    }
}

void func_gencode(func *expr)
{
    buff_add(text_buff, "mov eax, ");
    buff_addln(text_buff, expr->tag);
}

void arithmetic_gencode(arithmetic *expr)
{
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
        case unsigned_long_t: case long_t:
            buff_add(text_buff, "mov eax, ");
            buff_addln(text_buff, expr->value);
            break;
        case unsigned_long_long_t: case long_long_t: {
            /* CAUTION! atoll gives wrong results if you don't include <stdlib.h> */
            long long val;
            val = atoll(expr->value);
            buff_add(text_buff, "mov eax, ");
            buff_addln(text_buff, itoa(val & 0xffffffff));
            buff_add(text_buff, "mov edx, ");
            buff_addln(text_buff, itoa(val >> 32));
            break;
        }
        case float_t:
            buff_add(text_buff, "ffree st0\n");
            buff_add(text_buff, "push ");
            buff_addln(text_buff, expr->value);
            buff_add(text_buff,
                "fld dword [esp]\n"
                "pop eax\n"
                );
            break;
        case double_t: {
            buff_add(text_buff, "ffree st0\n");
            char *tag = get_tag();
            buff_add(data_buff, tag);
            buff_add(data_buff, ":\ndq ");
            buff_addln(data_buff, expr->value);
            buff_add(text_buff, "fld qword [");
            buff_add(text_buff, tag);
            buff_add(text_buff, "]\n");
            break;
        }
    }
}

void string_gencode(string *expr)
{
    buff_add(text_buff, "mov eax, ");
    buff_addln(text_buff, data_addr(expr->address));
}

void posinc_gencode(posinc *expr)
{
    gencode(expr->primary);
    list *type_list = get_type_list(expr);
    switch (type(type_list->content)) {
        case pointer_t: {
            buff_add(text_buff, "push eax\n");
            gencode(ADDR(expr->primary));
            buff_add(text_buff, "push eax\n");
            gencode(size_expr(type_list->next));
            if (expr->inc)
                buff_add(text_buff,
                    "pop ebx\n"
                    "add [ebx], eax\n"
                    "pop eax\n"
                    );
            else
                buff_add(text_buff,
                    "pop ebx\n"
                    "sub [ebx], eax\n"
                    "pop eax\n"
                    );
            break;
        }
        case arithmetic_specifier_t: {
            arithmetic_specifier *specifier = type_list->content;
            switch (specifier->atype) {
                case unsigned_long_long_t: case long_long_t:
                    buff_add(text_buff,
                        "push edx\n"
                        "push eax\n"
                        );
                    gencode(ADDR(expr->primary));
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc qword [eax]\n"
                            "pop eax\n"
                            "pop edx\n"
                        );
                    else
                        buff_add(text_buff,
                            "dec qword [eax]\n"
                            "pop eax\n"
                            "pop edx\n"
                        );
                    break;
                case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
                    buff_add(text_buff, "push eax\n");
                    gencode(ADDR(expr->primary));
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc dword [eax]\n"
                            "pop eax\n"
                        );
                    else
                        buff_add(text_buff,
                            "dec dword [eax]\n"
                            "pop eax\n"
                        );
                    break;
                case unsigned_short_t: case short_t:
                    buff_add(text_buff, "push eax\n");
                    gencode(ADDR(expr->primary));
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc word [eax]\n"
                            "pop eax\n"
                        );
                    else
                        buff_add(text_buff,
                            "dec word [eax]\n"
                            "pop eax\n"
                        );
                    break;
                case unsigned_char_t: case char_t:
                    buff_add(text_buff, "push eax\n");
                    gencode(ADDR(expr->primary));
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc word [eax]\n"
                            "pop eax\n"
                        );
                    else
                        buff_add(text_buff,
                            "dec word [eax]\n"
                            "pop eax\n"
                        );
                    break;
                case float_t:
                    gencode(ADDR(expr->primary));
                    if (expr->inc)
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld dword [eax]\n"
                            "fld1\n"
                            "faddp\n"
                            "fstp dword [eax]\n"
                        );
                    else
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld dword [eax]\n"
                            "fld1\n"
                            "fsubp\n"
                            "fstp dword [eax]\n"
                        );
                    break;
                case double_t:
                    gencode(ADDR(expr->primary));
                    if (expr->inc)
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld qword [eax]\n"
                            "fld1\n"
                            "faddp\n"
                            "fstp qword [eax]\n"
                        );
                    else
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld qword [eax]\n"
                            "fld1\n"
                            "fsubp\n"
                            "fstp qword [eax]\n"
                        );
                    break;
            }
            break;
        }
    }
}

void preinc_gencode(preinc *expr)
{
    list *type_list = get_type_list(expr);
    switch (type(type_list->content)) {
        case pointer_t: {
            gencode(ADDR(expr->expr));
            buff_add(text_buff, "push eax\n");
            gencode(size_expr(type_list->next));
            if (expr->inc)
                buff_add(text_buff,
                    "pop ebx\n"
                    "add [ebx], eax\n"
                    "mov eax, [ebx]\n"
                    );
            else
                buff_add(text_buff,
                    "pop ebx\n"
                    "sub [ebx], eax\n"
                    "mov eax, [ebx]\n"
                    );
            break;
        }
        case arithmetic_specifier_t: {
            gencode(ADDR(expr->expr));
            arithmetic_specifier *specifier = type_list->content;
            switch (specifier->atype) {
                case unsigned_long_long_t: case long_long_t:
                    if (expr->inc)
                        buff_add(text_buff,
                            "mov ecx, eax\n"
                            "inc qword [ecx]\n"
                            "mov eax, [ecx]\n"
                            "mov edx, [ecx+4]\n"
                            );
                    else
                        buff_add(text_buff,
                            "mov ecx, eax\n"
                            "dec qword [ecx]\n"
                            "mov eax, [ecx]\n"
                            "mov edx, [ecx+4]\n"
                            );
                    break;
                case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc dword [eax]\n"
                            "mov eax, [eax]\n"
                            );
                    else
                        buff_add(text_buff,
                            "dec dword [eax]\n"
                            "mov eax, [eax]\n"
                            );
                    break;
                case unsigned_short_t: case short_t:
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc word [eax]\n"
                            "mov ax, [eax]\n"
                            );
                    else
                        buff_add(text_buff,
                            "dec word [eax]\n"
                            "mov ax, [eax]\n"
                            );
                    break;
                case unsigned_char_t: case char_t:
                    if (expr->inc)
                        buff_add(text_buff,
                            "inc byte [eax]\n"
                            "mov al, [eax]\n"
                            );
                    else
                        buff_add(text_buff,
                            "dec byte [eax]\n"
                            "mov al, [eax]\n"
                            );
                    break;
                case float_t:
                    if (expr->inc)
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld dword [eax]\n"
                            "fld1\n"
                            "faddp\n"
                            "fst dword [eax]\n"
                            );
                    else
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld dword [eax]\n"
                            "fld1\n"
                            "fsubp\n"
                            "fst dword [eax]\n"
                            );
                    break;
                case double_t:
                    if (expr->inc)
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld qword [eax]\n"
                            "fld1\n"
                            "faddp\n"
                            "fst qword [eax]\n"
                            );
                    else
                        buff_add(text_buff,
                            "ffree st0\n"
                            "fld qword [eax]\n"
                            "fld1\n"
                            "fsubp\n"
                            "fst qword [eax]\n"
                            );
                    break;
            }
            break;
        }
    }
}

void union_ref_gencode(union_ref_t *expr)
{
    list *old_type_list = get_type_list(expr->primary);
    set_type_list(expr->primary, expr->type_list);
    gencode(expr->primary);
    set_type_list(expr->primary, old_type_list);
}

// if flag == 0, convert arrays to pointers
static int gencode_and_push_argument(void *expr, int flag)
{
    int size = 0;
    list *type_list = get_type_list(expr);
    switch (type(type_list->content)) {
        case pointer_t: case function_t: // converted to a pointer
            gencode(expr);
            buff_add(text_buff, "push eax\n");
            size += 4;
            break;
        case array_t:
            if (flag == 0) {
                gencode(expr);
                buff_add(text_buff, "push eax\n");
                size += 4;
            }
            else {
                array *a = type_list->content;
                if (type(a->size) == arithmetic_t) {
                    int val = atoi(((arithmetic *)a->size)->value), i;
                    for (i = val-1; i >= 0; i--) {
                        size += gencode_and_push_argument(ARRAY_REF(expr, ARITHMETIC(itoa(i), int_t)), 1);
                    }
                }
            }
            break;
        case arithmetic_specifier_t: {
            gencode(expr);
            arithmetic_specifier *specifier = type_list->content;
            switch (specifier->atype) {
                case unsigned_int_t: case int_t: case unsigned_long_t: case long_t:
                    buff_add(text_buff, "push eax\n");
                    size += 4;
                    break;
                case unsigned_long_long_t: case long_long_t:
                    buff_add(text_buff, "push eax\n");
                    buff_add(text_buff, "push edx\n");
                    size += 8;
                    break;
                case float_t:
                    buff_add(text_buff,
                        "sub esp, 4\n"
                        "fst dword [esp]\n"
                        );
                    size += 4;
                    break;
                case double_t:
                    buff_add(text_buff,
                        "sub esp, 8\n"
                        "fst qword [esp]\n"
                        );
                    size += 8;
                    break;
            }
            break;
        }
        case struct_specifier_t: {
            struct_specifier *specifier = type_list->content;
            if (!specifier->declaration_list)
                return;
            list *ptr = specifier->declaration_list;
            while (ptr->next)
                ptr = ptr->next;
            for ( ; ptr != specifier->declaration_list; ptr = ptr->prev)
                size += gencode_and_push_argument(ptr->content, 1);
            break;
        }
    }
    return size;
}

void funcall_gencode(funcall *expr)
{
    int size = 0;
    if (expr->argument_expression_list) {
        list *ptr = expr->argument_expression_list;
        while (ptr->next)
            ptr = ptr->next;
        for ( ; ptr != expr->argument_expression_list; ptr = ptr->prev)
            size += gencode_and_push_argument(ptr->content, 0);
    }
    gencode(expr->primary);
    buff_add(text_buff, "call eax\n");
    buff_add(text_buff, "add esp, ");
    buff_addln(text_buff, itoa(size));
}

static char *bss_addr(int addr) {
    char *ichar = itoa(addr);
    char *retptr = malloc(strlen(ichar)+strlen(bss_start_tag)+1);
    sprintf(retptr, "%s+%s", bss_start_tag, ichar);
    return retptr;
}

static char *data_addr(int addr) {
    char *ichar = itoa(addr);
    char *retptr = malloc(strlen(ichar)+strlen(data_start_tag)+2);
    sprintf(retptr, "%s+%s", data_start_tag, ichar);
    return retptr;
}

void addr_gencode(addr *expr)
{
    if (type(expr->expr) == declaration_t) {
        declaration *node = expr->expr;
        if (type(node->storage) == auto_storage_t) {
            auto_storage *storage = node->storage;
            if (storage->constant) {
                buff_add(text_buff, "mov eax, ");
                buff_addln(text_buff, itoa(storage->ival));
            }
            else
                gencode(storage->vval);
            buff_add(text_buff,
                "mov ebx, ebp\n"
                "sub ebx, eax\n"
                "mov eax, ebx\n"
            );
        }
        else if (type(node->storage) == parameter_storage_t) {
            parameter_storage *storage = node->storage;
            buff_add(text_buff, "lea eax, [ebp+");
            // NOTE! EIP is pushed AFTER arguments are pushed,
            //       so there are stored EBP and EIP on top of the arguments in the stack.
            //       should add 8.
            buff_add(text_buff, itoa(8+storage->address));
            buff_add(text_buff, "]\n");
        }
        else if (type(node->storage) == static_storage_t) {
            static_storage *storage = node->storage;
            buff_add(text_buff, "mov eax, ");
            char *val;
            if (storage->initialized)
                val = data_addr(storage->ival);
            else
                val = bss_addr(storage->ival);
            buff_addln(text_buff, val);
        }
        else if (type(node->storage) == static_offsetted_storage_type) {
            static_offsetted_storage_t *storage = node->storage;
            buff_add(text_buff, "mov eax, ");
            char *val;
            if (storage->base->initialized)
                val = data_addr(storage->base->ival+storage->offset);
            else
                val = bss_addr(storage->base->ival+storage->offset);
            buff_addln(text_buff, val);
        }
    }
    else if (type(expr->expr) == string_t) {
        string_gencode(expr->expr);
    }
    else if (type(expr->expr) == indirection_t) {
        indirection *ind = expr->expr;
        gencode(ind->expr);
    }
    else if (type(expr->expr) == union_ref_type) {
        union_ref_t *uni = expr->expr;
        gencode(ADDR(uni->primary));
    }
}

void indirection_gencode(indirection *expr)
{
    gencode(expr->expr);
    if (type(expr->type_list->content) == arithmetic_specifier_t) {
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
            case unsigned_long_t: case long_t:
                buff_add(text_buff, "mov eax, [eax]\n");
                return;
            case unsigned_long_long_t: case long_long_t:
                buff_add(text_buff,
                    "mov ecx, eax\n"
                    "mov eax, [ecx]\n"
                    "mov edx, [ecx+4]\n"
                );
                return;
            case float_t:
                buff_add(text_buff,
                    "ffree st0\n"
                    "fld dword [eax]\n"
                );
                return;
            case double_t:
                buff_add(text_buff,
                    "ffree st0\n"
                    "fld qword [eax]\n"
                );
                return;
        }
    }
    // NOTE! if type(expr->type_list->content) == pointer_t, don't need to do anything!
    else if (type(expr->type_list->content) == pointer_t)
        buff_add(text_buff, "mov eax, [eax]\n");
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
                case long_t: case unsigned_long_t:
                    buff_add(text_buff, "neg eax\n");
                    break;
                case long_long_t: case unsigned_long_long_t:
                    buff_add(text_buff,
                        "not eax\n"
                        "not ebx\n"
                        "add eax, 1\n"
                        "adc ebx, 1\n"
                    );
                    break;
                case float_t: case double_t:
                    buff_add(text_buff, "fchs\n");
                    break;
            }
        }
    }
}

static inline void cast_char(int lt);
static inline void cast_short(int lt);
static inline void cast_int(int lt);
static inline void cast_long(int lt);
static inline void cast_uchar(int lt);
static inline void cast_ushort(int lt);
static inline void cast_uint(int lt);
static inline void cast_ulong(int lt);
static inline void cast_double(int lt);

static inline void cast_char(int lt) {
    switch (lt) {
        case char_t: case unsigned_char_t: return;
        default: buff_add(text_buff, "cbw\n"); cast_short(lt); return;
    }
}

static inline void cast_uchar(int lt) {
    switch (lt) {
        case char_t: case unsigned_char_t: return;
        default: buff_add(text_buff, "xor ah, ah\n"); cast_ushort(lt); return;
    }
}

static inline void cast_short(int lt) {
    switch (lt) {
        case char_t: case unsigned_char_t: return;
        case short_t: case unsigned_short_t: return;
        default: buff_add(text_buff, "cwde\n"); cast_int(lt); return;
    }
}

static inline void cast_ushort(int lt) {
    switch (lt) {
        case char_t: case unsigned_char_t: return;
        case short_t: case unsigned_short_t: return;
        default: buff_add(text_buff, "movzx eax, ax\n"); cast_uint(lt); return;
    }
}

static inline void cast_int(int lt) {
    switch (lt) {
        case char_t: case unsigned_char_t: return;
        case short_t: case unsigned_short_t: return;
        case int_t: case unsigned_int_t: return;
        case long_t: case unsigned_long_t: return;
        default: buff_add(text_buff, "cdq\n"); cast_long(lt); return;
    }
}

static inline void cast_uint(int lt) {
    switch (lt) {
        case char_t: case unsigned_char_t: return;
        case short_t: case unsigned_short_t: return;
        case int_t: case unsigned_int_t: return;
        case long_t: case unsigned_long_t: return;
        default: buff_add(text_buff, "xor edx, edx\n"); cast_ulong(lt); return;
    }
}

static inline void cast_long(int lt) {
    switch (lt) {
        case float_t: case double_t:
            buff_add(text_buff,
                "ffree st0\n"
                "push edx\n"
                "push eax\n"
                "fild qword [esp]\n"
                "pop eax\n"
                "pop edx\n"
            );
            return;
        default:
            return;
    }
}

static inline void cast_ulong(int lt) {
    switch (lt) {
        case float_t: case double_t:
            buff_add(text_buff,
                "ffree st0\n"
                "push edx\n"
                "push eax\n"
                "fild qword [esp]\n"
                "pop eax\n"
                "pop edx\n"
            );
            return;
        default:
            return;
    }
}

static inline void cast_double(int lt) {
    if (lt == float_t || lt == double_t)
        return;
    else
        buff_add(text_buff, "sub esp, 8\nfistp qword [esp]\npop eax\npop edx\n");
}

void cast_gencode(cast *expr) {
    gencode(expr->expr);

    list *llist = expr->type_list;
    list *rlist = get_type_list(expr->expr);
    int lt, rt;
    if (type(llist->content) == pointer_t)
        lt = int_t;
    else
        lt = ((arithmetic_specifier *)llist->content)->atype;
    if (type(rlist->content) == pointer_t)
        rt = int_t;
    else
        rt = ((arithmetic_specifier *)rlist->content)->atype;

    switch (rt) {
        case char_t: cast_char(lt); break;
        case short_t: cast_short(lt); break;
        case int_t: cast_int(lt); break;
        case long_t: cast_int(lt); break;
        case long_long_t: cast_long(lt); break;
        case unsigned_char_t: cast_uchar(lt); break;
        case unsigned_short_t: cast_ushort(lt); break;
        case unsigned_int_t: cast_uint(lt); break;
        case unsigned_long_t: cast_uint(lt); break;
        case unsigned_long_long_t: cast_ulong(lt); break;
        case float_t: case double_t: cast_double(lt); break;
    }
}

void binary_gencode_mul(void *left, void *right)
{
    switch (((arithmetic_specifier *)get_type_list(left)->content)->atype) {
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
        case float_t:
            gencode(left);
            buff_add(text_buff,
                "sub esp, 4\n"
                "fst dword [esp]\n"
                );
            gencode(right);
            buff_add(text_buff,
                "fmul dword [esp]\n"
                "pop eax\n"
                );
            break;
        case double_t:
            gencode(left);
            buff_add(text_buff,
                "sub esp, 8\n"
                "fst qword [esp]\n"
                );
            gencode(right);
            buff_add(text_buff,
                "fmul qword [esp]\n"
                "pop eax\n"
                "pop eax\n"
                );
            break;
    }
}

void binary_gencode_divi(btype_t btype, void *left, void *right)
{
    switch (((arithmetic_specifier *)get_type_list(left)->content)->atype) {
        case unsigned_int_t: case unsigned_long_t:
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
        case int_t: case long_t:
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
        case unsigned_long_long_t: case long_long_t:
            fprintf(stderr, "[FATAL] in gencode.c:binary_gencode_divi\n");
            exit(-1);
        case float_t:
            gencode(right);
            buff_add(text_buff,
                "sub esp, 4\n"
                "fst dword [esp]\n"
                );
            gencode(left);
            buff_add(text_buff,
                "fdiv dword [esp]\n"
                "pop eax\n"
                );
            break;
        case double_t:
            gencode(right);
            buff_add(text_buff,
                "sub esp, 8\n"
                "fst qword [esp]\n"
                );
            gencode(left);
            buff_add(text_buff,
                "fdiv qword [esp]\n"
                "pop eax\n"
                "pop eax\n"
                );
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
        case unsigned_long_t: case long_t: case unsigned_int_t: case int_t:
            gencode(right);
            buff_add(text_buff, "push eax\n");
            gencode(left);
            buff_add(text_buff, "pop ebx\n");
            btype == add ? buff_add(text_buff, "add eax, ebx\n")
                         : buff_add(text_buff, "sub eax, ebx\n");
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
            if (btype == add) {
                char *tag = get_tag();
                buff_add(text_buff, "adc eax, ecx\n");
                buff_add(text_buff, "jnc ");
                buff_addln(text_buff, tag);
                buff_add(text_buff, "inc edx\n");
                buff_add(text_buff, tag);
                buff_add(text_buff, ": add edx, ebx\n");
            }
            else {
                buff_add(text_buff,
                    "sub eax, ecx\n"
                    "sbb edx, ebx\n"
                    "\n");
            }
            break;
        case float_t:
            gencode(right);
            buff_add(text_buff,
                "sub esp, 4\n"
                "fst dword [esp]\n"
                );
            gencode(left);
            btype == add ?
                buff_add(text_buff,
                    "fadd dword [esp]\n"
                    "pop eax\n"
                    ) :
                buff_add(text_buff,
                    "fsub dword [esp]\n"
                    "pop eax\n"
                    );
            break;
        case double_t:
            gencode(right);
            buff_add(text_buff,
                "sub esp, 8\n"
                "fst qword [esp]\n"
                );
            gencode(left);
            btype == add ?
                buff_add(text_buff,
                    "fadd qword [esp]\n"
                    "pop eax\n"
                    "pop eax\n"
                    ) :
                buff_add(text_buff,
                    "fsub qword [esp]\n"
                    "pop eax\n"
                    "pop eax\n"
                    );
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

    if (atype == long_long_t || atype == unsigned_long_long_t) {
        gencode(left);
        buff_add(text_buff,
            "push edx\n"
            "push eax\n"
        );
        gencode(right);
        buff_add(text_buff,
            "pop ecx\n"
            "pop ebx\n"
            "cmp ecx, edx\n"
        );
        if (btype == lt || gt || le || ge) {
            char *jlt = atype == long_long_t ? "jl " : "jb ";
            char *jgt = atype == long_long_t ? "jg " : "ja ";
            char *jle = atype == long_long_t ? "jle " : "jbe ";
            char *jge = atype == long_long_t ? "jge " : "jae ";
            char *tag1 = get_tag();
            char *tag2 = get_tag();
            buff_add(text_buff, (btype==lt||btype==le) ? jlt : jgt);
            buff_addln(text_buff, tag1);
            buff_add(text_buff, (btype==lt||btype==le) ? jgt : jlt);
            buff_addln(text_buff, tag2);
            buff_add(text_buff, "cmp ebx, eax\n");
            buff_add(text_buff, btype==lt ? jge : btype==gt ? jle : btype==le ? jgt : jlt);
            buff_addln(text_buff, tag2);
            buff_add(text_buff, tag1);
            buff_add(text_buff, ":\n");
            buff_add(text_buff, "mov eax, 1\n");
            buff_add(text_buff, tag2);
            buff_add(text_buff, ":\n");
            buff_add(text_buff, "mov eax, 0\n");
        }
        else if (btype == eq || btype == neq) {
            char *tag = get_tag();
            buff_add(text_buff, "jne ");
            buff_addln(text_buff, tag);
            buff_add(text_buff, "cmp ebx, eax\n");
            buff_add(text_buff, "jne ");
            buff_addln(text_buff, tag);
            buff_add(text_buff, btype==eq ? "mov eax, 1\n" : "mov eax, 0\n");
            buff_add(text_buff, tag);
            buff_add(text_buff, ":\n");
            buff_add(text_buff, btype==eq? "mov eax, 0\n" : "mov eax, 1\n");
        }
    }
    else {
        switch (atype) {
            case unsigned_char_t: case char_t:
                gencode(left);
                buff_add(text_buff, "push ax\n");
                gencode(right);
                buff_add(text_buff,
                    "pop bx\n"
                    "cmp bl, al\n"
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
            case unsigned_int_t: case int_t: case unsigned_long_t: case long_t:
                gencode(left);
                buff_add(text_buff, "push eax\n");
                gencode(right);
                buff_add(text_buff,
                    "pop ebx\n"
                    "cmp ebx, eax\n"
                    );
                break;
        }
        switch (atype) {
            case unsigned_char_t: case unsigned_short_t: case unsigned_int_t: case unsigned_long_t:
                btype == lt ? buff_add(text_buff, "setb al\n") :
                btype == gt ? buff_add(text_buff, "seta al\n") :
                btype == le ? buff_add(text_buff, "setbe al\n") :
                btype == ge ? buff_add(text_buff, "setae al\n") :
                btype == eq ? buff_add(text_buff, "sete al\n") :
                              buff_add(text_buff, "setne al\n");
                buff_add(text_buff, "movzx eax, al\n");
                break;
            case char_t: case short_t: case int_t: case long_t:
                btype == lt ? buff_add(text_buff, "setl al\n") :
                btype == gt ? buff_add(text_buff, "setg al\n") :
                btype == le ? buff_add(text_buff, "setle al\n") :
                btype == ge ? buff_add(text_buff, "setge al\n") :
                btype == eq ? buff_add(text_buff, "sete al\n") :
                              buff_add(text_buff, "setne al\n");
                buff_add(text_buff, "movzx eax, al\n");
                break;
        }
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
            btype == bxor ? buff_add(text_buff, "xor eax, ebx\n") :
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
            char *t1 = get_tag();
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
            char *t2 = get_tag();
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

void conditional_gencode(conditional_t *expr)
{
    gencode(expr->expr1);
    char *tag1 = get_tag();
    buff_add(text_buff, "jz ");
    buff_addln(text_buff, tag1);
    gencode(expr->expr2);
    char *tag2 = get_tag();
    buff_add(text_buff, "jmp ");
    buff_addln(text_buff, tag2);
    buff_add(text_buff, tag1);
    buff_add(text_buff, ":\n");
    gencode(expr->expr3);
    buff_add(text_buff, tag2);
    buff_add(text_buff, ":\n");
}

void assignment_gencode(assignment *expr)
{
    gencode(ADDR(expr->expr1));
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
                case unsigned_long_t: case long_t:
                    buff_add(text_buff, "mov [ebx], eax\n");
                    break;
                case unsigned_long_long_t: case long_long_t:
                    buff_add(text_buff,
                        "mov [ebx], eax\n"
                        "mov [ebx+4], edx\n"
                        );
                    break;
                case float_t:
                    buff_add(text_buff, "fst dword [ebx]\n");
                    break;
                case double_t:
                    buff_add(text_buff, "fst qword [ebx]\n");
                    break;
            }
            break;
        case struct_specifier_t: {
            list *declarations_left = ((struct_specifier *)get_type_list(expr->expr1)->content)->declaration_list;
            list *declarations_right = ((struct_specifier *)get_type_list(expr->expr2)->content)->declaration_list;
            if (!declarations_left)
                return;
            list *ptr1, *ptr2;
            for (ptr1 = declarations_left->next, ptr2 = declarations_right->next; ptr1 && ptr2; ptr1 = ptr1->next, ptr2 = ptr2->next)
                gencode(ASSIGNMENT(ptr1->content, ptr2->content));
        }
    }
}

static inline char *init_prefix(int size)
{
    switch (size) {
        case 1: return "db ";
        case 2: return "dw ";
        case 4: return "dd ";
        case 8: return "dq ";
    }
}

static inline char *constant_value(void *constant)
{
    switch (type(constant)) {
        case arithmetic_t:
            return ((arithmetic *)constant)->value;
        case addr_t: {
            declaration *dptr = ((addr *)constant)->expr;
            if (type(dptr->storage) == static_storage_t) {
                static_storage *ss = dptr->storage;
                if (ss->initialized)
                    return data_addr(ss->ival);
                else
                    return bss_addr(ss->ival);
            }
            else if (type(dptr->storage) == static_offsetted_storage_type) {
                static_offsetted_storage_t *sos = dptr->storage;
                if (sos->base->initialized)
                    return data_addr(sos->base->ival+sos->offset);
                else
                    return bss_addr(sos->base->ival+sos->offset);
            }
        }
    }
}

static void sig_subproc1(void *body)
{
    if (type(body) == compound_stmt_t) {
        compound_stmt *stmt = body;
        list *ptr;
        for (ptr = stmt->statement_list->next; ptr; ptr = ptr->next)
            sig_subproc1(ptr->content);
    }
    else {
        assignment *stmt = body;
        struct size *thesize = size(get_type_list(stmt->expr1));
        char *prefix = init_prefix(thesize->ival);
        buff_add(data_buff, prefix);
        if (stmt->expr2)
            buff_addln(data_buff, constant_value(stmt->expr2));
        else
            buff_addln(data_buff, "0");
    }
}

static void sig_subproc2(void *body)
{
    if (type(body) == compound_stmt_t) {
        compound_stmt *stmt = body;
        list *ptr;
        for (ptr = stmt->statement_list->next; ptr; ptr = ptr->next)
            sig_subproc2(ptr->content);
    }
    else {
        assignment *stmt = body;
        struct size *thesize = size(get_type_list(stmt->expr1));
        buff_add(bss_buff, "resb ");
        buff_addln(bss_buff, itoa(thesize->ival));
    }
}

void static_initialization_gencode(static_initialization *initial)
{
    list *ptr;
    if (initial->initialized)
        sig_subproc1(initial->body);
    else
        sig_subproc2(initial->body);
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

void if_stmt_gencode(if_stmt *stmt)
{
    gencode(BINARY(neq, stmt->expr, ARITHMETIC(strdup("0"), int_t)));
    char *tag = get_tag();
    buff_add(text_buff, "cmp eax, 0\n");
    buff_add(text_buff, "je ");
    buff_addln(text_buff, tag);
    gencode(stmt->statement1);
    buff_add(text_buff, tag);
    buff_add(text_buff, ":\n");
    if (stmt->statement2)
        gencode(stmt->statement2);
}

static list *list_add_ip(list *left, list *right)
{
    list *ptr = left;
    while (ptr->next)
        ptr = ptr->next;
    ptr->next = right->next;
    return left;
}

static list *find_cases(void *statement)
{
    switch (type(statement)) {
        case expression_stmt_t:
            return list_node();
        case compound_stmt_t: {
            compound_stmt *compound = statement;
            list *ptr;
            list *retptr = list_node();
            for (ptr = compound->statement_list->next; ptr; ptr = ptr->next)
                retptr = list_add_ip(retptr, find_cases(ptr->content));
            return retptr;
        }
        case id_labeled_stmt_t:
            return find_cases(((id_labeled_stmt *)statement)->statement);
        case case_stmt_t:
            return list_add_ip(list_init_wh(statement), find_cases(((case_stmt *)statement)->statement));
        case default_stmt_t:
            return list_add_ip(list_init_wh(statement), find_cases(((default_stmt *)statement)->statement));
        case if_stmt_t:
            return list_add_ip(find_cases(((if_stmt *)statement)->statement1),
                               find_cases(((if_stmt *)statement)->statement2));
        case switch_stmt_t:
            return list_node();
    }
}

void switch_stmt_gencode(switch_stmt *stmt)
{
    gencode(stmt->expr);
    list *cases = find_cases(stmt->statement), *ptr;
    default_stmt *d = 0;
    for (ptr = cases->next; ptr; ptr = ptr->next) {
        if (type(ptr->content) == case_stmt_t) {
            case_stmt *c = ptr->content;
            buff_add(text_buff, "cmp eax, ");
            buff_addln(text_buff, c->value);
            buff_add(text_buff, "je ");
            buff_addln(text_buff, c->tag);
        }
        else
            d = ptr->content;
    }
    buff_add(text_buff, "jmp ");
    if (d)
        buff_addln(text_buff, d->tag);
    else
        buff_addln(text_buff, stmt->break_tag);
    gencode(stmt->statement);
    buff_add(text_buff, stmt->break_tag);
    buff_add(text_buff, ":\n");
}

void case_stmt_gencode(case_stmt *stmt)
{
    buff_add(text_buff, stmt->tag);
    buff_add(text_buff, ":\n");
    gencode(stmt->statement);
}

void default_stmt_gencode(default_stmt *stmt)
{
    buff_add(text_buff, stmt->tag);
    buff_add(text_buff, ":\n");
    gencode(stmt->statement);
}

void id_labeled_stmt_gencode(id_labeled_stmt *stmt)
{
    buff_add(text_buff, stmt->tag);
    buff_add(text_buff, ":\n");
    gencode(stmt->statement);
}

void while_stmt_gencode(while_stmt *stmt)
{
    void *ftag = get_tag();
    if (!stmt->flag) {
        buff_add(text_buff, "jmp ");
        buff_addln(text_buff, stmt->continue_tag);
    }
    buff_add(text_buff, ftag);
    buff_add(text_buff, ":\n");
    gencode(stmt->statement);
    buff_add(text_buff, stmt->continue_tag);
    buff_add(text_buff, ":\n");
    gencode(BINARY(eq, stmt->expr, ARITHMETIC(strdup("0"), int_t)));
    buff_add(text_buff, "test eax, eax\n");
    buff_add(text_buff, "jz ");
    buff_addln(text_buff, ftag);
    buff_add(text_buff, stmt->break_tag);
    buff_add(text_buff, ":\n");
}

void for_stmt_gencode(for_stmt *stmt)
{
    if (stmt->expr1)
        gencode(stmt->expr1);
    void *ftag = get_tag();
    buff_add(text_buff, "jmp ");
    buff_addln(text_buff, stmt->continue_tag);
    buff_add(text_buff, ftag);
    buff_add(text_buff, ":\n");
    gencode(stmt->statement);
    if (stmt->expr3)
        gencode(stmt->expr3);
    buff_add(text_buff, stmt->continue_tag);
    buff_add(text_buff, ":\n");
    if (stmt->expr2) {
        gencode(BINARY(neq, stmt->expr2, ARITHMETIC(strdup("0"), int_t)));
        buff_add(text_buff, "cmp eax, 0\n");
        buff_add(text_buff, "jne ");
        buff_addln(text_buff, ftag);
    }
    buff_add(text_buff, stmt->break_tag);
    buff_add(text_buff, ":\n");
}

void goto_stmt_gencode(goto_stmt *stmt)
{
    list *ptr;
    char *tag;
    for (ptr = stmt->func_namespace->labels->next; ptr; ptr = ptr->next) {
        id_labeled_stmt *label = ptr->content;
        if (!strcmp(stmt->id, label->id))
            tag = label->tag;
    }
    buff_add(text_buff, "jmp ");
    buff_addln(text_buff, tag);
}

void continue_stmt_gencode(continue_stmt *stmt)
{
    buff_add(text_buff, "jmp ");
    buff_addln(text_buff, stmt->tag);
}

void break_stmt_gencode(break_stmt *stmt)
{
    buff_add(text_buff, "jmp ");
    buff_addln(text_buff, stmt->tag);
}

void return_stmt_gencode(return_stmt *stmt)
{
    gencode(stmt->expr);
    buff_add(text_buff, "jmp ");
    buff_addln(text_buff, stmt->tag);
}

void function_definition_gencode(function_definition_t *function_definition)
{
    buff_add(text_buff, function_definition->function_tag);
    buff_add(text_buff, ":\n");
    buff_add(text_buff,
        "push ebp\n"
        "mov ebp, esp\n"
        );
    gencode(size_to_expr(function_definition->body->namespace->auto_size));
    buff_add(text_buff, "sub esp, eax\n");
    compound_stmt_gencode(function_definition->body);
    buff_add(text_buff, function_definition->return_tag);
    buff_add(text_buff,
        ":\n"
        "leave\n"
        "ret\n"
        );
}

void static_addr_gencode(static_addr_t *expr) {
    if (expr->initialized)
        buff_add(text_buff, data_addr(expr->value));
    else
        buff_add(text_buff, bss_addr(expr->value));
}

void gencode(void *expr)
{
    switch (type(expr)) {
        case declaration_t: declaration_gencode(expr); break;
        case func_t: func_gencode(expr); break;
        case arithmetic_t: arithmetic_gencode(expr); break;
        case static_addr_type: static_addr_gencode(expr); break;
        case string_t: string_gencode(expr); break;
        case posinc_t: posinc_gencode(expr); break;
        case union_ref_type: union_ref_gencode(expr); break;
        case funcall_t: funcall_gencode(expr); break;
        case addr_t: addr_gencode(expr); break;
        case indirection_t: indirection_gencode(expr); break;
        case preinc_t: preinc_gencode(expr); break;
        case unary_t: unary_gencode(expr); break;
        case cast_t: cast_gencode(expr); break;
        case binary_t: binary_gencode(expr); break;
        case conditional_type: conditional_gencode(expr); break;
        case assignment_t: assignment_gencode(expr); break;
        case expression_t: expression_gencode(expr); break;
        case expression_stmt_t: expression_stmt_gencode(expr); break;
        case compound_stmt_t: compound_stmt_gencode(expr); break;
        case if_stmt_t: if_stmt_gencode(expr); break;
        case switch_stmt_t: switch_stmt_gencode(expr); break;
        case case_stmt_t: case_stmt_gencode(expr); break;
        case default_stmt_t: default_stmt_gencode(expr); break;
        case id_labeled_stmt_t: id_labeled_stmt_gencode(expr); break;
        case while_stmt_t: while_stmt_gencode(expr); break;
        case for_stmt_t: for_stmt_gencode(expr); break;
        case goto_stmt_t: goto_stmt_gencode(expr); break;
        case continue_stmt_t: continue_stmt_gencode(expr); break;
        case break_stmt_t: break_stmt_gencode(expr); break;
        case return_stmt_t: return_stmt_gencode(expr); break;
        case static_initialization_t: static_initialization_gencode(expr); break;
        case function_definition_type: function_definition_gencode(expr); break;
    }
}

void translation_unit_gencode(translation_unit_t *translation_unit)
{
    list *ptr;
    for (ptr = translation_unit->external_declarations->next; ptr; ptr = ptr->next) {
        gencode(ptr->content);
        buff_add(text_buff, "\n");
    }
}