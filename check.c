#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "barec.h"

static list *end_stack;

static int in_loop;
static int in_switch;
static list *return_type_list;

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
    fprintf(stderr, "%s:%d:%d: error: ", file_info.file_name, file_info.line, file_info.column);
    if (message)
        fprintf(stderr, "%s\n", message);
    else
        fprintf(stderr, "expected '%s'\n", ((end_t *)end_stack->content)->token);
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
                fprintf(stderr, "%s:%d:%d: error: expected '%s'\n", file_info.file_name, file_info.line, file_info.column, end->token);
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
                            fprintf(stderr, "%s:%d:%d: error: expected '%s'\n",
                                          file_info.file_name, file_info.line, file_info.column, end->token);
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
            return enumerators;
        unscan(token, stream);
    }
    t2: if (!setjmp(env)) goto t1;
}

char *specifier_to_string(void *specifier)
{
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
            storage = static_storage_init(0, 0);
        else if (!strcmp(token, "void")) {
            if (got_specifier) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            got_specifier = 1;
            specifier = void_specifier_init();
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
                case short_t: case long_t: case long_long_t: break;
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
                            syntax_declaration(stream, s_namespace, 1);
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
                        syntax_declaration(stream, s_namespace, 1);
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
        else if (is_id(token)) {
            if (specifier || atype >= 0 || sign != -1) {
                unscan(token, stream);
                break;
            }
            else {
                unscan(token, stream);
                int offset = ftell(stream);
                int line = file_info.line;
                int column = file_info.column;
                char *id = scan(stream);
                typedef_t *t = find_typedef(namespace, id);
                if (t)
                    specifier = t->type_list;
                else {
                    token = scan(stream);
                    if (!strcmp(token, ",") || !strcmp(token, ";")) {
                        fseek(stream, offset, SEEK_SET);
                        file_info.line = line;
                        file_info.column = column;
                        break;
                    }
                    else {
                        unscan(token, stream);
                        errorh_np(line, column);
                        fprintf(stderr, "unknown type name '%s'\n", id);
                        break;
                    }
                }
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
    else if (!specifier && sign != -1)
        specifier = arithmetic_specifier_init(sign == 0 ? unsigned_int_t : int_t);
    if (!specifier)
        specifier = arithmetic_specifier_init(int_t);
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
    fprintf(stderr, "%s:%d:%d: error: %s\n", file_info.file_name, file_info.line, file_info.column, message);
}

declarator_info_t *declarator_info_init(char *id, list *type_list, int line, int column) {
    declarator_info_t *retptr = malloc(sizeof(declarator_info_t));
    retptr->type = declarator_info_type;
    retptr->id = id;
    retptr->type_list = type_list;
    retptr->line = line;
    retptr->column = column;
    return retptr;
}

/* 
 * abstract == 0 : id
 * abstract == 1 : no id
 * abstract == 2 : id or no id
 */
declarator_info_t *syntax_declarator(FILE *stream, namespace_t *namespace, int abstract)
{
    char *token, *id = 0;
    list *type_list = list_node();

    int line1 = file_info.line;
    int column1 = file_info.column;

    int line;
    int column;

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
        declarator_info_t *dptr = syntax_declarator(stream, namespace, abstract);
        if (dptr) {
            id = dptr->id;
            line = dptr->line;
            column = dptr->column;
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
    else if (abstract == 1)
        unscan(token, stream);
    else if (is_id(token)) {
        unscan(token, stream);
        id = token;
        line = file_info.line;
        column = file_info.column;
        scan(stream);
    }
    else if (abstract == 0) {
        unscan(token, stream);
        error(stream, "expected identifier or '('");
    }
    else
        unscan(token, stream);

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
            list *parameter_list;
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
            end_stack_pop();
            continue;
            t8: if (!setjmp(env)) goto t7;
            list_append(new_type_list, function_init(0));
        }
        else {
            unscan(token, stream);
            list *ptr;
            int is_func = 0, is_array = 0;
            for (ptr = new_type_list->next; ptr; ptr = ptr->next) {
                if (is_func) {
                    if (type(ptr->content) == function_t)
                        error_message_np(line1, column1, "function cannot return function type");
                    else
                        error_message_np(line1, column1, "function cannot return array type");
                    type_list = list_node();
                    break;
                }
                else if (is_array) {
                    if (type(ptr->content) == function_t) {
                        error_message_np(line1, column1, "declaration as array of functions");
                        type_list = list_node();
                        break;
                    }
                    else if (!((array *)ptr->content)->size) {
                        error_message_np(line1, column1, "array type has incomplete element type");
                        type_list = list_node();
                        break;
                    }
                }
                else if (type(ptr->content) == function_t)
                    is_func = 1;
                else
                    is_array = 1;
            }
            ptr = new_type_list;
            while (ptr->next)
                ptr = ptr->next;
            ptr->next = type_list;
            type_list->prev = ptr;
            type_list = new_type_list->next;
            type_list->prev = 0;
            return declarator_info_init(id, type_list, line, column);
        }
    }
}

typedef struct {
    int type;
    list *type_list;
    int line;
    int column;
} init_info_t;

init_info_t *init_info_init(list *type_list, int line, int column) {
    init_info_t *retptr = malloc(sizeof(init_info_t));
    retptr->type = init_info_type;
    retptr->type_list = type_list;
    retptr->line = line;
    retptr->column = column;
    return retptr;
}

void *syntax_initializer(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (!strcmp(token, "{")) {
        list *retptr = list_node();
        token = scan(stream);
        if (!strcmp(token, "}"))
            return retptr;
        unscan(token, stream);
        jmp_buf env;
        goto t2; t1: end_stack_push("}", &env);
        while (1) {
            void *val = syntax_initializer(stream, namespace);
            list_append(retptr, val);
            token = scan(stream);
            if (!strcmp(token, ",")) {
                token = scan(stream);
                if (!strcmp(token, "}")) {
                    end_stack_pop();
                    return retptr;
                }
                unscan(token, stream);
            }
            else if (!strcmp(token, "}")) {
                end_stack_pop();
                return retptr;
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
        int line = file_info.line;
        int column = file_info.column;
        return init_info_init(syntax_assignment(stream, namespace), line, column);
    }
}

static void syntax_initializing(list *llist, void *initializer)
{
    switch (type(llist->content)) {
        case arithmetic_specifier_t: case enum_type: case pointer_t: case function_t: {
            if (type(initializer) == list_t)
                syntax_initializing(llist, ((list *)initializer)->next->content);
            else {
                init_info_t *init_info = initializer;
                list *rlist = init_info->type_list;
                int line = init_info->line;
                int column = init_info->column;
                if (llist->type == function_t)
                    error_message_np(line, column, "illegal initializer");
                else {
                    int lt = type(llist->content), rt = type(rlist->content);
                    if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                        arithmetic_specifier *s = llist->content;
                        if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                            errorh_np(line, column);
                            fprintf(stderr, "initializing '%s' with an expression of incompatible type '%s'\n",
                                type_list_to_str(llist), type_list_to_str(rlist));
                        }
                    }
                    else if (rt == arithmetic_specifier_t && lt == pointer_t) {
                        arithmetic_specifier *s = rlist->content;
                        if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                            errorh_np(line, column);
                            fprintf(stderr, "initializing '%s' with an expression of incompatible type '%s'\n",
                                type_list_to_str(llist), type_list_to_str(rlist));
                        }
                    }
                    else if ((lt == struct_specifier_t || lt == union_specifier_t) && !type_list_eq(llist, rlist)) {
                        errorh_np(line, column);
                        fprintf(stderr, "initializing '%s' with an expression of incompatible type '%s'\n",
                            type_list_to_str(llist), type_list_to_str(rlist));
                    }
                }

            }
            return;
        }
        case array_t: {
            array *a = llist->content;
            void *content_s = llist->next->content;
            int subsize = -1;
            if (type(content_s) == array_t) {
                array *a = content_s;
                subsize = atoi(((arithmetic *)a->size)->value);
            }
            else if (type(content_s) == struct_specifier_t) {
                struct_specifier *s = content_s;
                if (s->declaration_list)
                    subsize = list_len(s->declaration_list);
                else
                    subsize = 0;
            }
            int size = 0, i;
            list *stmts = list_node(), *ptr;
            if (a->size)
                size = atoi(((arithmetic *)a->size)->value);
            for (i = 0, ptr = ((list *)initializer)->next; ptr; i++) {
                void *inival;
                if (ptr->content && type(ptr->content) == list_t) {
                    inival = ptr->content;
                    ptr = ptr->next;
                }
                else if (subsize >= 0) {
                    int i;
                    list *sub_initializer = list_node();
                    for (i = 0; i < subsize && ptr; i++, ptr = ptr->next)
                        list_append(sub_initializer, ptr->content);
                    inival = sub_initializer;
                }
                else {
                    inival = ptr->content;
                    ptr = ptr->next;
                }
                syntax_initializing(llist->next, inival);
                if (size && i == size)
                    return;
            }
            return;
        }
        case struct_specifier_t: {
            struct_specifier *specifier = llist->content;
            if (!specifier->declaration_list)
                return;
            list *stmts = list_node(), *ptr, *ptr2;
            for (ptr = specifier->declaration_list->next, ptr2 = ((list *)initializer)->next;
                       ptr && ptr2; ptr = ptr->next) {
                declaration *node = ptr->content;
                void *s = node->type_list->content, *inival;
                int subsize = -1;
                if (type(s) == array_t) {
                    array *a = s;
                    subsize = atoi(((arithmetic *)a->size)->value);
                }
                else if (type(s) == struct_specifier_t) {
                    struct_specifier *ss = s;
                    if (ss->declaration_list)
                        subsize = list_len(ss->declaration_list);
                    else
                        subsize = 0;
                }
                if (ptr2->content && type(ptr2->content) == list_t) {
                    inival = ptr2->content;
                    ptr2 = ptr2->next;
                }
                else if (subsize >= 0) {
                    list *sub_initializer = list_node();
                    int i;
                    for (i = 0; i < subsize && ptr2; i++, ptr2 = ptr2->next)
                        list_append(sub_initializer, ptr2->content);
                    inival = sub_initializer;
                }
                else {
                    inival = ptr2->content;
                    ptr2 = ptr2->next;
                }
                syntax_initializing(node->type_list, inival);
            }
            return;
        }
    }
}

void syntax_declaration(FILE *stream, namespace_t *namespace, int in_struct)
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
        declarator_info_t *dptr = syntax_declarator(stream, namespace, 0);
        int line = dptr->line;
        int column = dptr->column;
        list *type_list = dptr->type_list;
        list *ptr = type_list;
        while (ptr->next)
            ptr = ptr->next;
        if (type(specifier) == list_t) {
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
            void *val;
            if ((val=find_declaration_cn(namespace, id)) || (val=find_typedef_cn(namespace, id))) {
                if (type(val) == declaration_t && type(storage) == typedef_storage_t ||
                          type(val) == typedef_type && type(storage) != typedef_storage_t) {
                    errorh_np(line, column);
                    fprintf(stderr, "'%s' redeclared as different kind of symbol\n", id);
                }
                else if (!type_list_eq(get_type_list(val), type_list)) {
                    errorh_np(line, column);
                    fprintf(stderr, "conflicting types for '%s'\n", id);
                }
                else if (type(val) == declaration_t) {
                    errorh_np(line, column);
                    fprintf(stderr, "redefinition of '%s'\n", id);
                }
            }
            else if (type(storage) == typedef_storage_t)
                list_append(namespace->typedefs, typedef_init(id, type_list));
            else
                list_append(namespace->declaration_list, declaration_init(id, storage, type_list));
        }
        token = scan(stream);
        if (in_struct) {
            if (!strcmp(token, ","))
                ;
            else if (!strcmp(token, ":")) {
            }
            else if (!strcmp(token, ";")) {
                end_stack_pop();
                return;
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
        }
        else {
            if (!strcmp(token, ","))
                ;
            else if (!strcmp(token, "=")) {
                list *initializer = syntax_initializer(stream, namespace);
                syntax_initializing(type_list, initializer);
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
            else if (!strcmp(token, ";")) {
                end_stack_pop();
                return;
            }
            else {
                unscan(token, stream);
                error(stream, 0);
            }
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
    declarator_info_t *dptr = syntax_declarator(stream, namespace, 1);
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
        retptr->type = constant_t;
        return retptr;
    }
    else if (is_char(token)) {
        list *retptr = list_init(arithmetic_specifier_init(char_t));
        retptr->type = constant_t;
        return retptr;
    }
    else if (is_float(token)) {
        list *retptr = list_init(arithmetic_specifier_init(float_t));
        retptr->type = constant_t;
        return retptr;
    }
    else if (is_str(token)) {
        list *retptr = list_init(array_init(ARITHMETIC(itoa(strlen(token)), int_t)));
        list_append(retptr, arithmetic_specifier_init(char_t));
        return retptr;
    }
    else if (is_id(token)) {
        declaration *dptr = find_declaration(namespace, token);
        if (!dptr) {
            unscan(token, stream);
            errorh();
            fprintf(stderr, "use of undeclared identifier '%s'\n", token);
            scan(stream);
            return 0;
        }
        else
            return type_list_copy(get_type_list(dptr));
    }
    else if (!strcmp(token, "(")) {
        jmp_buf env;
        goto t2; t1: end_stack_push(")", &env);
        list *retptr = syntax_expression(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ")")) {
            unscan(token, stream);
            error(stream, 0);
        }
        end_stack_pop();
        return retptr;
        t2: if (!setjmp(env)) goto t1;
    }
    else {
        unscan(token, stream);
        error(stream, "expected expression");
    }
}

static inline list *syntax_argument_expression_list(FILE *stream, namespace_t *namespace)
{
    char *token;
    list *retptr = list_node();
    int error;
    while (1) {
        list *cur = syntax_assignment(stream, namespace);
        if (!cur)
            error = 1;
        else
            list_append(retptr, cur);
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            if (error)
                return 0;
            else
                return retptr;
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
            fprintf(stderr, "no member name '%s' in 'struct'\n", token);
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
            fprintf(stderr, "no member name '%s' in 'union'\n", token);
        }
    }
    if (llist) {
        llist = type_list_copy(llist);
        if (type(llist->content) == array_t)
            llist->content = pointer_init();
    }
    return llist;
}

