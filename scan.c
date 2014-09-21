#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "barec.h"

#define write_code(CODE, STREAM) fwrite(CODE, 1, strlen(CODE), STREAM)

list *func_list;

static char f_fgetc(FILE *stream)
{
    char retval = fgetc(stream);
    if (retval == '\n') {
        file_info.line++;
        file_info.lastline_column = file_info.column;
        file_info.column = 1;
    }
    else
        file_info.column++;
    return retval;
}

void f_ungetc(char ch, FILE *stream)
{
    ungetc(ch, stream);
    if (ch == '\n') {
        file_info.column = file_info.lastline_column;
        file_info.line--; 
    }
    else
        file_info.column--;
}

void errorh()
{
    fprintf(stderr, "%s:%d:%d: error: ", file_info.file_name, file_info.line, file_info.column);
    file_info.error = 1;
}

char *scan(FILE *stream)
{
    char *buffer = (char *)malloc(1024);
    if (file_info.eof) {
        buffer[0] = 0;
        return buffer;
    }
    int ch, i = 0;
    int in_old_comment = 0;
    int in_new_comment = 0;
    while (1) {
        while (in_old_comment || in_new_comment) {
            ch = f_fgetc(stream);
            if (in_old_comment && ch == '*') {
                ch = f_fgetc(stream);
                if (ch == '/')
                    in_old_comment = 0;
            }
            else if (in_new_comment && ch == '\n')
                in_new_comment = 0;
        }
        ch = f_fgetc(stream);
        if (ch == EOF) {
            file_info.column--;
            file_info.eof = 1;
            break;
        }
        if (ch == '>' || ch == '<') {
            buffer[0] = ch;
            ch = f_fgetc(stream);
            if (ch == buffer[0]) {
                buffer[1] = ch;
                ch = f_fgetc(stream);
                if (ch == '=') {
                    buffer[2] = ch;
                    buffer[3] = 0;
                    return buffer;
                }
                else {
                    buffer[2] = 0;
                    f_ungetc(ch, stream);
                    return buffer;
                }
            }
            else if (ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                f_ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '+' || ch == '&' || ch == '|') {
            buffer[0] = ch;
            ch = f_fgetc(stream);
            if (ch == buffer[0] || ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                f_ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '-') {
            buffer[0] = ch;
            ch = f_fgetc(stream);
            if (ch == buffer[0] || ch == '=' || ch == '>') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                f_ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '=' || ch == '!' || ch == '*' || ch == '^'
                     || ch == '/' || ch == '%') {
            if (ch == '/') {
                int ch2 = f_fgetc(stream);
                if (ch2 == '*') {
                    in_old_comment = 1;
                    continue;
                }
                else if (ch2 == '/') {
                    in_new_comment = 1;
                    continue;
                }
                else
                    f_ungetc(ch2, stream);
            }
            buffer[0] = ch;
            ch = f_fgetc(stream);
            if (ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                f_ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
                     ch == ',' || ch == ';' || ch == '?' || ch == ':' || ch == '#') {
            buffer[0] = ch;
            buffer[1] = 0;
            return buffer;
        }
        else if (ch == '.') {
            ch = f_fgetc(stream);
            if (ch >= '0' && ch <= '9') {
                buffer[0] = '.';
                buffer[1] = ch;
                i = 2;
                while (1) {
                    ch = f_fgetc(stream);
                    if (ch >= '0' && ch <= '9')
                        buffer[i++] = ch;
                    else {
                        f_ungetc(ch, stream);
                        break;
                    }
                }
                buffer[i] = 0;
                return buffer;
            }
            else {
                f_ungetc(ch, stream);
                buffer[0] = '.';
                buffer[1] = 0;
                return buffer;
            }
        }
        else if (ch >= '0' && ch <= '9') {
            i = 0;
            buffer[i++] = ch;
            int point = 0;
            while (1) {
                ch = f_fgetc(stream);
                if (ch >= '0' && ch <= '9')
                    buffer[i++] = ch;
                else if (ch == '.') {
                    if (point == 0) {
                        point = 1;
                        buffer[i++] = ch;
                    }
                }
                else {
                    f_ungetc(ch, stream);
                    break;
                }
            }
            buffer[i] = 0;
            return buffer;
        }
        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
            i = 0;
            buffer[i++] = ch;
            while (1) {
                ch = f_fgetc(stream);
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z' || ch == '_') ||
                         (ch >= '0' && ch <= '9'))
                    buffer[i++] = ch;
                else {
                    f_ungetc(ch, stream);
                    break;
                }
            }
            buffer[i] = 0;
            return buffer;
        }
        else if (ch == '\'' || ch == '"') {
            i = 0;
            buffer[i++] = ch;
            while (1) {
                ch = f_fgetc(stream);
                buffer[i++] = ch;
                if (ch == buffer[0]) {
                    buffer[i] = 0;
                    return buffer;
                }
            }
        }
        else if (ch == ' ' || ch == '\t' || ch == '\n')
            ;
        else {
            buffer[0] = ch;
            buffer[1] = 0;
            return buffer;
        }
        // else {
        //     errorh();
        //     printf("stray '%c' in program\n", ch);
        // }
    }
    buffer[0] = 0;
    return buffer;
}

struct_specifier *find_struct_specifier(namespace_t *namespace, char *id)
{
    struct namespace *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->struct_s_list->next; ptr; ptr = ptr->next) {
            struct_specifier *specifier = ptr->content;
            if (!strcmp(id, specifier->id))
                return specifier;
        }
    }
    return 0;
}

union_specifier *find_union_specifier(namespace_t *namespace, char *id)
{
    struct namespace *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->union_s_list->next; ptr; ptr = ptr->next) {
            union_specifier *specifier = ptr->content;
            if (!strcmp(id, specifier->id))
                return specifier;
        }
    }
    return 0;
}

declaration *find_declaration(namespace_t *namespace, char *id)
{
    struct namespace *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->declaration_list->next; ptr; ptr = ptr->next) {
            declaration *d = ptr->content;
            if (!strcmp(id, d->id))
                return d;
        }
    }
    return 0;
}

void *find_identifier(namespace_t *namespace, char *id)
{
    struct namespace *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->declaration_list->next; ptr; ptr = ptr->next) {
            declaration *d = ptr->content;
            if (!strcmp(id, d->id))
                return d;
        }
    }
    for (ptr = func_list->next; ptr; ptr = ptr->next) {
        func *f = ptr->content;
        if (!strcmp(id, f->id))
            return f;
    }
    return 0;
}

typedef_t *find_typedef(namespace_t *namespace, char *id)
{
    namespace_t *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->typedefs->next; ptr; ptr = ptr->next) {
            typedef_t *t = ptr->content;
            if (!strcmp(id, t->id))
                return t;
        }
    }
    return 0;
}

enum_t *find_enum(namespace_t *namespace, char *id)
{
    namespace_t *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->enums->next; ptr; ptr = ptr->next) {
            enum_t *e = ptr->content;
            if (!strcmp(id, e->id))
                return e;
        }
    }
    return 0;
}

enumerator_t *find_enumerator(namespace_t *namespace, char *id)
{
    namespace_t *nptr;
    list *ptr;
    for (nptr = namespace; nptr; nptr = nptr->outer) {
        for (ptr = nptr->enums->next; ptr; ptr = ptr->next) {
            enum_t *e = ptr->content;
            list *eptr;
            for (eptr = e->enumerators->next; eptr; eptr = eptr->next) {
                enumerator_t *enumerator = eptr->content;
                if (!strcmp(id, enumerator->id))
                    return enumerator;
            }
        }
    }
    return 0;
}

struct namespace *namespace_init(namespace_t *outer)
{
    struct namespace *retptr = (struct namespace *)malloc(sizeof(struct namespace));
    retptr->declaration_list = list_node();
    retptr->struct_s_list = list_node();
    retptr->union_s_list = list_node();
    retptr->auto_size = size_init(1, 0, ARITHMETIC(strdup("0"), int_t));
    retptr->outer = outer;
    retptr->labels = 0;
    retptr->typedefs = list_node();
    retptr->enums = list_node();
    return retptr;
}

