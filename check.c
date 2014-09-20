#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "barec.h"

list *end_stack;

void end_stack_push(char *token, jmp_buf *env);
void end_stack_pop();

typedef struct {
    char *token;
    jmp_buf *env;
} end_t;

end_t *end_init(char *token, jmp_buf *env)
{
    end_t *retptr = malloc(sizeof(end_t));
    retptr->token = token;
    retptr->env = env;
    return retptr;
}

void error(FILE *stream, char *message)
{
    list *ptr;
    printf("%s:%d:%d: error: ", file_info.file_name, file_info.line, file_info.column);
    if (message)
        printf("%s\n", message);
    else
        printf("expected '%s'\n", ((end_t *)end_stack->content)->token);
    file_info.error = 1;

    int a = 0;  // {}
    int b = 0;  // []
    int c = 0;  // ()
    while (1) {
        char *token = scan(stream);
        if (!*token) {
            list *ptr;
            for (ptr = end_stack->next; ptr->next; ptr = ptr->next) {
                end_t *end = ptr->content;
                printf("%s:%d:%d: error: expected '%s'\n", file_info.file_name, file_info.line, file_info.column, end->token);
            }
            exit(-1);
        }
        if (!strcmp(token, "{"))
            a++;
        else if (!strcmp(token, "["))
            b++;
        else if (!strcmp(token, "("))
            c++;
        else if (!a && !b && !c) {
            list *ptr;
            for (ptr = end_stack; ptr->next; ptr = ptr->next) {
                end_t *end = ptr->content;
                if (!strcmp(token, end->token)) {
                    unscan(end->token, stream);
                    list *ptr2;
                    if (ptr != end_stack)
                        for (ptr2 = end_stack->next; ptr2 != ptr; ptr2 = ptr2->next) {
                            end_t *end = ptr2->content;
                            printf("%s:%d:%d: error: expected '%s'\n", file_info.file_name, file_info.line, file_info.column, end->token);
                        }
                    end_stack = ptr->next;
                    end_stack->prev = 0;
                    scan(stream);
                    longjmp(*end->env, 1);
                }
            }
        }
        else if (!strcmp(token, "}") && a > 0)
            a--;
        else if (!strcmp(token, "]") && b > 0)
            b--;
        else if (!strcmp(token, ")") && c > 0)
            c--;
    }
}

void syntax_aconstant(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        syntax_a_conditional(stream);
        token = scan(stream);
    }
    else if (!is_int(token) && !is_float(token)) {
        unscan(token, stream);
        error(stream, "expression is not an integer constant expression");
    }
}

void syntax_a_unary(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "+") || !strcmp(token, "-") || !strcmp(token, "!") || !strcmp(token, "~"))
        syntax_aconstant(stream);
    else {
        unscan(token, stream);
        syntax_aconstant(stream);
    }
}