static inline list *type_list_copy(list *type_list)
{
    list *retptr = list_init(type_list->content);
    list *ptr, *ptr2;
    for (ptr = type_list->next, ptr2 = retptr; ptr; ptr = ptr->next, ptr2 = ptr2->next) {
        ptr2->next = list_init(ptr->content);
        ptr2->next->prev = ptr2;
    }
    return retptr;
}

static list *epost_incdec(char *token, FILE *stream, list *llist)
{
    if (llist) {
        if (llist->type == constant_t) {
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
            else {
                llist = type_list_copy(llist);
                llist->type = constant_t;
            }
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
                            else
                                llist = llist->next;
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
            list *arguments;
            if (!strcmp(token, ")"))
                arguments = list_node();
            else {
                jmp_buf env;
                goto t4; t3: end_stack_push(")", &env);
                unscan(token, stream);
                arguments = syntax_argument_expression_list(stream, namespace);
                token = scan(stream);
                if (strcmp(token, ")")) {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop();
                longjmp(env, 1);
                t4: if (!setjmp(env)) goto t3;
                arguments = 0;
            }
            if (llist && arguments) {
                if (type(llist->content) == function_t || (type(llist->content) == pointer_t &&
                          type(llist->next->content) == function_t)) {
                    list *parameter_list;
                    if (type(llist->content) == function_t)
                        parameter_list = ((function *)llist->content)->parameter_list;
                    else
                        parameter_list = ((function *)llist->next->content)->parameter_list;
                    list *ptr1, *ptr2;
                    for (ptr1 = parameter_list->next, ptr2 = arguments->next; ptr1 && ptr2; ptr1 = ptr1->next, ptr2 = ptr2->next) {
                        list *llist = ((declaration *)ptr1->content)->type_list;
                        list *rlist = ptr2->content;
                        int lt = type(llist->content), rt = type(rlist->content);
                        if ((lt == struct_specifier_t || lt == union_specifier_t) && !type_list_eq(llist, rlist)) {
                            errorh_np(line, column);
                            fprintf(stderr, "assigning to '%s' from incompatible type '%s'\n",
                                type_list_to_str(llist), type_list_to_str(rlist));
                            llist = 0;
                        }
                        else if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                            arithmetic_specifier *s = llist->content;
                            if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                                errorh_np(line, column);
                                fprintf(stderr, "incompatible types when initializing type '%s' using type '%s'\n",
                                    type_list_to_str(llist), type_list_to_str(rlist));
                                llist = 0;
                            }
                            else {
                                llist->type = constant_t;
                                llist = llist;
                            }
                        }
                        else {
                            llist->type = constant_t;
                            llist = llist;
                        }
                    }
                    if (ptr1) {
                        errorh_np(line, column);
                        fprintf(stderr, "too few arguments to function call\n");
                        llist = 0;
                    }
                    else {
                        list *ptr = llist;
                        while (ptr->next)
                            ptr = ptr->next;
                        llist = list_init(ptr->content);
                        llist->type = constant_t;
                    }
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "called object type '%s' is not a function or function pointer\n",
                                  type_list_to_str(llist));
                    llist = 0;
                }
            }
            else
                llist = 0;
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
                else {
                    ulist = type_list_copy(ulist);
                    ulist->type = constant_t;
                }
            }
        }
    }
    else if (!strcmp(token, "&")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            if (ulist->type == constant_t) {
                errorh_np(line, column);
                fprintf(stderr, "cannot take the address of an rvalue of type '%s'\n",
                    type_list_to_str(ulist));
                ulist = 0;
            }
            else {
                list *newlist = list_init(pointer_init());
                newlist->next = type_list_copy(ulist);
                newlist->next->prev = newlist;
                newlist->type = constant_t;
                ulist = newlist;
            }
        }
    }
    else if (!strcmp(token, "*")) {
        ulist = syntax_cast(stream, namespace);
        if (ulist) {
            int t = type(ulist->content);
            if (t == pointer_t || t == array_t)
                ulist = ulist->next;
            else if (t == function_t)
                ;
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
                ulist->type = constant_t;
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
                    ulist->type = constant_t;
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
                ulist = list_init(arithmetic_specifier_init(int_t));
                ulist->type = constant_t;
            }
            else {
                ulist = 0;
                error_message_np(line, column, "wrong type argument to unary exclamation mark");
            }
        }
    }
    else if (!strcmp(token, "sizeof")) {
        ulist = list_init(arithmetic_specifier_init(int_t));
        ulist->type = constant_t;
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
    int offset = ftell(stream);
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
                        int lt = type(llist->content);
                        int rt = type(rlist->content);
                        if (rt != arithmetic_specifier_t && rt != pointer_t && rt != array_t && rt != function_t) {
                            error_message_np(line2, column2, "arithmetic or pointer type is required");
                            return 0;
                        }
                        else if ((rt == pointer_t || rt == array_t || rt == function_t) && lt == arithmetic_specifier_t) {
                            arithmetic_specifier *s = llist->content;
                            if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                                errorh_np(line2, column2);
                                fprintf(stderr, "pointer cannot be cast to type '%s'\n", s->atype == float_t ? "float" :
                                            s->atype == double_t ? "double" : "long double");
                                return 0;
                            }
                        }
                        else {
                            llist->type = constant_t;
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
            fseek(stream, offset, SEEK_SET);
            file_info.line = line;
            file_info.column = column;
            return syntax_unary(stream, namespace);
        }
    }
    else {
        unscan(token, stream);
        return syntax_unary(stream, namespace);
    }
}