/* in the returned list, first is the storage specifier, the second is the type specifier */
/*
 * flag == 0 : internal
 * flag == 1 : external
 * flag == 2 : struct-declaration-list
 * flag == 3 : parameter list
 * flag == 4 : external-declaration
 */

typedef_storage *typedef_storage_init()
{
    typedef_storage *retptr = malloc(sizeof(typedef_storage));
    retptr->type = typedef_storage_t;
    return retptr;
}

enum_t *enum_init(char *id, list *enumerators)
{
    enum_t *retptr = malloc(sizeof(enum_t));
    retptr->type = enum_type;
    retptr->id = id;
    retptr->enumerators = enumerators;
    return retptr;
}

enumerator_t *enumerator_init(char *id, int value)
{
    enumerator_t *retptr = malloc(sizeof(enumerator_t));
    retptr->id = id;
    retptr->value = value;
    return retptr;
}

list *parse_enumerator_body(FILE *stream, namespace_t *namespace)
{                
    char *token = scan(stream);  // should be "{"
    list *enumerators = list_node();
    int default_value = 0;
    while (1) {
        char *eid = scan(stream);
        int value;
        token = scan(stream);
        if (!strcmp(token, "=")) {
            arithmetic *a = parse_conditional(stream, namespace);
            value = atoi(a->value);
            default_value = value + 1;
            token = scan(stream);
        }
        else
            value = default_value++;
        list_append(enumerators, enumerator_init(eid, value));
        if (!strcmp(token, ",")) {
            token = scan(stream);
            if (!strcmp(token, "}"))
                return enumerators;
            else
                unscan(token, stream);
        }
        else if (!strcmp(token, "}"))
            return enumerators;
    }
}

list *parse_specifier(FILE *stream, namespace_t *namespace)
{
    char *token;
    void *storage = 0, *specifier = 0;
    list *retptr = list_node();
    int sign = 1;
    enum atypes atype = -1;
    int void_specifier = 0;
    int got_specifier = 0;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "typedef"))
            storage = typedef_storage_init();
        else if (!strcmp(token, "auto"))
            storage = auto_storage_init(1, 0, 0);
        else if (!strcmp(token, "static")) {
            storage = static_storage_init();
        }
        else if (!strcmp(token, "void")) {
            got_specifier = 1;
            void_specifier = 1;
        }
        else if (!strcmp(token, "char")) {
            got_specifier = 1;
            atype = char_t;
        }
        else if (!strcmp(token, "short")) {
            got_specifier = 1;
            atype = short_t;
        }
        else if (!strcmp(token, "int")) {
            got_specifier = 1;
            atype = int_t;
        }
        else if (!strcmp(token, "long")) {
            got_specifier = 1;
            if (atype == long_t)
                atype = long_long_t;
            else if (atype == -1 || atype == int_t)
                atype = long_t;
        }
        else if (!strcmp(token, "float")) {
            got_specifier = 1;
            atype = float_t;
        }
        else if (!strcmp(token, "double")) {
            got_specifier = 1;
            if (atype = long_t)
                atype = long_double_t;
            else if (atype == -1)
                atype = double_t;
        }
        else if (!strcmp(token, "unsigned")) {
            sign = 0;
        }
        else if (!strcmp(token, "signed")) {
            sign = 1;
        }
        else if (!strcmp(token, "struct") || !strcmp(token, "union")) {
            got_specifier = 1;
            int is_struct = !strcmp(token, "struct");
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
                        struct namespace *s_namespace = namespace_init(namespace);
                        while (strcmp((token = scan(stream)), "}")) {
                            unscan(token, stream);
                            parse_declaration(stream, s_namespace, 1);
                        }
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
                    if (is_struct)
                        specifier = find_struct_specifier(namespace, id);
                    else
                        specifier = find_union_specifier(namespace, id);
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
                    while (strcmp((token = scan(stream)), "}")) {
                        unscan(token, stream);
                        parse_declaration(stream, s_namespace, 1);
                    }
                    if (is_struct)
                        specifier = struct_specifier_init(0, s_namespace->declaration_list);
                    else
                        specifier = union_specifier_init(0, s_namespace->declaration_list);
                }
            }
        }
        else if (!strcmp(token, "enum")) {
            got_specifier = 1;
            token = scan(stream);
            if (is_id(token)) {
                char *id = token;
                token = scan(stream);
                if (!strcmp(token, "{")) {
                    unscan(token, stream);
                    list *enumerators = parse_enumerator_body(stream, namespace);
                    specifier = enum_init(id, enumerators);
                    list_append(namespace->enums, specifier);
                }
                else {
                    unscan(token, stream);
                    specifier = find_enum(namespace, id);
                }
            }
            else {
                list *enumerators = parse_enumerator_body(stream, namespace);
                specifier = enum_init(0, enumerators);
            }
        }
        // CAUTION!!! convert enum to int before comparing
        else if (!got_specifier && sign == -1) {
            typedef_t *t = find_typedef(namespace, token);
            specifier = t->type_list;
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

parameter_storage *parameter_storage_init()
{
    parameter_storage *retptr = (parameter_storage *)malloc(sizeof(parameter_storage));
    retptr->type = parameter_storage_t;
    return retptr;
}

list *parse_parameter_list(FILE *stream, struct namespace *namespace)
{
    list *retptr = list_node();
    int abstract = 2;
    int psize = 0;
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, ")"))
        return retptr;
    while (1) {
        list *specifiers = parse_specifier(stream, namespace);
        void *storage = specifiers->next->content;
        void *specifier = specifiers->next->next->content;
        declarator *d = parse_declarator(stream, abstract, namespace);
        list *type_list = d->type_list, *ptr = type_list;
        char *id = d->id;
        while (ptr->next)
            ptr = ptr->next;
        ptr->content = specifier;
        list_append(retptr, declaration_init(id, storage, type_list));
        token = scan(stream);
        if (!strcmp(token, ")")) {
            unscan(token, stream);
            return retptr;
        }
    }
}

/* 
 * abstract == 0 : id
 * abstract == 1 : no id
 * abstract == 2 : id or no id
 */
declarator *parse_declarator(FILE *stream, int abstract, struct namespace *namespace)
{
    char *token;
    list *type_list = list_node(), *tmp;
    char *id = 0;

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

    token = scan(stream);
    if (!strcmp(token, "(")) {
        declarator *dptr = parse_declarator(stream, abstract, namespace);
        token = scan(stream);
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
    else {
        if (!abstract) {
            if (is_id(token))
                id = token;
        }
        else if (abstract == 1)
            unscan(token, stream);
        else if (abstract == 2) {
            if (is_id(token))
                id = token;
            else
                unscan(token, stream);
        }
    }

    list *new_type_list = list_node();
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            token = scan(stream);
            if (!strcmp(token, "]"))
                list_append(new_type_list, array_init(0));
            else {
                unscan(token, stream);
                list_append(new_type_list, array_init(parse_conditional(stream, namespace)));
                token = scan(stream);
            }
        }
        else if (!strcmp(token, "(")) {
            list_append(new_type_list, function_init(parse_parameter_list(stream, namespace)));
            token = scan(stream);
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
            return declarator_init(id, type_list);
        }
    }
}

static int address_int(void *address)
{
    switch (type(address)) {
        case arithmetic_t:
            return atoi(((arithmetic *)address)->value);
        case unary_t:
            return (((unary *)address)->op == "+") ? address_int(((unary *)address)->expr)
                         : -address_int(((unary *)address)->expr);
    }
}

static is_zero(void *expr)
{
    if (type(expr) == arithmetic_t) {
        arithmetic *arith = expr;
        double val = atof(arith->value);
        if (val == 0)
            return 1;
    }
    return 0;
}

static void *parse_initializer(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    int initialized = 0;
    if (!strcmp(token, "{")) {
        list *retptr = list_node();
        token = scan(stream);
        if (!strcmp(token, "}"))
            return retptr;
        unscan(token, stream);
        while (1) {
            void *val = parse_initializer(stream, namespace);
            if (val)
                initialized = 1;
            list_append(retptr, val);
            token = scan(stream);
            if (!strcmp(token, ",")) {
                token = scan(stream);
                if (!strcmp(token, "}"))
                    return retptr;
                unscan(token, stream);
            }
            else if (!strcmp(token, "}")) {
                if (initialized)
                    return retptr;
                else
                    return list_node();
            }
        }
    }
    else {
        unscan(token, stream);
        void *expr = parse_assignment(stream, namespace);
        if (is_zero(expr))
            return 0;
        return expr;
    }
}