void syntax_a_m_expr(FILE *stream)
{
    syntax_a_unary(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%"))
            syntax_a_unary(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_a_expr(FILE *stream)
{
    syntax_a_m_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-"))
            syntax_a_m_expr(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_shift(FILE *stream)
{
    syntax_a_a_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, ">>") || !strcmp(token, "<<"))
            syntax_a_a_expr(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_relational(FILE *stream)
{
    syntax_a_shift(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") || !strcmp(token, "<=") || !strcmp(token, ">="))
            syntax_a_shift(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_equality(FILE *stream)
{
    syntax_a_relational(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!="))
            syntax_a_relational(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_and(FILE *stream)
{
    syntax_a_equality(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&"))
            syntax_a_equality(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_xor(FILE *stream)
{
    syntax_a_and(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^"))
            syntax_a_and(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_or(FILE *stream)
{
    syntax_a_xor(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|"))
            syntax_a_xor(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_logical_and(FILE *stream)
{
    syntax_a_or(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&"))
            syntax_a_or(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_logical_or(FILE *stream)
{
    syntax_a_logical_and(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||"))
            syntax_a_logical_and(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_conditional(FILE *stream)
{
    syntax_a_logical_or(stream);
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        syntax_a_conditional(stream);
        token = scan(stream);
        syntax_a_conditional(stream);
    }
    else {
        unscan(token, stream);
        return;
    }
}

list *syntax_enumerator_body(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "{"
    list *enumerators = list_node();
    jmp_buf env;
    goto t2; t1: end_stack_push("}", &env);
    while (1) {
        jmp_buf env2;
        goto t4; t3: end_stack_push(",", &env2);
        char *eid = scan(stream);
        if (!is_id(eid)) {
            unscan(token, stream);
            error(stream, "expected identifier");
        }
        token = scan(stream);
        if (strcmp(token, "=") && strcmp(token, ",") && strcmp(token, "}")) {
            unscan(token, stream);
            error(stream, "expected '= constant-expression' or end of enumerator definition");
        }
        else if (!strcmp(token, "=")) {
            token = scan(stream); unscan(token, stream); // to adjust offset and file_info
            int offset = ftell(stream);
            int line = file_info.line;
            int column = file_info.column;
            syntax_conditional(stream, namespace);
            token = scan(stream);
            if (strcmp(token, ",") && strcmp(token, "}")) {
                unscan(token, stream);
                error(stream, "expected '}' or ','");
            }
            else {
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                syntax_a_conditional(stream);
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                aconstant_t *aconstant = parse_a_conditional(stream);
                if (!aconstant->is_int) {
                    fseek(stream, offset, SEEK_SET);
                    file_info.line = line;
                    file_info.column = column;
                    error(stream, "expression is not an integer constant expression");
                }
                token = scan(stream);
            }
        }
        list_append(enumerators, enumerator_init(eid, 0));
        if (!strcmp(token, ",")) {
            token = scan(stream);
            if (!strcmp(token, "}")) {
                end_stack_pop();
                end_stack_pop();
                return enumerators;
            }
            else
                unscan(token, stream);
            continue;
        }
        else if (!strcmp(token, "}")) {
            end_stack_pop();
            end_stack_pop();
            return enumerators;
        }
        t4: if (!setjmp(env2)) goto t3;
        token = scan(stream);
        if (!strcmp(token, "}"))
            return;
        unscan(token, stream);
    }
    t2: if (!setjmp(env)) goto t1;
}

char *specifier_to_string(void *specifier)
{
    list *type_list = get_type_list(specifier);
    if (type(specifier) == arithmetic_specifier_t) {
        switch (((arithmetic_specifier *)specifier)->atype) {
            case char_t: return strdup("char");
            case unsigned_char_t: return strdup("unsigned char");
            case short_t: return strdup("short");
            case unsigned_short_t: return strdup("unsigned short");
            case int_t: return strdup("int");
            case unsigned_int_t: return strdup("unsigned int");
            case long_t: return strdup("long");
            case unsigned_long_t: return strdup("unsigned long");
            case long_long_t: return strdup("long long");
            case unsigned_long_long_t: return strdup("unsigned long long");
            case float_t: return strdup("float");
            case double_t: return strdup("double");
            case long_double_t: return strdup("long double");
        }
    }
    else if (type(specifier) == struct_specifier_t) {
        char *id = ((struct_specifier *)specifier)->id;
        int retlen = strlen("struct ''") + strlen(id);
        char *retptr = malloc(retlen+1);
        retptr[retlen] = 0;
        sprintf(retptr, "struct '%s'", id);
        return retptr;
    }
    else if (type(specifier) == union_specifier_t) {
        char *id = ((union_specifier *)specifier)->id;
        int retlen = strlen("union ''") + strlen(id);
        char *retptr = malloc(retlen+1);
        retptr[retlen] = 0;
        sprintf(retptr, "union '%s'", id);
        return retptr;
    }
    else if (type(specifier) == enum_type) {
        char *id = ((struct_specifier *)specifier)->id;
        int retlen = strlen("enum ''") + strlen(id);
        char *retptr = malloc(retlen+1);
        retptr[retlen] = 0;
        sprintf(retptr, "enum '%s'", id);
        return retptr;
    }
}

list *syntax_specifier(FILE *stream, namespace_t *namespace)
{
    char *token;
    void *storage = 0, *specifier = 0;
    int got_specifier = 0;
    list *retptr = list_node();
    int sign = -1;
    int atype = -1;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "typedef"))
            storage = typedef_storage_init();
        else if (!strcmp(token, "auto"))
            storage = auto_storage_init(1, 0, 0);
        else if (!strcmp(token, "static"))
            storage = static_storage_init();
        else if (!strcmp(token, "void")) {
            if (got_specifier) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            got_specifier = 1;
        }
        else if (!strcmp(token, "char")) {
            got_specifier = 1;
            if (atype == -1)
                atype = char_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "short")) {
            got_specifier = 1;
            if (atype == -1 || atype == int_t)
                atype = short_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "int")) {
            got_specifier = 1;
            switch (atype) {
                case -1: atype = int_t; break;
                case short_t: long_t: long_long_t: break;
                default:
                    unscan(token, stream);
                    error_message("two or more data types in declaration specifiers");
                    scan(stream);
            }
        }
        else if (!strcmp(token, "long")) {
            got_specifier = 1;
            if (atype == -1 || atype == int_t)
                atype = long_t;
            else if (atype == long_t)
                atype = long_long_t;
            else if (atype == double_t)
                atype = long_double_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "float")) {
            got_specifier = 1;
            if (atype == -1)
                atype = float_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "double")) {
            got_specifier = 1;
            if (atype == -1)
                atype = double_t;
            else if (atype == long_t)
                atype = long_double_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "unsigned") || !strcmp(token, "signed")) {
            if (sign == 1) {
                unscan(token, stream);
                if (!strcmp(token, "signed"))
                    error_message("duplicate 'signed'");
                else
                    error_message("both 'signed' and 'unsigned' in declaration specifiers");
                scan(stream);
            }
            else if (sign == 0) {
                unscan(token, stream);
                if (!strcmp(token, "signed"))
                    error_message("both 'signed' and 'unsigned' in declaration specifiers");
                else
                    error_message("duplicate 'signed'");
                scan(stream);
            }
            else if (atype == float_t) {
                unscan(token, stream);
                error_message("'float' cannot be signed or unsigned");
                scan(stream);
            }
            else if (atype == double_t) {
                unscan(token, stream);
                error_message("'double' cannot be signed or unsigned");
                scan(stream);
            }
            else if (atype == long_double_t) {
                unscan(token, stream);
                error_message("'long double' cannot be signed or unsigned");
                scan(stream);
            }
            else if (specifier) {
                if (type(specifier) == struct_specifier_t) {
                    unscan(token, stream);
                    error_message("'struct' cannot be signed or unsigned");
                    scan(stream);
                }
                else if (type(specifier) == union_specifier_t) {
                    unscan(token, stream);
                    error_message("'union' cannot be signed or unsigned");
                    scan(stream);
                }
                else if (type(specifier) == enum_type) {
                    unscan(token, stream);
                    error_message("'enum' cannot be signed or unsigned");
                }
            }
            else
                sign = !strcmp(token, "signed");
        }
        else if (!strcmp(token, "struct") || !strcmp(token, "union")) {
            if (got_specifier) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            int is_struct = !strcmp(token, "struct");
            got_specifier = 1;
            if (sign >= 0) {
                unscan(token, stream);
                if (is_struct)
                    error_message("'struct' cannot be signed or unsigned");
                else
                    error_message("'union' cannot be signed or unsigned");
            }
            token = scan(stream);
            if (is_id(token)) {
                char *id = token;
                token = scan(stream);
                if (!strcmp(token, "{")) {
                    token = scan(stream);
                    if (!strcmp(token, "}")) {
                        if (is_struct)
                            specifier = struct_specifier_init(id, 0);
                        else
                            specifier = union_specifier_init(id, 0);
                    }
                    else {
                        unscan(token, stream);
                        jmp_buf env;
                        goto t2; t1: end_stack_push("}", &env);
                        struct namespace *s_namespace = namespace_init(namespace);
                        while (strcmp((token = scan(stream)), "}")) {
                            unscan(token, stream);
                            syntax_declaration(stream, s_namespace);
                        }
                        end_stack_pop(); longjmp(env, 1);
                        t2: if (!setjmp(env)) goto t1;
                        if (is_struct)
                            specifier = struct_specifier_init(id, s_namespace->declaration_list);
                        else
                            specifier = union_specifier_init(id, s_namespace->declaration_list);
                    }
                    if (is_struct)
                        list_append(namespace->struct_s_list, specifier);
                    else
                        list_append(namespace->union_s_list, specifier);
                }
                else {
                    unscan(token, stream);
                    list *ptr;
                    if (is_struct) {
                        specifier = find_struct_specifier(namespace, id);
                    }
                    else {
                        specifier = find_union_specifier(namespace, id);
                    }
                }
            }
            else if (!strcmp(token, "{")) {
                struct namespace *s_namespace = namespace_init(namespace);
                token = scan(stream);
                if (!strcmp(token, "}")) {
                    if (is_struct)
                        specifier = struct_specifier_init(0, 0);
                    else
                        specifier = union_specifier_init(0, 0);
                }
                else {
                    unscan(token, stream);
                    jmp_buf env;
                    goto t4; t3: end_stack_push("}", &env);
                    while (strcmp((token = scan(stream)), "}")) {
                        unscan(token, stream);
                        syntax_declaration(stream, s_namespace);
                    }
                    end_stack_pop(); longjmp(env, 1);
                    t4: if (!setjmp(env)) goto t3;
                    if (is_struct)
                        specifier = struct_specifier_init(0, s_namespace->declaration_list);
                    else
                        specifier = union_specifier_init(0, s_namespace->declaration_list);
                }
            }
        }
        else if (!strcmp(token, "enum")) {
            if (got_specifier) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            got_specifier = 1;
            token = scan(stream);
            if (is_id(token)) {
                char *id = token;
                token = scan(stream);
                if (!strcmp(token, "{")) {
                    unscan(token, stream);
                    list *enumerators = syntax_enumerator_body(stream, namespace);
                    specifier = enum_init(id, enumerators);
                    list_append(namespace->enums, specifier);
                }
                else {
                    unscan(token, stream);
                    specifier = find_enum(namespace, id);
                }
            }
            else {
                list *enumerators = syntax_enumerator_body(stream, namespace);
                specifier = enum_init(0, enumerators);
            }
        }
        // CAUTION!!! convert enum to int before comparing
        else if (!got_specifier && sign == -1) {
            typedef_t *t = find_typedef(namespace, token);
            if (t)
                specifier = t->type_list;
            else {
                unscan(token, stream);
                break;
            }
        }
        else {
            unscan(token, stream);
            break;
        }
    }
    if (!sign) {
        switch (atype) {
            case char_t: atype = unsigned_char_t; break;
            case short_t: atype = unsigned_short_t; break;
            case int_t: atype = unsigned_int_t; break;
            case long_t: atype = unsigned_long_t; break;
            case long_long_t: atype = unsigned_long_long_t; break;
        }
    }
    if (!specifier && atype >= 0)
        specifier = arithmetic_specifier_init(atype);
    list_append(retptr, storage);
    list_append(retptr, specifier);
    return retptr;
}

void syntax_parameter_list(FILE *stream, namespace_t *namespace)
{
    int abstract = 2;
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, ")"))
        return;
    while (1) {
        syntax_specifier(stream, namespace);
        syntax_declarator(stream, namespace, abstract);
        token = scan(stream);
        if (!strcmp(token, ","))
            ;
        else {
            unscan(token, stream);
            return;
        }
    }
}

void error_message(char *message)
{
    file_info.error = 1;
    printf("%s:%d:%d: error: %s\n", file_info.file_name, file_info.line, file_info.column, message);
}

/* 
 * abstract == 0 : id
 * abstract == 1 : no id
 * abstract == 2 : id or no id
 */
declarator *syntax_declarator(FILE *stream, namespace_t *namespace, int abstract)
{
    char *token, *id = 0;
    list *type_list = list_node();

    int line1 = file_info.line;
    int column1 = file_info.column;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "*")) {
            type_list->prev = list_init(pointer_init());
            type_list->prev->next = type_list;
            type_list = type_list->prev;
        }
        else {
            unscan(token, stream);
            break;
        }
    }
    int offset2 = ftell(stream);
    int line2 = file_info.line;
    int column2 = file_info.column;
    token = scan(stream);
    if (!strcmp(token, "(")) {
        if (abstract == 1 || abstract == 2) {
            token = scan(stream);
            if (!strcmp(token, ")")) {
                fseek(stream, offset2, SEEK_SET);
                file_info.line = line2;
                file_info.column = column2;
                goto bw;
            }
            else
                unscan(token, stream);
        }
        jmp_buf env;
        goto t2; t1: end_stack_push(")", &env);
        declarator *dptr = syntax_declarator(stream, namespace, abstract);
        if (dptr) {
            id = dptr->id;
            list *nlist = dptr->type_list;
            if (nlist->next) {
                list *ptr = nlist;
                while (ptr->next)
                    ptr = ptr->next;
                ptr->prev->next = type_list;
                type_list->prev = ptr->prev;
                free(ptr);
                type_list = nlist;
            }
        }
        token = scan(stream);
        if (strcmp(token, ")")) {
            unscan(token, stream);
            error(stream, 0);
        }
        end_stack_pop(")"); longjmp(env, 1);
        t2: if (!setjmp(env)) goto t1;
    }
    else {
        if (abstract == 0) {
            if (is_id(token))
                id = token;
            else {
                unscan(token, stream);
                error(stream, "expected identifier or '('");
            }
        }
        else if (abstract == 1)
            unscan(token, stream);
        else if (is_id(token))
            id = token;
        else
            unscan(token, stream);
    }

    bw: ; list *new_type_list = list_node();
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            token = scan(stream);
            if (!strcmp(token, "]")) {
                list_append(new_type_list, array_init(0));
            }
            else {
                unscan(token, stream);
                int offset = ftell(stream);
                int line = file_info.line;
                int column = file_info.column;
                jmp_buf env;
                goto t5; t4: end_stack_push("]", &env);
                syntax_conditional(stream, namespace);
                token = scan(stream);
                if (!strcmp(token, "]")) {
                    fseek(stream, offset, SEEK_SET);
                    file_info.line = line;
                    file_info.column = column;
                    void *array_size = parse_conditional(stream, namespace);
                    token = scan(stream);  // Don't forget the ']'.
                    list *the_type_list = get_type_list(array_size);
                    if (the_type_list) {
                        void *specifier = the_type_list->content;
                        if (type(specifier) != arithmetic_specifier_t) {
                            int cur_line = file_info.line;
                            int cur_column = file_info.column;
                            file_info.line = line;
                            file_info.column = column;
                            error_message("size of array has non-integer type");
                            file_info.line = cur_line;
                            file_info.column = cur_column;
                        }
                        else {
                            arithmetic_specifier *as = specifier;
                            if (as->atype == float_t || as->atype == double_t || as->atype == long_double_t) {
                                int cur_line = file_info.line;
                                int cur_column = file_info.column;
                                file_info.line = line;
                                file_info.column = column;
                                error_message("size of array has non-integer type");
                                file_info.line = cur_line;
                                file_info.column = cur_column;
                            }
                        }
                    }
                    list_append(new_type_list, array_init(array_size));
                }
                else {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop(); longjmp(env, 1);
                t5: if (!setjmp(env)) goto t4;
            }
        }
        else if (!strcmp(token, "(")) {
            int offset = ftell(stream);
            int line = file_info.line;
            int column = file_info.column;
            jmp_buf env;
            goto t8; t7: end_stack_push(")", &env);
            syntax_parameter_list(stream, namespace);
            token = scan(stream);
            if (!strcmp(token, ")")) {
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                list_append(new_type_list, function_init(parse_parameter_list(stream, namespace)));
                token = scan(stream);  // Don't forget the ')'.
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
            end_stack_pop(); longjmp(env, 1);
            t8: if (!setjmp(env)) goto t7;
        }
        else {
            unscan(token, stream);
            list *ptr = new_type_list;
            while (ptr->next)
                ptr = ptr->next;
            ptr->next = type_list;
            type_list->prev = ptr;
            type_list = new_type_list->next;
            type_list->prev = 0;
            int is_func = 0, is_array = 0, first_array = 1;
            for (ptr = type_list; ptr->next; ptr = ptr->next) {
                if (is_func) {
                    if (type(ptr->content) == function_t)
                        error_message_np(line1, column1, "function cannot return function type");
                    else
                        error_message_np(line1, column1, "function cannot return array type");
                    break;
                }
                else if (is_array) {
                    if (type(ptr->content) == function_t) {
                        error_message_np(line1, column1, "declaration as array of functions");
                        break;
                    }
                    else if (!first_array && !((array *)ptr->content)->size) {
                        error_message_np(line1, column1, "array type has incomplete element type");
                        break;
                    }
                }
                else if (type(ptr->content) == function_t)
                    is_func = 1;
                else {
                    is_array = 1;
                    if (!first_array && !((array *)ptr->content)->size) {
                        error_message_np(line1, column1, "array type has incomplete element type");
                        break;
                    }
                    first_array = 0;
                }
            }
            return declarator_init(id, type_list);
        }
    }
}

void syntax_initializer(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (!strcmp(token, "{")) {
        token = scan(stream);
        if (!strcmp(token, "}"))
            return;
        unscan(token, stream);
        jmp_buf env;
        goto t2; t1: end_stack_push("}", &env);
        while (1) {
            syntax_initializer(stream, namespace);
            token = scan(stream);
            if (!strcmp(token, ",")) {
                token = scan(stream);
                if (!strcmp(token, "}")) {
                    end_stack_pop();
                    return;
                }
                unscan(token, stream);
            }
            else if (!strcmp(token, "}")) {
                end_stack_pop();
                return;
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
        }
        longjmp(env, 1);
        t2: if (!setjmp(env)) goto t1;
    }
    else {
        unscan(token, stream);
        syntax_assignment(stream, namespace);
    }
}

void syntax_declaration(FILE *stream, namespace_t *namespace)
{
    list *specifiers = syntax_specifier(stream, namespace);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    if (!storage)
        storage = auto_storage_init(1, 0, 0);
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        declarator *dptr = syntax_declarator(stream, namespace, 0);
        list *type_list = dptr->type_list;
        list *ptr = type_list;
        while (ptr->next)
            ptr = ptr->next;
        if (specifier && type(specifier) == list_t) {
            if (!ptr->prev) {
                free(type_list);
                type_list = specifier;
            }
            else {
                list *last = ptr->prev;
                free(ptr);
                last->next = specifier;
                last->next->prev = last;
            }
        }
        else
            ptr->content = specifier;
        char *id = dptr->id;
        if (id) {
            if (type(storage) == typedef_storage_t)
                list_append(namespace->typedefs, typedef_init(id, type_list));
            else
                list_append(namespace->declaration_list, declaration_init(id, storage, type_list));
        }
        token = scan(stream);
        if (!strcmp(token, ","))
            continue;
        else if (!strcmp(token, "=")) {
            syntax_initializer(stream, namespace);
            token = scan(stream);
            if (!strcmp(token, ","))
                continue;
            else if (!strcmp(token, ";")) {
                end_stack_pop();
                return;
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
        }
        else if (!strcmp(token, ";"))
            return;
        else {
            unscan(token, stream);
            error(stream, 0);
        }
    }
    longjmp(env, 1);
    t2: if (!setjmp(env)) goto t1;
    return;
}

list *syntax_type_name(FILE *stream, namespace_t *namespace)
{
    list *specifiers = syntax_specifier(stream, namespace);
    void *specifier = specifiers->next->next->content;
    declarator *dptr = syntax_declarator(stream, namespace, 1);
    list *type_list = dptr->type_list;
    list *ptr = type_list;
    while (ptr->next)
        ptr = ptr->next;
    if (specifier && type(specifier) == list_t) {
        if (!ptr->prev) {
            free(type_list);
            type_list = specifier;
        }
        else {
            list *last = ptr->prev;
            free(ptr);
            last->next = specifier;
            last->next->prev = last;
        }
    }
    else
        ptr->content = specifier;
    return type_list;
}

list *syntax_primary(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (is_int(token)) {
        list *retptr = list_init(arithmetic_specifier_init(int_t));
        retptr->type = not_lvalue;
        return retptr;
    }
    else if (is_char(token)) {
        list *retptr = list_init(arithmetic_specifier_init(char_t));
        retptr->type = not_lvalue;
        return retptr;
    }
    else if (is_float(token)) {
        list *retptr = list_init(arithmetic_specifier_init(float_t));
        retptr->type = not_lvalue;
        return retptr;
    }
    else if (is_str(token)) {
        list *retptr = list_init(pointer_init());
        list_append(retptr, arithmetic_specifier_init(char_t));
        return retptr;
    }
    else if (is_id(token)) {
        declaration *dptr = find_declaration(namespace, token);
        if (!dptr) {
            unscan(token, stream);
            errorh();
            printf("use of undeclared identifier '%s'\n", token);
            scan(stream);
            return 0;
        }
        else
            return get_type_list(dptr);
    }
    else if (!strcmp(token, "(")) {
        jmp_buf env;
        goto t2; t1: end_stack_push(")", &env);
        syntax_expression(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ")")) {
            unscan(token, stream);
            error(stream, 0);
        }
        end_stack_pop();
        return;
        t2: if (!setjmp(env)) goto t1;
    }
    else {
        unscan(token, stream);
        error(stream, "expected expression");
    }
}

static inline void syntax_argument_expression_list(FILE *stream, namespace_t *namespace)
{
    char *token;
    while (1) {
        syntax_assignment(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            return;
        }
    }
}

int at_integral(int t)
{
    int types[] = {
        char_t, unsigned_char_t, short_t, unsigned_short_t, int_t, unsigned_int_t,
        long_t, unsigned_long_t, long_long_t, unsigned_long_long_t, -1,
    };
    int *ptr;
    for (ptr = types; *ptr != -1; ptr++)
        if (t == *ptr)
            return 1;
    return 0;
}

void error_message_np(int line, int column, char *message)
{
    int old_line = file_info.line;
    int old_column = file_info.column;
    file_info.line = line;
    file_info.column = column;
    error_message(message);
    file_info.line = old_line;
    file_info.column = old_column;
}

void errorh_np(int line, int column)
{
    int old_line = file_info.line;
    int old_column = file_info.column;
    file_info.line = line;
    file_info.column = column;
    errorh();
    file_info.line = old_line;
    file_info.column = old_column;
}

static list *epost_suref(char *token, list *llist, int line, int column)
{
    int lt = type(llist->content);
    if (lt != struct_specifier_t && lt != union_specifier_t) {
        llist = 0;
        error_message_np(line, column, "member reference base type is not a structure or union");
    }
    else if (lt == struct_specifier_t) {
        struct_specifier *sp = llist->content;
        list *declaration_list = sp->declaration_list, *ptr;
        int is_member = 0;
        for (ptr = declaration_list->next; ptr; ptr = ptr->next) {
            declaration *dptr = ptr->content;
            if (!strcmp(dptr->id, token)) {
                is_member = 1;
                llist = dptr->type_list;
                break;
            }
        }
        if (!is_member) {
            llist = 0;
            errorh_np(line, column);
            printf("no member name '%s' in 'struct'\n", token);
        }
    }
    else {
        union_specifier *sp = llist->content;
        list *declaration_list = sp->declaration_list, *ptr;
        int is_member = 0;
        for (ptr = declaration_list->next; ptr; ptr = ptr->next) {
            declaration *dptr = ptr->content;
            if (!strcmp(dptr->id, token)) {
                is_member = 1;
                llist = dptr->type_list;
                break;
            }
        }
        if (!is_member) {
            llist = 0;
            errorh_np(line, column);
            printf("no member name '%s' in 'union'\n", token);
        }
    }
    return llist;
}

static list *epost_incdec(char *token, FILE *stream, list *llist)
{
    if (llist) {
        if (llist->type == not_lvalue) {
            llist = 0;
            unscan(token, stream);
            error_message("expression is not assignable");
            scan(stream);
        }
        else {
            int lt = type(llist->content);
            if (lt != arithmetic_specifier_t && lt != pointer_t) {
                llist = 0;
                unscan(token, stream);
                error_message("wrong type value to increment/decrement");
                scan(stream);
            }
            else
                llist->type = not_lvalue;
        }
    }
    return llist;
}

list *syntax_postfix(FILE *stream, namespace_t *namespace)
{
    list *llist = syntax_primary(stream, namespace);
    char *token;
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        token = scan(stream);
        if (!strcmp(token, "[")) {
            token = scan(stream);
            if (!strcmp(token, "]")) {
                unscan(token, stream);
                error_message("expected experssion");
                scan(stream);
            }
            else {
                unscan(token, stream);
                jmp_buf env;
                goto t2; t1: end_stack_push("]", &env);
                list *rlist = syntax_expression(stream, namespace);
                token = scan(stream);
                if (strcmp(token, "]")) {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop();
                if (llist && rlist) {
                    int lt = type(llist->content), rt = type(rlist->content);
                    if (lt != pointer_t && lt != array_t && rt != pointer_t && rt != array_t) {
                        llist = 0;
                        error_message_np(line, column, "subscripted value is not an array, pointer, or vector");
                    }
                    else if (lt == pointer_t || lt == array_t) {
                        if (rt != arithmetic_specifier_t) {
                            llist = 0;
                            error_message_np(line, column, "array subscript is not an integer");
                        }
                        else {
                            arithmetic_specifier *a = rlist->content;
                            if (!at_integral(a->atype)) {
                                llist = 0;
                                error_message_np(line, column, "array subscript is not an integer");
                            }
                            else {
                                llist = llist->next;
                                llist->prev = 0;
                            }
                        }
                    }
                    else {
                        if (lt != arithmetic_specifier_t) {
                            llist = 0;
                            error_message_np(line, column, "array subscript is not an integer");
                        }
                        else {
                            arithmetic_specifier *a = llist->content;
                            if (!at_integral(a->atype)) {
                                llist = 0;
                                error_message_np(line, column, "array subscript is not an integer");
                            }
                            else {
                                llist = rlist->next;
                                rlist->prev = 0;
                            }
                        }
                    }
                }
                continue;
                t2: if (!setjmp(env)) goto t1;
                llist = 0;
            }
        }
        else if (!strcmp(token, "(")) {
            token = scan(stream);
            if (!strcmp(token, ")"))
                ;
            else {
                jmp_buf env;
                goto t4; t3: end_stack_push(")", &env);
                unscan(token, stream);
                syntax_argument_expression_list(stream, namespace);
                token = scan(stream);
                if (strcmp(token, ")")) {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop();
                continue;
                t4: if (!setjmp(env)) goto t3;
            }
        }
        else if (!strcmp(token, ".")) {
            token = scan(stream);
            if (!is_id(token)) {
                unscan(token, stream);
                error(stream, "expected identifier");
            }
            else if (llist)
                llist = epost_suref(token, llist, line, column);
        }
        else if (!strcmp(token, "->")) {
            token = scan(stream);
            if (!is_id(token)) {
                unscan(token, stream);
                error(stream, "expected identifier");
            }
            else if (llist) {
                int lt = type(llist->content);
                if (lt != pointer_t && lt != array_t) {
                    llist = 0;
                    error_message_np(line, column, "member reference type is not a pointer");
                }
                else
                    llist = epost_suref(token, llist->next, line, column);
            }
        }
        else if (!strcmp(token, "++") || !strcmp(token, "--"))
            llist = epost_incdec(token, stream, llist);
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_unary(FILE *stream, namespace_t *namespace)
{
    int line = file_info.line;
    int column = file_info.column;
    char *token = scan(stream);
    list *ulist;
    if (!strcmp(token, "++") || !strcmp(token, "--")) {
        ulist = syntax_unary(stream, namespace);
        if (ulist) {
            if (ulist->type == not_lvalue) {
                ulist = 0;
                error_message_np(line, column, "expression is not assignable");
            }
            else {
                int t = type(ulist->content);
                if (t != arithmetic_specifier_t && t != pointer_t) {
                    ulist = 0;
                    error_message_np(line, column, "wrong type value to increment/decrement");
                }
                else
                    ulist->type = not_lvalue;
            }
        }
    }
    else if (!strcmp(token, "&")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            if (ulist->type == not_lvalue) {
                ulist = 0;
                error_message_np(line, column, "cannot take the address of an lvalue");
            }
            else {
                list *newlist = list_init(pointer_init());
                newlist->next = ulist;
                ulist->prev = newlist;
                ulist = newlist;
                ulist->type = not_lvalue;
            }
        }
    }
    else if (!strcmp(token, "*")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            int t = type(ulist->content);
            if (t == pointer_t || t == array_t) {
                ulist = ulist->next;
                free(ulist->prev);
                ulist->prev = 0;
            }
            else {
                ulist = 0;
                error_message_np(line, column, "indirection requires pointer operand");
            }
        }
    }
    else if (!strcmp(token, "+") || !strcmp(token, "-")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            int t = type(ulist->content);
            if (t == arithmetic_specifier_t) {
                ulist->type = not_lvalue;
                arithmetic_specifier *s = ulist->content;
                if (s->atype == char_t || s->atype == unsigned_char_t ||
                          s->atype == short_t || s->atype == unsigned_short_t)
                    ulist->content = arithmetic_specifier_init(int_t);
            }
            else {
                ulist = 0;
                error_message_np(line, column, "wrong type argument to unary plus/minus");
            }
        }
    }
    else if (!strcmp(token, "~")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            int t = type(ulist->content);
            if (t == arithmetic_specifier_t) {
                arithmetic_specifier *s = ulist->content;
                if (at_integral(s->atype)) {
                    ulist->type = not_lvalue;
                    if (s->atype == char_t || s->atype == unsigned_char_t ||
                              s->atype == short_t || s->atype == unsigned_short_t)
                        ulist->content = arithmetic_specifier_init(int_t);
                }
                else {
                    ulist = 0;
                    error_message_np(line, column, "wrong type argument to bit-complement");
                }
            }
            else {
                ulist = 0;
                error_message_np(line, column, "wrong type argument to bit-complement");
            }
        }
    }
    else if (!strcmp(token, "!")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            int t = type(ulist->content);
            if (t == arithmetic_specifier_t || t == pointer_t || t == array_t) {
                ulist->type = not_lvalue;
                ulist->content = arithmetic_specifier_init(int_t);
            }
            else {
                ulist = 0;
                error_message_np(line, column, "wrong type argument to unary exclamation mark");
            }
        }
    }
    else if (!strcmp(token, "sizeof")) {
        ulist = list_init(arithmetic_specifier_init(int_t));
        ulist->type = not_lvalue;
        token = scan(stream);
        if (!strcmp(token, "(")) {
            jmp_buf env;
            goto t2; t1: end_stack_push(")", &env);
            token = scan(stream);
            unscan(token, stream);
            if (is_specifier(token) || is_qualifier(token) || find_typedef(namespace, token))
                syntax_type_name(stream, namespace);
            else
                syntax_expression(stream, namespace);
            token = scan(stream);
            if (strcmp(token, ")")) {
                unscan(token, stream);
                error(stream, 0);
            }
            end_stack_pop();
            longjmp(env, 1);
            t2: if (!setjmp(env)) goto t1;
        }
        else {
            unscan(token, stream);
            syntax_unary(stream, namespace);
        }
    }
    else {
        unscan(token, stream);
        ulist = syntax_postfix(stream, namespace);
    }
    return ulist;
}

list *syntax_cast(FILE *stream, namespace_t *namespace)
{
    int line = file_info.line;
    int column = file_info.column;
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        char *token = scan(stream);
        unscan(token, stream);
        if (is_specifier(token) || is_qualifier(token) || find_typedef(namespace, token)) {
            jmp_buf env;
            goto t2; t1: end_stack_push(")", &env);
            list *llist = syntax_type_name(stream, namespace);
            if (llist) {
                int lt = type(llist->content);
                if (lt != arithmetic_specifier_t && lt != pointer_t) {
                    llist = 0;
                    error_message_np(line, column, "arithmetic or pointer type is required");
                }
            }
            token = scan(stream);
            if (!strcmp(token, ")")) {
                end_stack_pop(); goto t3;
                t2: if (!setjmp(env)) goto t1;
                t3:
                token = scan(stream);
                unscan(token, stream);
                if (!strcmp(token, ";")) {
                    error_message("expected expression");
                    return 0;
                }
                else {
                    int line2 = file_info.line;
                    int column2 = file_info.column;
                    list *rlist = syntax_cast(stream, namespace);
                    if (llist && rlist) {
                        int rt = type(rlist->content);
                        if (rt != arithmetic_specifier_t && rt != pointer_t && rt != array_t) {
                            error_message_np(line2, column2, "arithmetic or pointer type is required");
                            return 0;
                        }
                        else if (rt = pointer_t || rt == array_t && lt == arithmetic_specifier_t) {
                            arithmetic_specifier *s = llist->content;
                            if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                                errorh_np(line2, column2);
                                printf("pointer cannot be cast to type '%s'\n", s->atype == float_t ? "float" :
                                            s->atype == double_t ? "double" : "long double");
                                return 0;
                            }
                        }
                        else {
                            llist->type = not_lvalue;
                            return llist;
                        }
                    }
                    else
                        return 0;
                }
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
        }
        else {
            unscan("(", stream);
            return syntax_primary(stream, namespace);
        }
    }
    else {
        unscan(token, stream);
        return syntax_unary(stream, namespace);
    }
}

void syntax_m_expr(FILE *stream, namespace_t *namespace) {
    syntax_cast(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%"))
            syntax_cast(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}
void syntax_a_expr(FILE *stream, namespace_t *namespace) {
    syntax_m_expr(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-"))
            syntax_m_expr(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_shift(FILE *stream, namespace_t *namespace) {
    syntax_a_expr(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<<") || !strcmp(token, ">>"))
            syntax_a_expr(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_relational(FILE *stream, namespace_t *namespace) {
    syntax_shift(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") ||
                  !strcmp(token, "<=") || !strcmp(token, ">="))
            syntax_shift(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_equality(FILE *stream, namespace_t *namespace) {
    syntax_relational(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!="))
            syntax_relational(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_and(FILE *stream, namespace_t *namespace) {
    syntax_equality(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&"))
            syntax_equality(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_xor(FILE *stream, namespace_t *namespace) {
    syntax_and(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^"))
            syntax_and(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_or(FILE *stream, namespace_t *namespace) {
    syntax_xor(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|"))
            syntax_xor(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_logical_and(FILE *stream, namespace_t *namespace) {
    syntax_or(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&"))
            syntax_or(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_logical_or(FILE *stream, namespace_t *namespace) {
    syntax_logical_and(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||"))
            syntax_logical_and(stream, namespace);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_conditional(FILE *stream, namespace_t *namespace) {
    syntax_logical_or(stream, namespace);
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        syntax_expression(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ":")) {
            unscan(token, stream);
            error_message("expected ':'");
        }
        syntax_conditional(stream, namespace);
    }
    else
        unscan(token, stream);
}

static int is_assignment_operator(char *token) {
    char *assignment_operators[] = {
        "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", 0,
        };
    char **ptr;
    for (ptr = assignment_operators; *ptr; ptr++)
        if (!strcmp(token, *ptr))
            return 1;
    return 0;
}

void syntax_assignment(FILE *stream, namespace_t *namespace) {
    syntax_conditional(stream, namespace);
    char *token = scan(stream);
    if (is_assignment_operator(token))
        syntax_assignment(stream, namespace);
    else
        unscan(token, stream);
}

list *syntax_expression(FILE *stream, namespace_t *namespace) {
    // while (1) {
    //     syntax_assignment(stream, namespace);
    //     char *token = scan(stream);
    //     if (!strcmp(token, ","))
    //         continue;
    //     else {
    //         unscan(token, stream);
    //         return;
    //     }
    // }
    return syntax_cast(stream, namespace);
}

void syntax_expression_stmt(FILE *stream, namespace_t *namespace) {
    char *token;
    token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        syntax_assignment(stream, namespace);
        token = scan(stream);
        if (!strcmp(token, ","))
            ;
        if (!strcmp(token, ";")) {
            end_stack_pop();
            return;
        }
        else {
            unscan(token, stream);
            error(stream, 0);
        }
    }
    t2: if (!setjmp(env)) goto t1;
}

void syntax_statement(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    unscan(token, stream);
    int offset = ftell(stream);
    int line = file_info.line;
    int column = file_info.column;
    if (!strcmp(token, "{"))
        syntax_compound_stmt(stream, namespace);
    else if (!strcmp(token, "if"))
        syntax_if_stmt(stream, namespace);
    else if (!strcmp(token, "switch"))
        syntax_switch_stmt(stream, namespace);
    else if (!strcmp(token, "case"))
        syntax_case_stmt(stream, namespace);
    else if (!strcmp(token, "default"))
        syntax_default_stmt(stream, namespace);
    else if (!strcmp(token, "while"))
        syntax_while_stmt(stream, namespace);
    else if (!strcmp(token, "do"))
        syntax_do_while_stmt(stream, namespace);
    else if (!strcmp(token, "for"))
        syntax_for_stmt(stream, namespace);
    else if (!strcmp(token, "goto"))
        syntax_goto_stmt(stream, namespace);
    else if (!strcmp(token, "continue"))
        syntax_continue_stmt(stream, namespace);
    else if (!strcmp(token, "break"))
        syntax_break_stmt(stream, namespace);
    else if (!strcmp(token, "return"))
        syntax_return_stmt(stream, namespace);
    else if (is_id(token)) {
        token = scan(stream);
        fseek(stream, offset, SEEK_SET);
        file_info.line = line;
        file_info.column = column;
        if (!strcmp(token, ":"))
            syntax_id_labeled_stmt(stream, namespace);
        else
            syntax_expression_stmt(stream, namespace);
    }
    else
        syntax_expression_stmt(stream, namespace);
}

void syntax_declaration_or_statement(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    unscan(token, stream);
    if (is_specifier(token) || is_storage(token) || is_qualifier(token))
        return syntax_declaration(stream, namespace);
    return syntax_statement(stream, namespace);
}

void end_stack_push(char *token, jmp_buf *env)
{
    end_stack->prev = list_init(end_init(token, env));
    end_stack->prev->next = end_stack;
    end_stack = end_stack->prev;
}

void end_stack_pop()
{
    end_stack = end_stack->next;
    free(end_stack->prev);
    end_stack->prev = 0;
}

void syntax_compound_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "{"
    jmp_buf env;
    goto t2; t1: end_stack_push("}", &env);
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "}")) {
            end_stack_pop();
            return;
        }
        unscan(token, stream);
        syntax_declaration_or_statement(stream, namespace);
    }
    longjmp(env, 1);
    t2: if (!setjmp(env)) goto t1;
}

void syntax_if_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "if"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'if'");
    }
    syntax_expression(stream, namespace);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    syntax_statement(stream, namespace);
    goto t3;
    t2: if (!setjmp(env)) goto t1;
    t3: token = scan(stream);
    if (!strcmp(token, "else"))
        syntax_statement(stream, namespace);
    else
        unscan(token, stream);
}

void syntax_switch_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "switch"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'switch'");
    }
    syntax_expression(stream, namespace);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    syntax_statement(stream, namespace);
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_case_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "case"
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    syntax_conditional(stream, namespace);
    token = scan(stream);
    if (strcmp(token, ":")) {
        unscan(token, stream);
        error(stream, "expected ':' after 'case'");
    }
    end_stack_pop();
    syntax_statement(stream, namespace);
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_default_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "default"
    token = scan(stream);
    if (strcmp(token, ":")) {
        unscan(token, stream);
        error_message("expected ':' after 'default'");
    }
    syntax_statement(stream, namespace);
}

void syntax_id_labeled_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    token = scan(stream);
    syntax_statement(stream, namespace);
}

void syntax_while_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "while"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'while'");
    }
    syntax_expression(stream, namespace);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    syntax_statement(stream, namespace);
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_do_while_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "do"
    syntax_statement(stream, namespace);
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "while")) {
        unscan(token, stream);
        error(stream, "expected 'while' in do-while loop");
    }
    token = scan(stream);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'while'");
    }
    jmp_buf env2;
    goto t4; t3: end_stack_push(")", &env);
    syntax_expression(stream, namespace);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, 0);
    }
    end_stack_pop();
    goto t5;
    t4: if (!setjmp(env2)) goto t3;
    t5: token = scan(stream);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        error(stream, 0);
    }
    end_stack_pop();
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_for_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "for"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'for'");
    }
    jmp_buf env2;
    goto t5; t4: end_stack_push(")", &env2);
    int i;
    for (i = 0; i < 2; i++) {
        token = scan(stream);
        if (strcmp(token, ";")) {
            unscan(token, stream);
            syntax_expression(stream, namespace);
            token = scan(stream);
            if (strcmp(token, ";")) {
                unscan(token, stream);
                error_message("expected ';' in for-statement");
            }
        }
    }
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        syntax_expression(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ")")) {
            unscan(token, stream);
            error(stream, 0);
        }
    }
    end_stack_pop(); goto t3;
    end_stack_pop(); goto t3;
    t5: if (!setjmp(env)) goto t4; goto t3;
    t2: if (!setjmp(env)) goto t1;
    t3: syntax_statement(stream, namespace);
}

void syntax_goto_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "goto"
    char *id = scan(stream);
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        error(stream, 0);
    }
    end_stack_pop();
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_continue_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "continue"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        error(stream, 0);
    }
    end_stack_pop();
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_break_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "break"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        error(stream, 0);
    }
    end_stack_pop();
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_return_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "return"
    token = scan(stream);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        jmp_buf env;
        goto t2; t1: end_stack_push(";", &env);
        syntax_expression(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ";")) {
            unscan(token, stream);
            error(stream, 0);
        }
        end_stack_pop();
        return;
        t2: if (!setjmp(env)) goto t1;
    }
}

void syntax_external_declaration(FILE *stream, namespace_t *namespace)
{
    list *specifiers = syntax_specifier(stream, namespace);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    if (!storage)
        storage = extern_storage_init();
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    int first_loop = 1;
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        declarator *dptr = syntax_declarator(stream, namespace, 0);
        list *type_list = dptr->type_list;
        list *ptr = type_list;
        while (ptr->next)
            ptr = ptr->next;
        if (specifier && type(specifier) == list_t) {
            if (!ptr->prev) {
                free(type_list);
                type_list = specifier;
            }
            else {
                list *last = ptr->prev;
                free(ptr);
                last->next = specifier;
                last->next->prev = last;
            }
        }
        else
            ptr->content = specifier;
        char *id = dptr->id;
        if (id) {
            if (type(storage) == typedef_storage_t)
                list_append(namespace->typedefs, typedef_init(id, type_list));
            else
                list_append(namespace->declaration_list, declaration_init(id, storage, type_list));
        }
        token = scan(stream);
        if (!strcmp(token, "{")) {
            unscan(token, stream);
            if (!first_loop)
                error(stream, 0);
            end_stack_pop();
            syntax_compound_stmt(stream, namespace);
            return;
        }
        else if (!strcmp(token, ","))
            ;
        else if (!strcmp(token, "=")) {
            syntax_initializer(stream, namespace);
            token = scan(stream);
            if (!strcmp(token, ","))
                continue;
            else if (!strcmp(token, ";")) {
                end_stack_pop();
                return;
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
            end_stack_pop();
            return;
        }
        else if (!strcmp(token, ";")) {
            end_stack_pop();
            return;
        }
        else {
            unscan(token, stream);
            error(stream, 0);
        }
        first_loop = 0;
    }
    longjmp(env, 1);
    t2: if (!setjmp(env)) goto t1;
}

void syntax_translation_unit(FILE *stream)
{
    namespace_t *namespace = namespace_init(0);
    char *token;
    end_stack = list_node();
    while (1) {
        token = scan(stream);
        if (!*token)
            return;
        unscan(token, stream);
        syntax_external_declaration(stream, namespace);
    }
}