static inline int int_promotionp(arithmetic_specifier *s)
{
    return s->atype == char_t || s->atype == unsigned_char_t ||
               s->atype == short_t || s->atype == unsigned_short_t;
}

static void *arith_conv(arithmetic_specifier *ls, arithmetic_specifier *rs)
{
    if (ls->atype == long_double_t || rs->atype == long_double_t)
        return arithmetic_specifier_init(long_double_t);
    else if (ls->atype == double_t || rs->atype == double_t)
        return arithmetic_specifier_init(double_t);
    else if (ls->atype == float_t || rs->atype == float_t)
        return arithmetic_specifier_init(float_t);
    else if (ls->atype == unsigned_long_long_t || rs->atype == unsigned_long_long_t)
        return arithmetic_specifier_init(unsigned_long_long_t);
    else if (ls->atype == long_long_t || rs->atype == long_long_t)
        return arithmetic_specifier_init(long_long_t);
    else if (ls->atype == unsigned_long_t || rs->atype == unsigned_long_t)
        return arithmetic_specifier_init(unsigned_long_t);
    else if (ls->atype == long_t || rs->atype == long_t)
        return arithmetic_specifier_init(long_t);
    else if (ls->atype == unsigned_int_t || rs->atype == unsigned_int_t)
        return arithmetic_specifier_init(unsigned_int_t);
    else
        return arithmetic_specifier_init(int_t);
}