int list_len(list *head) {
    int len = 0;
    while (head = head->next)
        len++;
    return len;
}

static void *initializing(void *node, void *initializer)
{
    switch (type(get_type_list(node)->content)) {
        case arithmetic_specifier_t: case enum_type: case pointer_t: case function_t: {
            void *aval;
            if (initializer) {
                if (type(initializer) == list_t)
                    return initializing(node, ((list *)initializer)->next->content);
                else
                    return ASSIGNMENT(node, initializer);
            }
            else
                return ASSIGNMENT(node, ARITHMETIC(strdup("0"), int_t));
        }
        case array_t: {
            array *a = (array *)get_type_list(node)->content;
            void *content_s = get_type_list(node)->next->content;
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
            if (a->size) {
                if (type(a->size) == arithmetic_t) {
                    arithmetic *arith = (arithmetic *)a->size;
                    arithmetic_specifier *specifier = arith->type_list->content;
                    if (specifier->atype == int_t) {
                        int size = atoi(arith->value);
                        int i;
                        list *ptr;
                        list *stmts = list_node();
                        if (initializer) {
                            for (i = 0, ptr = ((list *)initializer)->next; i < size; i++) {
                                void *inival;
                                if (ptr && ptr->content && type(ptr->content) == list_t) {
                                    inival = ptr->content;
                                    ptr = ptr->next;
                                }
                                else if (subsize >= 0) {
                                    int i = 0;
                                    list *sub_initializer = list_node();
                                    while (i++ < subsize) {
                                        if (!ptr)
                                            list_append(sub_initializer, 0);
                                        else {
                                            list_append(sub_initializer, ptr->content);
                                            ptr = ptr->next;
                                        }
                                    }
                                    inival = sub_initializer;
                                }
                                else {
                                    if (!ptr)
                                        inival = 0;
                                    else {
                                        inival = ptr->content;
                                        ptr = ptr->next;
                                    }
                                }
                                void *ap = initializing(ARRAY_REF(node, ARITHMETIC(strdup(itoa(i)), int_t)), inival);
                                if (ap)
                                    list_append(stmts, ap);
                            }
                        }
                        else {
                            for (i = 0; i < size; i++) {
                                void *ap = initializing(ARRAY_REF(node, ARITHMETIC(strdup(itoa(i)), int_t)), 0);
                                if (ap)
                                    list_append(stmts, ap);
                            }
                        }
                        return COMPOUND_STMT(stmts, 0);
                    }
                }
            }
            else {
                list *ptr;
                int i;
                list *stmts = list_node();
                for (ptr = ((list *)initializer)->next, i = 0; ptr; i++) {
                    void *inival;
                    if (type(ptr->content) == list_t) {
                        inival = ptr->content;
                        ptr = ptr->next;
                    }
                    else if (subsize >= 0) {
                        list *sub_initializer = list_node();
                        int i = 0;
                        while (i++ < subsize) {
                            if (!ptr)
                                list_append(sub_initializer, 0);
                            else {
                                list_append(sub_initializer, ptr->content);
                                ptr = ptr->next;
                            }
                        }
                        inival = sub_initializer;
                    }
                    else {
                        inival = ptr->content;
                        ptr = ptr->next;
                    }
                    void *ap = initializing(ARRAY_REF(node, ARITHMETIC(strdup(itoa(i)), int_t)), inival);
                    if (ap)
                        list_append(stmts, ap);
                }
                a->size = ARITHMETIC(itoa(i), int_t);
                return COMPOUND_STMT(stmts, 0);
            }
        }
        case struct_specifier_t: {
            struct_specifier *specifier = get_type_list(node)->content;
            if (!specifier->declaration_list)
                return 0;
            list *stmts = list_node();
            list *ptr;
            if (initializer) {
                list *ptr2 = ((list *)initializer)->next;
                for (ptr = specifier->declaration_list->next; ptr; ptr = ptr->next) {
                    declaration *node = ptr->content;
                    void *s = node->type_list->content;
                    void *inival;
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

                    if (ptr2 && type(ptr2) == list_t) {
                        inival = ptr2->content;
                        ptr2 = ptr2->next;
                    }
                    else if (subsize >= 0) {
                        list *sub_initializer = list_node();
                        int i = 0;
                        while (i++ < subsize) {
                            if (!ptr2)
                                list_append(sub_initializer, 0);
                            else {
                                list_append(sub_initializer, ptr2->content);
                                ptr2 = ptr2->next;
                            }
                        }
                        inival = sub_initializer;
                    }
                    else {
                        if (!ptr2)
                            inival = 0;
                        else {
                            inival = ptr2->content;
                            ptr2 = ptr2->next;
                        }
                    }
                    void *ap = initializing(node, inival);
                    if (ap)
                        list_append(stmts, ap);
                }
            }
            else {
                for (ptr = specifier->declaration_list->next; ptr; ptr = ptr->next) {
                    void *ap = initializing(node, 0);
                    if (ap)
                        list_append(stmts, ap);
                }
            }
            return COMPOUND_STMT(stmts, 0);
        }
    }
}

/* add the second parameter to the fist */
static void size_add(struct size *left, struct size *right)
{
    if (left->constant) {
        if (right->constant)
            left->ival += right->ival;
        else {
            left->vval = BINARY(add, ARITHMETIC(itoa(left->ival), int_t), right->vval);
            left->constant = 0;
        }
    }
    else {
        if (right->constant)
            left->vval = BINARY(add, left->vval, ARITHMETIC(itoa(right->ival), int_t));
        else
            left->vval = BINARY(add, left->vval, right->vval);
    }
}

static int is_declared(namespace_t *namespace, char *id)
{
    list *ptr;
    for (ptr = namespace->declaration_list->next; ptr; ptr = ptr->next) {
        declaration *node = ptr->content;
        if (!strcmp(node->id, id))
            return 1;
    }
    return 0;
}

static static_initialization *static_initialization_init(char *tag, compound_stmt *body, int initialized)
{
    static_initialization *retptr = (static_initialization *)malloc(sizeof(static_initialization));
    retptr->type = static_initialization_t;
    retptr->tag = tag;
    retptr->body = body;
    retptr->initialized = initialized;
    return retptr;
}

typedef_t *typedef_init(char *id, list *type_list)
{
    typedef_t *retptr = malloc(sizeof(typedef_t));
    retptr->id = id;
    retptr->type_list = type_list;
    return retptr;
}

void *parse_declaration(FILE *stream, namespace_t *namespace, int in_struct)
{
    list *specifiers = parse_specifier(stream, namespace);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    if (!storage)
        storage = auto_storage_init(1, 0, 0);
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return 0;
    unscan(token, stream);
    list *stmt_list = list_node();
    while (1) {
        declarator *dptr = parse_declarator(stream, 0, namespace);
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
        if (!in_struct && type(storage) == typedef_storage_t)
            list_append(namespace->typedefs, typedef_init(id, type_list));
        declaration *node;
        int array_no_size = 0;
        if (in_struct) {
            node = declaration_init(id, 0, type_list);
            list_append(namespace->declaration_list, node);
        }
        else if (type(storage) == auto_storage_t) {
            if (type(type_list->content) == array_t && !((array *)type_list->content)->size) {
                node = declaration_init(id, 0, type_list);
                list_append(namespace->declaration_list, node);
                array_no_size = 1;
            }
            else {
                struct size *thesize = size(type_list);
                size_add(namespace->auto_size, thesize);
                int constant = namespace->auto_size->constant;
                int iaddress = namespace->auto_size->ival;
                void *vaddress = namespace->auto_size->vval;
                auto_storage *a_storage = auto_storage_init(constant, iaddress, vaddress);
                if (type(type_list->content) == struct_specifier_t) {
                    auto_storage *s = a_storage;
                    struct_specifier *old_specifier = type_list->content;
                    list *old_declaration_list = old_specifier->declaration_list;
                    list *declaration_list = 0;
                    if (old_declaration_list) {
                        declaration_list = list_node();
                        list *ptr;
                        for (ptr = old_declaration_list->next; ptr; ptr = ptr->next) {
                            declaration *od = ptr->content;
                            list_append(declaration_list, declaration_init(od->id, s, od->type_list));
                            s = auto_storage_add_size_nip(s, size(od->type_list));
                        }
                    }
                    struct_specifier *own_specifier = struct_specifier_init(old_specifier->id, declaration_list);
                    type_list->content = own_specifier;
                }
                node = declaration_init(id, a_storage, type_list);
                list_append(namespace->declaration_list, node);
            }
        }
        else if (type(storage) == static_storage_t) {
            static_storage *s_storage = static_storage_init();
            s_storage->tag = get_tag();
            if (type(type_list->content) == struct_specifier_t) {
                static_storage *s = s_storage;
                struct_specifier *old_specifier = type_list->content;
                list *old_declaration_list = old_specifier->declaration_list;
                list *declaration_list = 0;
                if (old_declaration_list) {
                    declaration_list = list_node();
                    list *ptr;
                    for (ptr = old_declaration_list->next; ptr; ptr = ptr->next) {
                        declaration *od = ptr->content;
                        list_append(declaration_list, declaration_init(od->id, s, od->type_list));
                        struct size *thesize = size(od->type_list);
                        static_storage *ns = static_storage_init();
                        ns->tag = get_tag();
                        s = ns;
                    }
                }
                struct_specifier *own_specifier = struct_specifier_init(old_specifier->id, declaration_list);
                type_list->content = own_specifier;
            }
            node = declaration_init(id, s_storage, type_list);
            list_append(namespace->declaration_list, node);
        }
        token = scan(stream);
        if (!strcmp(token, ","))
            ;
        else if (!strcmp(token, "=")) {
            list *initializer = parse_initializer(stream, namespace);
            void *stmt = initializing(node, initializer);
            if (array_no_size) {
                struct size *thesize = size(type_list);
                size_add(namespace->auto_size, thesize);
                int constant = namespace->auto_size->constant;
                int iaddress = namespace->auto_size->ival;
                void *vaddress = namespace->auto_size->vval;
                auto_storage *a_storage = auto_storage_init(constant, iaddress, vaddress);
                node->storage = a_storage;
            }
            if (type(node->storage) == static_storage_t) {
                int initialized = initializer ? 1 : 0;
                char *tag = ((static_storage *)node->storage)->tag;
                compound_stmt *body;
                if (type(stmt) == compound_stmt_t)
                    body = stmt;
                else {
                    list *stmts = list_node();
                    list_append(stmts, stmt);
                    body = COMPOUND_STMT(stmts, 0);
                }
                list_append(stmt_list, static_initialization_init(tag, body, initialized));
            }
            else
                list_append(stmt_list, stmt);
            token = scan(stream);
            if (!strcmp(token, ","))
                ;
            else if (!strcmp(token, ";"))
                break;
        }
        else if (!strcmp(token, ";")) {
            if (!in_struct && type(storage) == static_storage_t) {
                char *tag = ((static_storage *)node->storage)->tag;
                list *stmts = list_node();
                list_append(stmts, ASSIGNMENT(node, 0));
                compound_stmt *body = COMPOUND_STMT(stmts, 0);
                list_append(stmt_list, static_initialization_init(tag, body, 0));
            }
            break;
        }
    }
    return COMPOUND_STMT(stmt_list, 0);
}

list *parse_type_name(FILE *stream, struct namespace *namespace)
{
    list *specifiers = parse_specifier(stream, namespace);
    void *specifier = specifiers->next->next->content;
    declarator *dptr = parse_declarator(stream, 1, namespace);
    list *type_list = dptr->type_list;
    list *ptr = type_list;
    while (ptr->next)
        ptr = ptr->next;
    ptr->content = specifier;
    return type_list;
}

void *parse_primary(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (is_int(token)) {
        unscan(token, stream);
        return parse_arithmetic_constant(stream);
    }
    else if (is_char(token))
        return ARITHMETIC(token, char_t);
    else if (is_float(token)) {
        unscan(token, stream);
        return parse_arithmetic_constant(stream);
    }
    else if (is_str(token)) {
        char *value = token + 1;
        value[strlen(value)] = 0;
        string *retptr = STRING(data_size, value);
        data_size += strlen(value)+1;
        return retptr;
    }
    else if (is_id(token)) {
        enumerator_t *enumerator = find_enumerator(namespace, token);
        if (enumerator)
            return ARITHMETIC(itoa(enumerator->value), int_t);
        void *retptr = find_identifier(namespace, token);
        if (retptr)
            return retptr;
    }
    else if (!strcmp(token, "(")) {
        void *retptr = parse_expression(stream, namespace);
        token = scan(stream);
        return retptr;
    }
}

static inline list *parse_argument_expression_list(FILE *stream, namespace_t *namespace)
{
    list *retptr = list_node();
    char *token;
    while (1) {
        list_append(retptr, parse_assignment(stream, namespace));
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            return retptr;
        }
    }
}

static void *union_ref_init(void *primary, list *type_list)
{
    union_ref_t *retptr = malloc(sizeof(union_ref_t));
    retptr->type = union_ref_type;
    retptr->primary = primary;
    retptr->type_list = type_list;
    return retptr;
}

static void *union_ref_proc(void *primary, char *id)
{
    union_specifier *specifier = get_type_list(primary)->content;
    if (specifier->declaration_list) {
        list *ptr, *type_list;
        for (ptr = specifier->declaration_list->next; ptr; ptr = ptr->next) {
            declaration *node = ptr->content;
            if (!strcmp(node->id, id))
                return union_ref_init(primary, node->type_list);
        }
    }
}

void *parse_postfix(FILE *stream, namespace_t *namespace)
{
    void *primary = parse_primary(stream, namespace);
    char *token;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            primary = ARRAY_REF(primary, parse_expression(stream, namespace));
            token = scan(stream);
        }
        else if (!strcmp(token, "(")) {
            token = scan(stream);
            list *argument_expression_list;
            if (!strcmp(token, ")"))
                argument_expression_list = 0;
            else {
                unscan(token, stream);
                argument_expression_list = parse_argument_expression_list(stream, namespace);
                token = scan(stream);
            }
            primary = FUNCALL(primary, argument_expression_list);
        }
        else if (!strcmp(token, ".")) {
            token = scan(stream);
            void *specifier = get_type_list(primary)->content;
            if (type(specifier) == struct_specifier_t)
                primary = STRUCT_REF(primary, token);
            else
                primary = union_ref_proc(primary, token);
        }
        else if (!strcmp(token, "->")) {
            token = scan(stream);
            void *specifier = get_type_list(primary)->content;
            if (type(specifier) == struct_specifier_t)
                primary = STRUCT_REF(INDIRECTION(primary), token);
            else if (type(specifier) == union_specifier_t)
                primary = union_ref_proc(INDIRECTION(primary), token);
        }
        else if (!strcmp(token, "++")) {
            primary = POSINC(primary, 1);
        }
        else if (!strcmp(token, "--")) {
            primary = POSINC(primary, 0);
        }
        else {
            unscan(token, stream);
            return primary;
        }
    }
}

void *size_to_expr(struct size *size)
{
    if (size->constant)
        return ARITHMETIC(itoa(size->ival), int_t);
    else
        return size->vval;
}

void *size_expr(list *type_list)
{
    struct size *thesize = size(type_list);
    return size_to_expr(thesize);
}