list *syntax_m_expr(FILE *stream, namespace_t *namespace)
{
    list *llist = syntax_cast(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%")) {
            list *rlist = syntax_cast(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == arithmetic_specifier_t && rt == arithmetic_specifier_t) {
                    arithmetic_specifier *ls = llist->content, *rs = rlist->content;
                    if (!strcmp(token, "%")) {
                        if (at_integral(ls->atype) && at_integral(rs->atype)) {
                            llist->content = arith_conv(ls, rs);
                            llist->type = constant_t;
                        }
                        else {
                            llist = 0;
                            error_message_np(line, column, "invalid operands to binary %");
                        }
                    }
                    else {
                        llist->content = arith_conv(ls, rs);
                        llist->type = constant_t;
                    }
                }
                else {
                    llist = 0;
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary %c\n", !strcmp(token, "*") ? '*' : '/');
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

int type_list_eq(list *left, list *right)
{
    list *ptr1 = left, *ptr2 = right;
    if ((type(ptr1->content) == pointer_t || type(ptr1->content) == array_t) &&
             (type(ptr2->content) == pointer_t || type(ptr2->content) == array_t)) {
        ptr1 = ptr1->next;
        ptr2 = ptr2->next;
    }
    else if (type(ptr1->content) == function_t &&
                  ((type(ptr2->content) == pointer_t || type(ptr2->content) == array_t) &&
                          type(ptr2->next->content) == function_t))
        ptr2 = ptr2->next;
    else if (type(ptr2->content) == function_t &&
                  ((type(ptr1->content) == pointer_t || type(ptr1->content) == array_t) &&
                          type(ptr1->next->content) == function_t))
        ptr1 = ptr1->next;

    for ( ; ptr1 && ptr2; ptr1 = ptr1->next, ptr2 = ptr2->next) {
        if (type(ptr1->content) != type(ptr2->content))
            return 0;
        else if (type(ptr1->content) == struct_specifier_t) {
            struct_specifier *s1 = ptr1->content, *s2 = ptr2->content;
            if (!strcmp(s1->id, s2->id))
                return 1;
            else
                return 0;
        }
        else if (type(ptr1->content) == union_specifier_t) {
            union_specifier *s1 = ptr1->content, *s2 = ptr2->content;
            if (!strcmp(s1->id, s2->id))
                return 1;
            else
                return 0;
        }
        else if (type(ptr2->content) == enum_type) {
            enum_t *s1 = ptr1->content, *s2 = ptr2->content;
            if (!strcmp(s1->id, s2->id))
                return 1;
            else
                return 0;
        }
        else if (type(ptr2->content) == arithmetic_specifier_t) {
            arithmetic_specifier *s1 = ptr1->content, *s2 = ptr2->content;
            if (s1->atype == s2->atype)
                return 1;
            else
                return 0;
        }
    }
}

static char *arithmetic_specifier_to_str(arithmetic_specifier *specifier)
{
    switch (specifier->atype) {
        case char_t: return "char";
        case unsigned_char_t: return "unsigned char";
        case short_t: return "short";
        case unsigned_short_t: return "unsigned short";
        case int_t: return "int";
        case unsigned_int_t: return "unsigned int";
        case long_t: return "long";
        case unsigned_long_t: return "unsigned long";
        case long_long_t: return "long long";
        case unsigned_long_long_t: return "unsigned long long";
        case float_t: return "float";
        case double_t: return "double";
        case long_double_t: return "long double";
    }
}

char *type_list_to_str_nopg(list *type_list)
{
    list *ptr = type_list;
    int t, last_type;
    buffer *buff = buff_init();
    // if (type(ptr->content) == array_t || type(ptr->content) == pointer_t) {
    //     buff_add(buff, "*");
    //     ptr = ptr->next;
    //     last_type = pointer_t;
    // }
    // else if (type(ptr->content) == function_t) {
    //     list *parameter_list = ((function *)ptr->content)->parameter_list;
    //     buff_add(buff, "(*)(");
    //     list *ptr2 = parameter_list->next;
    //     if (ptr2) {
    //         for ( ; ptr2->next; ptr2 = ptr2->next) {
    //             buff_add(buff, type_list_to_str(((declaration *)ptr2->content)->type_list));
    //             buff_add(buff, ", ");
    //         }
    //         buff_add(buff, type_list_to_str(((declaration *)ptr2->content)->type_list));
    //     }
    //     buff_add(buff, ")");
    //     ptr = ptr->next;
    // }

    for ( ; ptr; ptr = ptr->next) {
        t = type(ptr->content);
        if (t == pointer_t) {
            buffer *new_buff = buff_init();
            buff_add(new_buff, "*");
            buff_add(new_buff, buff_puts(buff));
            buff = new_buff;
        }
        else if (type(ptr->content) == array_t) {
            if (last_type == pointer_t) {
                buffer *new_buff = buff_init();
                buff_add(new_buff, "(");
                buff_add(new_buff, buff_puts(buff));
                buff_add(new_buff, ")[]");
                buff = new_buff;
            }
            else
                buff_add(buff, "[]");
        }
        else if (type(ptr->content) == function_t) {
            if (last_type == pointer_t) {
                buffer *new_buff = buff_init();
                buff_add(new_buff, "(");
                buff_add(new_buff, buff_puts(buff));
                buff_add(new_buff, ")(");
                list *ptr2 = ((function *)ptr->content)->parameter_list->next;
                if (ptr2) {
                    for ( ; ptr2->next; ptr2 = ptr2->next) {
                        buff_add(new_buff, type_list_to_str(((declaration *)ptr2->content)->type_list));
                        buff_add(new_buff, ", ");
                    }
                    buff_add(new_buff, type_list_to_str(((declaration *)ptr2->content)->type_list));
                }
                buff_add(new_buff, ")");
                buff = new_buff;
            }
            else
                buff_add(buff, "()");
        }
        else if (type(ptr->content) == arithmetic_specifier_t) {
            buffer *new_buff = buff_init();
            buff_add(new_buff, arithmetic_specifier_to_str(ptr->content));
            if (ptr != type_list)
                buff_add(new_buff, " ");
            buff_add(new_buff, buff_puts(buff));
            buff = new_buff;
        }
        else if (type(ptr->content) == struct_specifier_t) {
            buffer *new_buff = buff_init();
            buff_add(new_buff, "struct ");
            buff_add(new_buff, ((struct_specifier *)ptr->content)->id);
            if (ptr != type_list)
                buff_add(new_buff, " ");
            buff_add(new_buff, buff_puts(buff));
            buff = new_buff;
        }
        else if (type(ptr->content) == union_specifier_t) {
            buffer *new_buff = buff_init();
            buff_add(new_buff, "union ");
            buff_add(new_buff, ((union_specifier *)ptr->content)->id);
            if (ptr != type_list)
                buff_add(new_buff, " ");
            buff_add(new_buff, buff_puts(buff));
            buff = new_buff;
        }
        else if (type(ptr->content) == enum_type) {
            buffer *new_buff = buff_init();
            buff_add(new_buff, "enum ");
            buff_add(new_buff, ((enum_t *)ptr->content)->id);
            if (ptr != type_list)
                buff_add(new_buff, " ");
            buff_add(new_buff, buff_puts(buff));
            buff = new_buff;
        }
        last_type = t;
    }
    return buff_puts(buff);
}

char *type_list_to_str(list *type_list) {
    char *retptr;
    if (type(type_list->content) == array_t) {
        array *old = type_list->content;
        type_list->content = pointer_init();
        retptr = type_list_to_str_nopg(type_list);
        type_list->content = old;
    }
    else if (type(type_list->content) == function_t) {
        type_list->prev = list_init(pointer_init());
        type_list->prev->next = type_list;
        type_list = type_list->prev;
        retptr = type_list_to_str_nopg(type_list);
        type_list = type_list->next;
        type_list->prev = 0;
    }
    else
        retptr = type_list_to_str_nopg(type_list);
    return retptr;
}

list *syntax_a_expr(FILE *stream, namespace_t *namespace)
{
    list *llist = syntax_m_expr(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-")) {
            list *rlist = syntax_m_expr(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == arithmetic_specifier_t && rt == arithmetic_specifier_t) {
                    arithmetic_specifier *ls = llist->content, *rs = rlist->content;
                    llist->content = arith_conv(ls, rs);
                    llist->type = constant_t;
                }
                else if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                    arithmetic_specifier *ls = llist->content;
                    if (at_integral(ls->atype)) {
                        llist = rlist;
                        llist->type = constant_t;
                    }
                    else {
                        llist = 0;
                        error_message_np(line, column, "invalid operands to binary +/-");
                    }
                }
                else if ((lt == pointer_t || lt == array_t || lt == function_t) && rt == arithmetic_specifier_t) {
                    arithmetic_specifier *rs = rlist->content;
                    if (at_integral(rs->atype))
                        llist->type = constant_t;
                    else {
                        llist = 0;
                        error_message_np(line, column, "invalid operands to binary +/-");
                    }
                }
                else if (!strcmp(token, "-") && (lt == pointer_t || lt == array_t || lt == function_t)
                                                       && (rt == pointer_t || rt == array_t || rt == function_t)) {
                    if (type_list_eq(llist, rlist))
                        llist->type = constant_t;
                    else {
                        errorh_np(line, column);
                        fprintf(stderr, "invalid operands to binary - (have '%s' and '%s')\n",
                                     type_list_to_str(llist), type_list_to_str(rlist));
                        llist = 0;
                    }
                }
                else {
                    llist = 0;
                    error_message_np(line, column, "invalid operands to binary +/-");
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_shift(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_a_expr(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "<<") || !strcmp(token, ">>")) {
            list *rlist = syntax_a_expr(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == arithmetic_specifier_t && rt == arithmetic_specifier_t) {
                    arithmetic_specifier *ls = llist->content, *rs = rlist->content;
                    if (at_integral(ls->atype) && at_integral(rs->atype)) {
                        llist->content = arith_conv(ls, rs);
                        llist->type = constant_t;
                        llist->next = 0;
                    }
                    else {
                        errorh_np(line, column);
                        fprintf(stderr, "invalid operands to binary %s (hava '%s' and '%s'\n",
                                     !strcmp(token, "<<") ? "<<" : ">>", type_list_to_str(llist), type_list_to_str(rlist));
                        llist = 0;
                    }
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary %s (hava '%s' and '%s'\n",
                                 !strcmp(token, "<<") ? "<<" : ">>", type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_relational(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_shift(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") ||
                  !strcmp(token, "<=") || !strcmp(token, ">=")) {
            list *rlist = syntax_shift(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == arithmetic_specifier_t && rt == arithmetic_specifier_t) {
                    llist->type = constant_t;
                    llist->content = arithmetic_specifier_init(int_t);
                    llist->next = 0;
                }
                else if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                    if (at_integral(((arithmetic_specifier *)llist->content)->atype)) {
                        llist->type = constant_t;
                        llist->content = arithmetic_specifier_init(int_t);
                        llist->next = 0;
                    }
                    else {
                        errorh_np(line, column);
                        fprintf(stderr, "invalid operands to binary %s (have '%s' and '%s')\n",
                                      !strcmp(token, "<") ? "<" : !strcmp(token, ">") ? ">" :
                                      !strcmp(token, "<=") ? "<=" : ">=", type_list_to_str(llist), type_list_to_str(rlist));
                        llist = 0;
                    }
                }
                else if (rt == arithmetic_specifier_t && (lt == pointer_t || lt == array_t || lt == function_t)) {
                    if (at_integral(((arithmetic_specifier *)rlist->content)->atype)) {
                        llist->type = constant_t;
                        llist->content = arithmetic_specifier_init(int_t);
                        llist->next = 0;
                    }
                    else {
                        errorh_np(line, column);
                        fprintf(stderr, "invalid operands to binary %s (have '%s' and '%s')\n",
                                      !strcmp(token, "<") ? "<" : !strcmp(token, ">") ? ">" :
                                      !strcmp(token, "<=") ? "<=" : ">=", type_list_to_str(llist), type_list_to_str(rlist));
                        llist = 0;
                    }
                }
                else if ((lt == pointer_t || lt == array_t || lt == function_t) &&
                               (rt == pointer_t || rt == array_t || rt == function_t)) {
                    llist->type = constant_t;
                    llist->content = arithmetic_specifier_init(int_t);
                    llist->next = 0;
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary %s (have '%s' and '%s')\n",
                                  !strcmp(token, "<") ? "<" : !strcmp(token, ">") ? ">" :
                                  !strcmp(token, "<=") ? "<=" : ">=", type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_equality(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_relational(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!=")) {
            list *rlist = syntax_relational(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == arithmetic_specifier_t && rt == arithmetic_specifier_t) {
                    llist->type = constant_t;
                    llist->content = arithmetic_specifier_init(int_t);
                    llist->next = 0;
                }
                else if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                    if (at_integral(((arithmetic_specifier *)llist->content)->atype)) {
                        llist->type = constant_t;
                        llist->content = arithmetic_specifier_init(int_t);
                        llist->next = 0;
                    }
                    else {
                        errorh_np(line, column);
                        fprintf(stderr, "invalid operands to binary %s (have '%s' and '%s')\n",
                                      !strcmp(token, "==") ? "==" : "!=", type_list_to_str(llist), type_list_to_str(rlist));
                        llist = 0;
                    }
                }
                else if (rt == arithmetic_specifier_t && (lt == pointer_t || lt == array_t || lt == function_t)) {
                    if (at_integral(((arithmetic_specifier *)rlist->content)->atype)) {
                        llist->type = constant_t;
                        llist->content = arithmetic_specifier_init(int_t);
                        llist->next = 0;
                    }
                    else {
                        errorh_np(line, column);
                        fprintf(stderr, "invalid operands to binary %s (have '%s' and '%s')\n",
                                      !strcmp(token, "==") ? "==" : "!=", type_list_to_str(llist), type_list_to_str(rlist));
                        llist = 0;
                    }
                }
                else if ((lt == pointer_t || lt == array_t || lt == function_t) &&
                               (rt == pointer_t || rt == array_t || rt == function_t)) {
                    llist->type = constant_t;
                    llist->content = arithmetic_specifier_init(int_t);
                    llist->next = 0;
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary %s (have '%s' and '%s')\n",
                                  !strcmp(token, "==") ? "==" : "!=", type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_and(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_equality(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "&")) {
            list *rlist = syntax_equality(stream, namespace);
            if (llist && rlist) {
                if ((type(llist->content) == arithmetic_specifier_t && type(rlist->content) == arithmetic_specifier_t) &&
                          at_integral(((arithmetic_specifier *)llist->content)->atype) &&
                                at_integral(((arithmetic_specifier *)rlist->content)->atype)) {
                    llist->content = arith_conv(llist->content, rlist->content);
                    llist->type = constant_t;
                    llist->next = 0;
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary & (have '%s' and '%s')\n",
                        type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_xor(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_and(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "^")) {
            list *rlist = syntax_and(stream, namespace);
            if (llist && rlist) {
                if ((type(llist->content) == arithmetic_specifier_t && type(rlist->content) == arithmetic_specifier_t) &&
                          at_integral(((arithmetic_specifier *)llist->content)->atype) &&
                                at_integral(((arithmetic_specifier *)rlist->content)->atype)) {
                    llist->content = arith_conv(llist->content, rlist->content);
                    llist->type = constant_t;
                    llist->next = 0;
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary & (have '%s' and '%s')\n",
                        type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_or(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_xor(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "|")) {
            list *rlist = syntax_xor(stream, namespace);
            if (llist && rlist) {
                if ((type(llist->content) == arithmetic_specifier_t && type(rlist->content) == arithmetic_specifier_t) &&
                          at_integral(((arithmetic_specifier *)llist->content)->atype) &&
                                at_integral(((arithmetic_specifier *)rlist->content)->atype)) {
                    llist->content = arith_conv(llist->content, rlist->content);
                    llist->type = constant_t;
                    llist->next = 0;
                }
                else {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary & (have '%s' and '%s')\n",
                        type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_logical_and(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_or(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "&&")) {
            list *rlist = syntax_or(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == struct_specifier_t || lt == union_specifier_t ||
                          rt == struct_specifier_t || rt == union_specifier_t) {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary && (have '%s' and '%s')\n",
                        type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
                else {
                    llist->content = arithmetic_specifier_init(int_t);
                    llist->next = 0;
                    llist->type = constant_t;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_logical_or(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_logical_and(stream, namespace);
    while (1) {
        int line = file_info.line;
        int column = file_info.column;
        char *token = scan(stream);
        if (!strcmp(token, "||")) {
            list *rlist = syntax_logical_and(stream, namespace);
            if (llist && rlist) {
                int lt = type(llist->content);
                int rt = type(rlist->content);
                if (lt == struct_specifier_t || lt == union_specifier_t ||
                          rt == struct_specifier_t || rt == union_specifier_t) {
                    errorh_np(line, column);
                    fprintf(stderr, "invalid operands to binary || (have '%s' and '%s')\n",
                        type_list_to_str(llist), type_list_to_str(rlist));
                    llist = 0;
                }
                else {
                    llist->content = arithmetic_specifier_init(int_t);
                    llist->next = 0;
                    llist->type = constant_t;
                }
            }
            else
                llist = 0;
        }
        else {
            unscan(token, stream);
            return llist;
        }
    }
}

list *syntax_conditional(FILE *stream, namespace_t *namespace) {
    int line1 = file_info.line;
    int column1 = file_info.column;
    list *llist = syntax_logical_or(stream, namespace);
    int line2 = file_info.line;
    int column2 = file_info.column;
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        list *mlist = syntax_expression(stream, namespace);
        token = scan(stream);
        if (strcmp(token, ":")) {
            unscan(token, stream);
            error_message("expected ':'");
        }
        list *rlist = syntax_conditional(stream, namespace);
        if (llist && mlist && rlist) {
            int lt = type(llist->content), mt = type(mlist->content), rt = type(rlist->content);
            if (lt == struct_specifier_t || lt == union_specifier_t) {
                errorh_np(line1, column1);
                fprintf(stderr, "use type '%s' where arithmetic, pointer, or vector type is required\n",
                             type_list_to_str(llist));
                return 0;
            }
            else if (mt != rt && (mt == struct_specifier_t || mt == union_specifier_t ||
                          rt == struct_specifier_t || rt == union_specifier_t)) {
                errorh_np(line2, column2);
                fprintf(stderr, "incompatible operand types ('%s' and '%s')\n",
                             type_list_to_str(mlist), type_list_to_str(rlist));
                return 0;
            }
            else if (mt == arithmetic_specifier_t && rt == arithmetic_specifier_t) {
                list *retptr = list_init(arith_conv(llist->content, rlist->content));
                retptr->type = constant_t;
                retptr->next = 0;
                return retptr;
            }
            else if (mt == pointer_t || mt == array_t || mt == function_t)
                return mlist;
            else
                return rlist;
        }
        else
            return 0;
    }
    else {
        unscan(token, stream);
        return llist;
    }
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

list *syntax_assignment(FILE *stream, namespace_t *namespace) {
    list *llist = syntax_conditional(stream, namespace);
    int line = file_info.line;
    int column = file_info.column;
    char *token = scan(stream);
    if (is_assignment_operator(token)) {
        list *rlist = syntax_assignment(stream, namespace);
        if (llist && rlist) {
            if (llist->type == constant_t || llist->type == function_t) {
                error_message_np(line, column, "expression is not assignable");
                return 0;
            }
            else {
                int lt = type(llist->content), rt = type(rlist->content);
                if (lt == array_t) {
                    errorh_np(line, column);
                    fprintf(stderr, "array type '%s' is not assignable\n",
                        type_list_to_str_nopg(llist));
                    return 0;
                }
                else if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                    arithmetic_specifier *s = llist->content;
                    if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                        errorh_np(line, column);
                        fprintf(stderr, "assigning to '%s' from incompatible type '%s'\n",
                            type_list_to_str(llist), type_list_to_str(rlist));
                        return 0;
                    }
                    else {
                        llist->type = constant_t;
                        return llist;
                    }
                }
                else if (rt == arithmetic_specifier_t && lt == pointer_t) {
                    arithmetic_specifier *s = rlist->content;
                    if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                        errorh_np(line, column);
                        fprintf(stderr, "assigning to '%s' from incompatible type '%s'\n",
                            type_list_to_str(llist), type_list_to_str(rlist));
                        return 0;
                    }
                    else {
                        llist->type = constant_t;
                        return llist;
                    }
                }
                else if ((lt == struct_specifier_t || lt == union_specifier_t) && !type_list_eq(llist, rlist)) {
                    errorh_np(line, column);
                    fprintf(stderr, "assigning to '%s' from incompatible type '%s'\n",
                        type_list_to_str(llist), type_list_to_str(rlist));
                    return 0;
                }
                else {
                    llist->type = constant_t;
                    return llist;
                }
            }
        }
        else
            return 0;
    }
    else {
        unscan(token, stream);
        return llist;
    }
}

list *syntax_expression(FILE *stream, namespace_t *namespace) {
    while (1) {
        list *llist = syntax_assignment(stream, namespace);
        char *token = scan(stream);
        if (!strcmp(token, ","))
            continue;
        else {
            unscan(token, stream);
            return llist;
        }
    }
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
        syntax_compound_stmt(stream, namespace, 0, 0);
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
    if (is_specifier(token) || is_storage(token) || is_qualifier(token) || find_typedef_cn(namespace, token) ||
              (!find_declaration_cn(namespace, token) && find_typedef(namespace->outer, token)))
        syntax_declaration(stream, namespace, 0);
    else
        syntax_statement(stream, namespace);
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

void syntax_compound_stmt(FILE *stream, namespace_t *namespace, list *parameter_list, int is_func)
{
    char *token = scan(stream);  // should be "{"
    jmp_buf env;
    goto t2; t1: end_stack_push("}", &env);
    struct namespace *i_namespace = namespace_init(namespace);
    if (is_func) {
        i_namespace->is_func_ns = 1;
        i_namespace->labels = list_node();
        if (parameter_list)
            i_namespace->declaration_list = parameter_list;
    }
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "}")) {
            end_stack_pop();
            return;
        }
        unscan(token, stream);
        syntax_declaration_or_statement(stream, i_namespace);
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
    int line = file_info.line;
    int column = file_info.column;
    list *llist = syntax_expression(stream, namespace);
    if (llist && (type(llist->content) == struct_specifier_t || type(llist->content) == union_specifier_t))
        error_message_np(line, column, "scalar is required");
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
    int line = file_info.line;
    int column = file_info.column;
    list *llist = syntax_expression(stream, namespace);
    if (llist && !(type(llist->content) == arithmetic_specifier_t &&
              at_integral(((arithmetic_specifier *)llist->content)->atype)))
        error_message_np(line, column, "switch quantity not an integer");
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    in_switch++;
    syntax_statement(stream, namespace);
    in_switch--;
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_case_stmt(FILE *stream, namespace_t *namespace)
{
    if (!in_switch)
        error_message("'case' statement not in switch statement");
    char *token = scan(stream);  // should be "case"
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    int line = file_info.line;
    int column = file_info.column;
    list *llist = syntax_conditional(stream, namespace);
    if (llist && !(type(llist->content) == arithmetic_specifier_t &&
              at_integral(((arithmetic_specifier *)llist->content)->atype)))
        error_message_np(line, column, "case label does not reduce to an integer constant");
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

static int find_label(namespace_t *namespace, char *id) {
    namespace_t *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        if (nptr->is_func_ns) {
            for (ptr = nptr->labels->next; ptr; ptr = ptr->next)
                if (!strcmp(ptr->content, id))
                    return 1;
        }
    }
    return 0;
}

static void add_label(namespace_t *namespace, char *id) {
    namespace_t *nptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        if (nptr->is_func_ns)
            list_append(nptr->labels, id);
    }
}

void syntax_id_labeled_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (find_label(namespace, token)) {
        unscan(token, stream);
        errorh();
        fprintf(stderr, "redefinition of label '%s'\n", token);
        scan(stream);
    }
    add_label(namespace, token);
    token = scan(stream);
    token = scan(stream);
    unscan(token, stream);
    if (is_specifier(token) || is_storage(token) || is_qualifier(token) || find_typedef_cn(namespace, token) ||
              (!find_declaration_cn(namespace, token) && find_typedef(namespace->outer, token))) {
        error_message("expected expression");
        syntax_declaration(stream, namespace, 0);
    }
    else
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
    int line = file_info.line;
    int column = file_info.column;
    list *llist = syntax_expression(stream, namespace);
    if (llist && (type(llist->content) == struct_specifier_t || type(llist->content) == union_specifier_t))
        error_message_np(line, column, "scalar is required");
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    in_loop++;
    syntax_statement(stream, namespace);
    in_loop--;
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_do_while_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "do"
    in_loop++;
    syntax_statement(stream, namespace);
    in_loop--;
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
    int line = file_info.line;
    int column = file_info.column;
    list *llist = syntax_expression(stream, namespace);
    if (llist && (type(llist->content) == struct_specifier_t || type(llist->content) == union_specifier_t))
        error_message_np(line, column, "scalar is required");
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
            if (i == 1) {
                int line = file_info.line;
                int column = file_info.column;
                list *llist = syntax_expression(stream, namespace);
                if (llist && (type(llist->content) == struct_specifier_t || type(llist->content) == union_specifier_t))
                    error_message_np(line, column, "scalar is required");
            }
            else
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
    t3: 
    in_loop++;
    syntax_statement(stream, namespace);
    in_loop--;
}

void syntax_goto_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "goto"
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    token = scan(stream);
    if (!is_id(token)) {
        unscan(token, stream);
        error(stream, "expected identifier");
    }
    char *id = token;
    if (!find_label(namespace, id)) {
        unscan(token, stream);
        errorh();
        fprintf(stderr, "use of undeclared label '%s'\n", token);
        scan(stream);
    }
    token = scan(stream);
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
    if (!in_loop)
        error_message("'continue' statement not in loop statement");
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
    if (!in_loop && !in_switch)
        error_message("'break' statement not in loop or switch statement");
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
    if (!strcmp(token, ";")) {
    }
    else {
        unscan(token, stream);
        jmp_buf env;
        goto t2; t1: end_stack_push(";", &env);
        int line = file_info.line;
        int column = file_info.column;
        list *llist = return_type_list;
        list *rlist = syntax_expression(stream, namespace);
        if (llist && rlist) {
            int lt = type(llist->content), rt = type(rlist->content);
            if (lt == arithmetic_specifier_t && (rt == pointer_t || rt == array_t || rt == function_t)) {
                arithmetic_specifier *s = llist->content;
                if (s->atype == float_t || s->atype == double_t || s->atype == long_double_t) {
                    errorh_np(line, column);
                    fprintf(stderr, "returning '%s' from a function with incompatible result type '%s'\n",
                        type_list_to_str(rlist), type_list_to_str(llist));
                }
            }
            else if ((lt == struct_specifier_t || lt == union_specifier_t) && !type_list_eq(llist, rlist)) {
                errorh_np(line, column);
                fprintf(stderr, "returning '%s' from a function with incompatible result type '%s'\n",
                    type_list_to_str(rlist), type_list_to_str(llist));
            }
        }
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
        storage = static_storage_init(-1, -1);
    ((static_storage *)storage)->initialized = -1;
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    int first_loop = 1;
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        declarator_info_t *dptr = syntax_declarator(stream, namespace, 0);
        int line = dptr->line;
        int column = dptr->column;
        list *type_list = dptr->type_list;
        list *ptr = type_list;
        while (ptr->next)
            ptr = ptr->next;
        if (type(specifier) == list_t) {
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

        declaration *node;
        func *func_;

        int defined = 0;

        if (id) {
            void *val = find_identifier(namespace, id);
            if (val) {
                if (type(val) == typedef_type) {
                    errorh_np(line, column);
                    fprintf(stderr, "'%s' redeclared as different kind of symbol\n", id);
                }
                else if (!type_list_eq(get_type_list(val), type_list)) {
                    errorh_np(line, column);
                    fprintf(stderr, "conflicting types for '%s'\n", id);
                }
                else if (type(val) == func_t) {
                    if (((func *)val)->tag)
                        defined = 1;
                    else
                        defined = 0;
                }
                else if (type(val) == declaration_t) {
                    static_storage *s = ((declaration *)val)->storage;
                    if (s->initialized != -1)
                        defined = 1;
                    else
                        defined = 0;
                }
            }
            else if (type(storage) == typedef_storage_t)
                list_append(namespace->typedefs, typedef_init(id, type_list));
            else if (type(type_list->content) == function_t) {
                func_ = func_init(id, 0, type_list);
                list_append(func_list, func_);
            }
            else {
                node = declaration_init(id, storage, type_list);
                list_append(namespace->declaration_list, node);
            }
        }
        token = scan(stream);
        if (!strcmp(token, "{")) {
            if (defined) {
                errorh_np(line, column);
                fprintf(stderr, "redefinition of '%s'\n", id);
            }
            else
                func_->tag = strdup("x");
            unscan(token, stream);
            if (type(type_list->content) != function_t)
                error(stream, "expected ';' after top level declarator");
            if (!first_loop)
                error(stream, 0);
            end_stack_pop();
            function *f = type_list->content;
            return_type_list = type_list->next;
            list *parameter_list;
            if (f->parameter_list)
                parameter_list = list_copy(f->parameter_list);
            else
                parameter_list = 0;
            syntax_compound_stmt(stream, namespace, parameter_list, 1);
            return;
        }
        else if (!strcmp(token, ","))
            ;
        else if (!strcmp(token, "=")) {
            if (defined) {
                errorh_np(line, column);
                fprintf(stderr, "redefinition of '%s'\n", id);
            }
            else
                ((static_storage *)node->storage)->initialized = 1;
            list *initializer = syntax_initializer(stream, namespace);
            syntax_initializing(type_list, initializer);
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
    in_loop = in_switch = 0;
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