void *parse_unary(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (!strcmp(token, "++"))
        return PREINC(parse_unary(stream, namespace), 1);
    else if (!strcmp(token, "--"))
        return PREINC(parse_unary(stream, namespace), 0);
    else if (!strcmp(token, "&"))
        return ADDR(parse_cast(stream, namespace));
    else if (!strcmp(token, "*"))
        return INDIRECTION(parse_cast(stream, namespace));
    else if (!strcmp(token, "+") || !strcmp(token, "-")
                   || !strcmp(token, "~") || !strcmp(token, "!"))
        return UNARY(parse_cast(stream, namespace), token);
    else if (!strcmp(token, "sizeof")) {
        token = scan(stream);
        if (!strcmp(token, "(")) {
            token = scan(stream);
            unscan(token, stream);
            list *type_list;
            if (is_specifier(token) || is_qualifier(token) || find_typedef(namespace, token))
                type_list = parse_type_name(stream, namespace);
            else
                type_list = get_type_list(parse_expression(stream, namespace));
            token = scan(stream);
            return size_expr(type_list);
        }
        else {
            unscan(token, stream);
            list *type_list = get_type_list(parse_unary(stream, namespace));
            return size_expr(type_list);
        }
    }
    else {
        unscan(token, stream);
        return parse_postfix(stream, namespace);
    }
}

void *parse_cast(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        token = scan(stream);
        unscan(token, stream);
        if (is_specifier(token) || is_qualifier(token) || find_typedef(namespace, token)) {
            list *type_list = parse_type_name(stream, namespace);
            token = scan(stream);
            if (!strcmp(token, ")"))
                return CAST(type_list, parse_cast(stream, namespace));
        }
        else {
            unscan("(", stream);
            parse_primary(stream, namespace);
        }
    }
    else {
        unscan(token, stream);
        return parse_unary(stream, namespace);
    }
}

enum types get_type(void *expr)
{
    return type(get_type_list(expr)->content);
}

int type_is_int(void *expr)
{
    void *a = get_type_list(expr)->content;
    if (type(a) == arithmetic_specifier_t) {
        arithmetic_specifier *s = a;
        switch (s->atype) {
            case char_t: case unsigned_char_t:
            case short_t: case unsigned_short_t:
            case int_t: case unsigned_int_t:
            case long_t: case unsigned_long_t:
            case long_long_t: case unsigned_long_long_t:
                return 1;
            default: return 0;
        }
    }
    return 0;
}

/* arithmetic convertion contains integral promotion */
void **arithmetic_conversion(void *left, void *right)
{
    void **retptr = (void **)malloc(sizeof(void *)*2);
    enum atypes l = get_type(left), r = get_type(right), res;
    if (l == long_double_t || r == long_double_t)
        res = long_double_t;
    else if (l == double_t || r == double_t)
        res = double_t;
    else if (l == float_t || r == float_t)
        res = float_t;
    else if (l == unsigned_long_long_t || r == unsigned_long_long_t)
        res = unsigned_long_long_t;
    else if (l == long_long_t || r == long_long_t)
        res = long_long_t;
    else if (l == unsigned_int_t || r == unsigned_int_t)
        res = unsigned_int_t;
    else /* integral promotion */
        res = int_t;
    if (l != res) {
        list *type_list = list_node();
        type_list->content = arithmetic_specifier_init(res);
        left = CAST(type_list, left);
    }
    if (r != res) {
        list *type_list = list_node();
        type_list->content = arithmetic_specifier_init(res);
        right = CAST(type_list, right);
    }
    *retptr = left;
    *(retptr+1) = right;
    return retptr;
}

void *parse_m_expr(FILE *stream, namespace_t *namespace) {
    void *expr = parse_cast(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/")) {
            btype_t btype = !strcmp(token, "*") ? mul : divi;
            void *expr2 = parse_cast(stream, namespace);
            if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(btype, *ptr, *(ptr+1));
            }
        }
        else if (!strcmp(token, "%")) {
            void *expr2 = parse_cast(stream, namespace);
            if (type_is_int(expr) && type_is_int(expr2)) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(mod, *ptr, *(ptr+1));
            }
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_a_expr(FILE *stream, namespace_t *namespace) {
    void *expr = parse_m_expr(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-")) {
            btype_t btype = !strcmp(token, "+") ? add : sub;
            void *expr2 = parse_m_expr(stream, namespace);
            if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(btype, *ptr, *(ptr+1));
            }
            else if ((get_type(expr) == pointer_t || get_type(expr) == array_t) && type_is_int(expr2))
                expr = BINARY(btype, expr, BINARY(mul, expr2, size_expr(get_type_list(INDIRECTION(expr)))));
            else if ((get_type(expr2) == pointer_t || get_type(expr2) == array_t) && type_is_int(expr))
                expr = BINARY(btype, expr2, BINARY(mul, expr, size_expr(get_type_list(INDIRECTION(expr2)))));
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_shift(FILE *stream, namespace_t *namespace) {
    void *expr = parse_a_expr(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<<") || !strcmp(token, ">>")) {
            btype_t btype = !strcmp(token, "<<") ? lshift : rshift;
            void *expr2 = parse_a_expr(stream, namespace);
            if (type_is_int(expr) && type_is_int(expr2)) {
                set_type_list(expr, integral_promotion2(get_type_list(expr)));
                set_type_list(expr2, integral_promotion2(get_type_list(expr2)));
                expr = BINARY(btype, expr, expr2);
            }
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_relational(FILE *stream, namespace_t *namespace) {
    void *expr = parse_shift(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") ||
                  !strcmp(token, "<=") || !strcmp(token, ">=")) {
            btype_t btype = !strcmp(token, "<") ? lt :
                            !strcmp(token, ">") ? gt :
                            !strcmp(token, "<=") ? le : ge;
            void *expr2 = parse_shift(stream, namespace);
            if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(btype, *ptr, *(ptr+1));
            }
            else if ((get_type(expr) == pointer_t ||get_type(expr) == array_t) &&
                           (get_type(expr2) == pointer_t || get_type(expr2) == array_t))
                expr = BINARY(btype, expr, expr2);
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_equality(FILE *stream, namespace_t *namespace) {
    void *expr = parse_relational(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!=")) {
            btype_t btype = !strcmp(token, "==") ? eq : neq;
            void *expr2 = parse_relational(stream, namespace);
            if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(btype, *ptr, *(ptr+1));
            }
            else if ((get_type(expr) == pointer_t ||get_type(expr) == array_t) &&
                           (get_type(expr2) == pointer_t || get_type(expr2) == array_t))
                expr = BINARY(btype, expr, expr2);
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_and(FILE *stream, namespace_t *namespace) {
    void *expr = parse_equality(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&")) {
            void *expr2 = parse_equality(stream, namespace);
            if (type_is_int(expr) && type_is_int(expr2)) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(band, *ptr, *(ptr+1));
            }
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_xor(FILE *stream, namespace_t *namespace) {
    void *expr = parse_and(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^")) {
            void *expr2 = parse_and(stream, namespace);
            if (type_is_int(expr) && type_is_int(expr2)) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(bxor, *ptr, *(ptr+1));
            }
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_or(FILE *stream, namespace_t *namespace) {
    void *expr = parse_xor(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|")) {
            void *expr2 = parse_xor(stream, namespace);
            if (type_is_int(expr) && type_is_int(expr2)) {
                void **ptr = arithmetic_conversion(expr, expr2);
                expr = BINARY(bor, *ptr, *(ptr+1));
            }
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_logical_and(FILE *stream, namespace_t *namespace) {
    void *expr = parse_or(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&"))
            expr = BINARY(land, expr, parse_or(stream, namespace));
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_logical_or(FILE *stream, namespace_t *namespace)
{
    void *expr = parse_logical_and(stream, namespace);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||"))
            expr = BINARY(lor, expr, parse_logical_and(stream, namespace));
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *conditional_init(void *expr1, void *expr2, void *expr3)
{
    conditional_t *retptr = malloc(sizeof(conditional_t));
    retptr->type = conditional_type;
    retptr->expr1 = expr1;
    list *type_list1 = get_type_list(expr2);
    list *type_list2 = get_type_list(expr3);
    void *s1 = type_list1->content, *s2 = type_list2->content;
    if (type(s1) == type(s2) == arithmetic_t) {
        void **exprs = arithmetic_conversion(expr2, expr3);
        retptr->expr2 = exprs[0];
        retptr->expr3 = exprs[1];
        retptr->type_list = get_type_list(expr2);
    }
    else if (type(s1) == arithmetic_t && type(s2) == pointer_t) {
        retptr->expr2 = CAST(type_list2, expr2);
        retptr->expr3 = expr3;
        retptr->type_list = type_list2;
    }
    else if (type(s1) == pointer_t && type(s2) == arithmetic_t) {
        retptr->expr2 = expr2;
        retptr->expr3 = CAST(type_list1, expr3);
        retptr->type_list = type_list1;
    }
    else if (type(s1) == type(s2) == pointer_t) {
        if (!type_list1->next->content && type_list2->next->content) {
            retptr->expr2 = expr2;
            retptr->expr3 = CAST(type_list1, expr3);
            retptr->type_list = type_list1;
        }
        else if (type_list1->next->content && !type_list2->next->content) {
            retptr->expr2 = CAST(type_list2, expr2);
            retptr->expr3 = expr3;
            retptr->type_list = type_list2;
        }
        else {
            retptr->expr2 = expr2;
            retptr->expr3 = expr3;
            retptr->type_list = type_list1;
        }
    }
    else {
        retptr->expr2 = expr2;
        retptr->expr3 = expr3;
        retptr->type_list = type_list1;
    }
    return retptr;
}

void *parse_conditional(FILE *stream, namespace_t *namespace)
{
    void *expr1 = parse_logical_or(stream, namespace);
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        void *expr2 = parse_expression(stream, namespace);
        token = scan(stream);
        void *expr3 = parse_conditional(stream, namespace);
        return conditional_init(expr1, expr2, expr3);
    }
    else {
        unscan(token, stream);
        return expr1;
    }
}

static int is_assignment_operator(char *token)
{
    char *assignment_operators[] = {
        "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", 0,
    };
    char **ptr;
    for (ptr = assignment_operators; *ptr; ptr++)
        if (!strcmp(token, *ptr))
            return 1;
    return 0;
}

void *parse_assignment(FILE *stream, namespace_t *namespace)
{
    void *expr = parse_conditional(stream, namespace);
    char *token = scan(stream);
    if (!strcmp(token, "=")) {
        void *expr2 = parse_assignment(stream, namespace);
        return ASSIGNMENT(expr, expr2);
    }
    else if (is_assignment_operator(token)) {
        btype_t btype = !strcmp(token, "*=") ? mul :
                        !strcmp(token, "/=") ? divi :
                        !strcmp(token, "%=") ? mod :
                        !strcmp(token, "+=") ? add :
                        !strcmp(token, "-=") ? sub :
                        !strcmp(token, "<<=") ? lshift :
                        !strcmp(token, ">>=") ? rshift :
                        !strcmp(token, "&=") ? band :
                        !strcmp(token, "^=") ? bxor : bor;
        void *expr2 = parse_assignment(stream, namespace);
        return ASSIGNMENT(expr, BINARY(btype, expr, expr2));
    }
    else {
        unscan(token, stream);
        return expr;
    }
}

void *parse_expression(FILE *stream, namespace_t *namespace)
{
    list *assignment_list = list_node();
    char *token;
    while (1) {
        void *assignment = parse_assignment(stream, namespace);
        list_append(assignment_list, assignment);
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            return EXPRESSION(assignment_list, get_type_list(assignment));
        }
    }
}

void *parse_expression_stmt(FILE *stream, namespace_t *namespace)
{
    list *assignment_list = list_node();
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return EXPRESSION_STMT(assignment_list);
    unscan(token, stream);
    while (1) {
        list_append(assignment_list, parse_assignment(stream, namespace));
        token = scan(stream);
        if (!strcmp(token, ";"))
            return EXPRESSION_STMT(assignment_list);
    }
}

void *parse_statement(FILE *stream, struct namespace *namespace)
{
    int offset = ftell(stream);
    int line = file_info.line;
    int column = file_info.column;
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, "{"))
        return parse_compound_stmt(stream, namespace, 0);
    else if (!strcmp(token, "if"))
        return parse_if_stmt(stream, namespace);
    else if (!strcmp(token, "switch"))
        return parse_switch_stmt(stream, namespace);
    else if (!strcmp(token, "case"))
        return parse_case_stmt(stream, namespace);
    else if (!strcmp(token, "default"))
        return parse_default_stmt(stream, namespace);
    else if (!strcmp(token, "while"))
        return parse_while_stmt(stream, namespace);
    else if (!strcmp(token, "for"))
        return parse_for_stmt(stream, namespace);
    else if (!strcmp(token, "goto"))
        return parse_goto_stmt(stream, namespace);
    else if (!strcmp(token, "continue"))
        return parse_continue_stmt(stream, namespace);
    else if (!strcmp(token, "break"))
        return parse_break_stmt(stream, namespace);
    else if (!strcmp(token, "return"))
        return parse_return_stmt(stream, namespace);
    else if (is_id(token)) {
        token = scan(stream);
        fseek(stream, offset, SEEK_SET);
        file_info.line = line;
        file_info.column = column;
        if (!strcmp(token, ":"))
            return parse_id_labeled_stmt(stream, namespace);
        else
            return parse_expression_stmt(stream, namespace);
    }
    else
        return parse_expression_stmt(stream, namespace);
}

void *parse_declaration_or_statement(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    unscan(token, stream);
    if (is_specifier(token) || is_storage(token) || is_qualifier(token) || find_typedef(namespace, token))
        return parse_declaration(stream, namespace, 0);
    return parse_statement(stream, namespace);
}

void *parse_compound_stmt(FILE *stream, namespace_t *namespace, list *parameter_list)
{
    char *token = scan(stream);
    struct namespace *i_namespace = namespace_init(namespace);
    if (parameter_list) {
        i_namespace->declaration_list = parameter_list;
        i_namespace->labels = list_node();
    }
    list *statement_list = list_node();
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "}"))
            return COMPOUND_STMT(statement_list, i_namespace);
        unscan(token, stream);
        void *statement = parse_declaration_or_statement(stream, i_namespace);
        if (statement)
            list_append(statement_list, statement);
    }
}

void *parse_if_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "if"
    token = scan(stream);
    void *expr = parse_expression(stream, namespace);
    token = scan(stream);
    void *statement = parse_statement(stream, namespace), *statement2 = 0;
    token = scan(stream);
    if (!strcmp(token, "else"))
        statement2 = parse_statement(stream, namespace);
    else
        unscan(token, stream);
    return IF_STMT(expr, statement, statement2);
}

void *parse_switch_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "switch"
    token = scan(stream);
    void *expr = parse_expression(stream, namespace);
    token = scan(stream);
    char *old_break_tag = break_tag;
    break_tag = get_tag();
    void *statement = parse_statement(stream, namespace);
    switch_stmt *retptr = SWITCH_STMT(expr, statement, break_tag);
    break_tag = old_break_tag;
    return retptr;
}

static int is_integral_constant(void *expression)
{
    if (type(expression) == arithmetic_t) {
        arithmetic *arith = expression;
        arithmetic_specifier *specifier = arith->type_list->content;
        if (specifier->atype == int_t) {
            return 1;
        }
    }
    return 0;
}

void *parse_case_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "case"
    void *expression = parse_conditional(stream, namespace);
    char *value = ((arithmetic *)expression)->value;
    char *tag = get_tag();
    token = scan(stream);
    void *statement = parse_statement(stream, namespace);
    return CASE_STMT(tag, value, statement);
}

void *parse_default_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "default"
    token = scan(stream);  // it should be ":"
    void *statement = parse_statement(stream, namespace);
    return DEFAULT_STMT(get_tag(), statement);
}

void *parse_id_labeled_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    char *id = token;
    token = scan(stream);  // it should be ":"
    void *statement = parse_statement(stream, namespace);
    id_labeled_stmt *retptr = ID_LABELED_STMT(get_tag(), id, statement);
    namespace_t *nptr = namespace;
    do {
        if (nptr->labels)
            list_append(nptr->labels, retptr);
        nptr = nptr->outer;
    } while (nptr);
    return retptr;
}

void *parse_while_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "while"
    token = scan(stream);
    void *expr = parse_expression(stream, namespace);
    token = scan(stream);
    char *old_continue_tag = continue_tag;
    char *old_break_tag = break_tag;
    continue_tag = get_tag();
    break_tag = get_tag();
    void *statement = parse_statement(stream, namespace);
    while_stmt *retptr = WHILE_STMT(expr, statement, 0, continue_tag, break_tag);
    continue_tag = old_continue_tag;
    break_tag = old_break_tag;
    return retptr;
}

void *parse_do_while_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "do"
    char *old_continue_tag = continue_tag;
    char *old_break_tag = break_tag;
    continue_tag = get_tag();
    break_tag = get_tag();
    void *statement = parse_statement(stream, namespace);
    token = scan(stream);
    token = scan(stream);
    void *expr = parse_expression(stream, namespace);
    token = scan(stream);
    token = scan(stream);
    while_stmt *retptr = WHILE_STMT(expr, statement, 1, continue_tag, break_tag);
    continue_tag = old_continue_tag;
    break_tag = old_break_tag;
    return retptr;
}

void *parse_for_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // it should be "for"
    token = scan(stream);
    void *exprs[3];
    char *etks[] = { ";", ";", ")" };
    int i;
    for (i = 0; i < 3; i++) {
        token = scan(stream);
        if (!strcmp(token, etks[i]))
            exprs[i] = 0;
        else {
            unscan(token, stream);
            exprs[i] = parse_expression(stream, namespace);
            token = scan(stream);
        }
    }
    char *old_continue_tag = continue_tag;
    char *old_break_tag = break_tag;
    continue_tag = get_tag();
    break_tag = get_tag();
    void *statement = parse_statement(stream, namespace);
    for_stmt *retptr = FOR_STMT(exprs[0], exprs[1], exprs[2], statement, continue_tag, break_tag);
    continue_tag = old_continue_tag;
    break_tag = old_break_tag;
    return retptr;
}

void *parse_goto_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "goto"
    char *id = scan(stream);
    token = scan(stream);
    namespace_t *nptr = namespace;
    do {
        if (nptr->labels)
            return GOTO_STMT(id, nptr);
        nptr = nptr->outer;
    } while (nptr);
}

void *parse_continue_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "continue"
    token = scan(stream);
    return CONTINUE_STMT(continue_tag);
}

void *parse_break_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "break"
    token = scan(stream);
    return BREAK_STMT(break_tag);
}

void *parse_return_stmt(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);  // should be "return"
    token = scan(stream);
    void *expr = 0;
    if (strcmp(token, ";")) {
        unscan(token, stream);
        expr = parse_expression(stream, namespace);
        token = scan(stream);
    }
    return RETURN_STMT(expr, return_tag);
}

static function_definition_t *
function_definition_init(char *id, list *return_type_list, list *parameter_list, compound_stmt *body, char *return_tag, char *function_tag)
{
    function_definition_t *retptr = (function_definition_t *)malloc(sizeof(function_definition_t));
    retptr->type = function_definition_type;
    retptr->id = id;
    retptr->return_type_list = return_type_list;
    retptr->parameter_list = parameter_list;
    retptr->body = body;
    retptr->return_tag = return_tag;
    retptr->function_tag = function_tag;
    return retptr;
}

static inline list *list_copy(list *head)
{
    list *retptr = list_node(), *ptr, *ptr2 = retptr;
    for (ptr = head->next; ptr; ptr = ptr->next) {
        ptr2->next = list_node();
        ptr2->next->prev = ptr2;
        ptr2->next->content = ptr->content;
        ptr2 = ptr2->next;
    }
    return retptr;
}

static func *
func_init(char *id, char *tag, list *type_list)
{
    func *retptr = malloc(sizeof(func));
    retptr->type = func_t;
    retptr->id = id;
    retptr->tag = tag;
    retptr->type_list = type_list;
    return retptr;
}

void *parse_external_declaration(FILE *stream, namespace_t *namespace)
{
    list *specifiers = parse_specifier(stream, namespace);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    if (!storage)
        storage = extern_storage_init();
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return 0;
    unscan(token, stream);
    int first_loop = 1;
    while (1) {
        declarator *dptr = parse_declarator(stream, 0, namespace);
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
        if (type(storage) == typedef_storage_t)
            list_append(namespace->typedefs, typedef_init(id, type_list));

        token = scan(stream);
        if (!strcmp(token, "{")) {
            function *f = type_list->content;
            list *ptr;
            int psize = 0;
            for (ptr = f->parameter_list->next; ptr; ptr = ptr->next) {
                declaration *d = ptr->content;
                parameter_storage *storage = parameter_storage_init();
                storage->address = psize;
                struct size *thesize = size(d->type_list);
                psize += thesize->ival;
                d->storage = storage;
            }
            unscan(token ,stream);
            char *function_tag = !strcmp(id, "main") ? strdup("_start") : get_tag();
            list_append(func_list, func_init(id, function_tag, type_list));
            list *parameter_list = ((function *)type_list->content)->parameter_list;
            list *return_type_list = list_copy(type_list->next);
            return_tag = get_tag();
            compound_stmt *body = parse_compound_stmt(stream, namespace, list_copy(parameter_list));
            function_definition_t *retptr = function_definition_init(id, return_type_list, parameter_list, body, return_tag, function_tag);
            return retptr;
        }

        declaration *node;

        static_storage *s_storage = static_storage_init();
        s_storage->tag = get_tag();
        if (type(type_list->content) == struct_specifier_t) {
            static_storage *s = s_storage;
            struct_specifier *old_specifier = type_list->content;
            list *old_declaration_list = old_specifier->declaration_list;
            list *declaration_list = 0;
            if (old_declaration_list) {
                declaration_list = list_node();
                list *ptr;
                for (ptr = old_declaration_list->next; ptr; ptr = ptr->next) {
                    declaration *od = ptr->content;
                    list_append(declaration_list, declaration_init(od->id, s, od->type_list));
                    struct size *thesize = size(od->type_list);
                    static_storage *ns = static_storage_init();
                    ns->tag = get_tag();
                    s = ns;
                }
            }
            struct_specifier *own_specifier = struct_specifier_init(old_specifier->id, declaration_list);
            type_list->content = own_specifier;
        }
        node = declaration_init(id, s_storage, type_list);
        list_append(namespace->declaration_list, node);

        if (!strcmp(token, ","))
            continue;
        else if (!strcmp(token, "=")) {
            list *initializer = parse_initializer(stream, namespace);
            token = scan(stream);
            if (!strcmp(token, ";")) {
                void *stmt = initializing(node, initializer);
                compound_stmt *body;
                if (type(node->storage) == static_storage_t) {
                    int initialized = initializer ? 1 : 0;
                    char *tag = ((static_storage *)node->storage)->tag;
                    if (type(stmt) == compound_stmt_t)
                        body = stmt;
                    else {
                        list *stmts = list_node();
                        list_append(stmts, stmt);
                        body = COMPOUND_STMT(stmts, 0);
                    }
                    return static_initialization_init(tag, body, initialized);
                }
                else
                    return stmt;
            }
        }
        else if (!strcmp(token, ";")) {
            if (type(node->storage) == static_storage_t) {
                char *tag = ((static_storage *)node->storage)->tag;
                list *stmts = list_node();
                list_append(stmts, ASSIGNMENT(node, 0));
                compound_stmt *body = COMPOUND_STMT(stmts, 0);
                return static_initialization_init(tag, body, 0);
            }
            return 0;
        }

        first_loop = 0;
    }
}

translation_unit_t *translation_unit_init(list *external_declarations)
{
    translation_unit_t *retptr = malloc(sizeof(translation_unit_t));
    retptr->external_declarations = external_declarations;
    return retptr;
}

translation_unit_t *parse_translation_unit(FILE *stream)
{
    namespace_t *namespace = namespace_init(0);
    list *external_declarations = list_node();
    char *token;
    while (1) {
        token = scan(stream);
        if (!*token)
            break;
        unscan(token, stream);
        void *content = parse_external_declaration(stream, namespace);
        if (content)
            list_append(external_declarations, content);
    }
    return translation_unit_init(external_declarations);
}

aconstant_t *aconstant_init(int is_int, int ival, double fval)
{
    aconstant_t *retptr = malloc(sizeof(aconstant_t));
    retptr->is_int = is_int;
    retptr->ival = ival;
    retptr->fval = fval;
    return retptr;
}

aconstant_t *parse_aconstant(FILE *stream)
{
    long offset = ftell(stream);
    int line = file_info.line;
    int column = file_info.column;

    char *token = scan(stream);
    int is_int_flag;
    int ival;
    double fval;
    if (is_int(token)) {
        ival = atoi(token);
        is_int_flag = 1;
    }
    else if (is_float(token)) {
        fval = atof(token);
        is_int_flag = 0;
    }
    else if (!strcmp(token, "(")) {
        aconstant_t *retptr = parse_a_conditional(stream);
        if (!retptr) {
            unscan(token, stream);
            return 0;
        }
        else {
            token = scan(stream);
            if (!strcmp(token, ")"))
                return retptr;
            else {
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                return 0;
            }
        }
    }
    else {
        unscan(token, stream);
        return 0;
    }
    return aconstant_init(is_int_flag, ival, fval);
}

aconstant_t *parse_a_unary(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "+")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else
            return primary;
    }
    else if (!strcmp(token, "-")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else {
            if (primary->is_int)
                primary->ival = -primary->ival;
            else
                primary->fval = -primary->fval;
            return primary;
        }
    }
    else if (!strcmp(token, "!")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else {
            if (primary->is_int)
                return aconstant_init(1, !primary->ival, 0);
            else
                return aconstant_init(1, !primary->fval, 0);
        }
    }
    else if (!strcmp(token, "~")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else {
            if (primary->is_int)
                return aconstant_init(1, ~primary->ival, 0);
        }
    }
    else {
        unscan(token, stream);
        return parse_aconstant(stream);
    }
}

aconstant_t *parse_a_m_expr(FILE *stream)
{
    aconstant_t *left = parse_a_unary(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*")) {
            aconstant_t *right = parse_a_unary(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival * right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival * right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval * right->ival);
                    else
                        return aconstant_init(0, 0, left->fval * right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "/")) {
            aconstant_t *right = parse_a_unary(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival / right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival / right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval / right->ival);
                    else
                        return aconstant_init(0, 0, left->fval / right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "%")) {
            aconstant_t *right = parse_a_unary(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival * right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}


aconstant_t *parse_a_a_expr(FILE *stream)
{
    aconstant_t *left = parse_a_m_expr(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+")) {
            aconstant_t *right = parse_a_m_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival + right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival + right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval + right->ival);
                    else
                        return aconstant_init(0, 0, left->fval + right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "-")) {
            aconstant_t *right = parse_a_m_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival - right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival - right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval - right->ival);
                    else
                        return aconstant_init(0, 0, left->fval - right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_shift(FILE *stream)
{
    aconstant_t *left = parse_a_a_expr(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, ">>")) {
            aconstant_t *right = parse_a_a_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival >> right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "<<")) {
            aconstant_t *right = parse_a_a_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival << right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_relational(FILE *stream)
{
    aconstant_t *left = parse_a_shift(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival < right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival < right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval < right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval < right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, ">")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival > right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival > right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval > right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval > right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "<=")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival <= right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival <= right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval <= right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval <= right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, ">=")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival >= right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival >= right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval >= right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval >= right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_equality(FILE *stream)
{
    aconstant_t *left = parse_a_relational(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!=")) {
            aconstant_t *right = parse_a_relational(stream);
            if (right) {
                if (!strcmp(token, "==")) {
                    if (left->is_int) {
                        if (right->is_int)
                            left = aconstant_init(1, left->ival == right->ival, 0);
                        else
                            left = aconstant_init(1, left->ival == right->fval, 0);
                    }
                    else {
                        if (right->is_int)
                            left = aconstant_init(1, left->fval == right->ival, 0);
                        else
                            left = aconstant_init(1, left->fval == right->fval, 0);
                    }
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_and(FILE *stream)
{
    aconstant_t *left = parse_a_equality(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&")) {
            aconstant_t *right = parse_a_equality(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival & right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_xor(FILE *stream)
{
    aconstant_t *left = parse_a_and(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^")) {
            aconstant_t *right = parse_a_and(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival ^ right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_or(FILE *stream)
{
    aconstant_t *left = parse_a_xor(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|")) {
            aconstant_t *right = parse_a_xor(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival | right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_logical_and(FILE *stream)
{
    aconstant_t *left = parse_a_or(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&")) {
            aconstant_t *right = parse_a_or(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival && right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival && right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval && right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval && right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_logical_or(FILE *stream)
{
    aconstant_t *left = parse_a_logical_and(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||")) {
            aconstant_t *right = parse_a_logical_and(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival || right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival || right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval || right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval || right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_conditional(FILE *stream)
{
    aconstant_t *cond = parse_a_logical_or(stream);
    if (!cond)
        return 0;
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        long offset = ftell(stream);
        int line = file_info.line;
        int column = file_info.column;
        aconstant_t *first = parse_a_conditional(stream);
        if (first) {
            token = scan(stream);
            if (!strcmp(token, ":")) {
                aconstant_t *second = parse_a_conditional(stream);
                if (second) {
                    token = scan(stream);
                    char *atks[] = {
                        "?", "||", "&&", "|", "^", "&", "==", "!=", "<", ">",
                        "<=", ">=", "<<", ">>", "+", "-", "*", "/", "%", 0,
                    };
                    char **ptr;
                    for (ptr = atks; *ptr; ptr++) {
                        if (!strcmp(token, *ptr)) {
                            fseek(stream, offset, SEEK_SET);
                            file_info.line = line;
                            file_info.column = column;
                            return cond;
                        }
                    }
                    unscan(token, stream);
                    if (cond->is_int) {
                        if (cond->ival)
                            return first;
                        else
                            return second;
                    }
                    else {
                        if (cond->fval)
                            return first;
                        else
                            return second;
                    }
                }
                else {
                    fseek(stream, offset, SEEK_SET);
                    file_info.line = line;
                    file_info.column = column;
                    return cond;
                }
            }
            else {
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                return cond;
            }
        }
        else {
            unscan(token, stream);
            return cond;
        }
    }
    else {
        unscan(token, stream);
        return cond;
    }
}

arithmetic *parse_arithmetic_constant(FILE *stream)
{
    aconstant_t *aconstant = parse_a_conditional(stream);
    char *value = malloc(1024);
    int atype;
    if (aconstant->is_int) {
        atype = int_t;
        sprintf(value, "%d", aconstant->ival);
    }
    else {
        atype = double_t;
        sprintf(value, "%f", aconstant->fval);
    }
    return ARITHMETIC(value, atype);
}


int main(int argc, char **argv)
{
    if (argc != 3) {
        puts("<usage> ./scan infile outfile");
        exit(0);
    }

    char *ifile = argv[1], *ofile = argv[2];

    file_info.file_name = ifile;
    file_info.line = 1;
    file_info.column = 1;
    file_info.error = 0;
    file_info.eof = 0;

    continue_tag = break_tag = return_tag = 0;
    func_list = list_node();

    FILE *istream = fopen(ifile, "r");
    FILE *ostream = fopen(ofile, "w");
    data_buff = buff_init();
    bss_buff = buff_init();
    text_buff = buff_init();
    data_size = 0;
    namespace = namespace_init(0);
    tab = 0;

    syntax_translation_unit(istream);
    if (file_info.error) {
        fclose(istream);
        fclose(ostream);
        return 0;
    }

    file_info.eof = 0;
    rewind(istream);
    translation_unit_t *translation_unit = parse_translation_unit(istream);
    translation_unit_gencode(translation_unit);
    char *data_section = buff_puts(data_buff);
    char *bss_section = buff_puts(bss_buff);
    char *text_section = buff_puts(text_buff);

    /* NOTE! don't add a ':' after section name */
    write_code("section .data\n", ostream);
    write_code("global _start\n", ostream);
    write_code(data_section, ostream);
    write_code("section .bss\n", ostream);
    write_code(bss_section, ostream);
    write_code("section .text\n", ostream);
    write_code(text_section, ostream);

    fclose(istream);
    fclose(ostream);

    return 0;
}
