#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "barec.h"

#define write_code(CODE, STREAM) fwrite(CODE, 1, strlen(CODE), STREAM)

char *scan(FILE *stream) {
    char *buffer = (char *)malloc(1024);
    int ch, i = 0;
    while (!feof(stream)) {
        ch = fgetc(stream);
        if (ch == '>' || ch == '<') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == buffer[0]) {
                buffer[1] = ch;
                ch = fgetc(stream);
                if (ch == '=') {
                    buffer[2] = ch;
                    buffer[3] = 0;
                    return buffer;
                }
                else {
                    buffer[2] = 0;
                    ungetc(ch, stream);
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
                ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '+' || ch == '&' || ch == '|') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == buffer[0] || ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '-') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == buffer[0] || ch == '=' || ch == '>') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                ungetc(ch, stream);
                return buffer;
            }
        }
        else if (ch == '=' || ch == '!' || ch == '*' || ch == '^'
                     || ch == '/' || ch == '%') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                ungetc(ch, stream);
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
            ch = fgetc(stream);
            if (ch >= '0' && ch <= '9') {
                buffer[0] = '.';
                buffer[1] = ch;
                i = 2;
                while (1) {
                    ch = fgetc(stream);
                    if (ch >= '0' && ch <= '9')
                        buffer[i++] = ch;
                    else {
                        ungetc(ch, stream);
                        break;
                    }
                }
                buffer[i] = 0;
                return buffer;
            }
            else {
                ungetc(ch, stream);
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
                ch = fgetc(stream);
                if (ch >= '0' && ch <= '9')
                    buffer[i++] = ch;
                else if (ch == '.') {
                    if (point == 0) {
                        point = 1;
                        buffer[i++] = ch;
                    }
                }
                else {
                    ungetc(ch, stream);
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
                ch = fgetc(stream);
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z' || ch == '_') ||
                         (ch >= '0' && ch <= '9'))
                    buffer[i++] = ch;
                else {
                    ungetc(ch, stream);
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
                ch = fgetc(stream);
                buffer[i++] = ch;
                if (ch == buffer[0]) {
                    buffer[i] = 0;
                    return buffer;
                }
            }
        }
    }
    buffer[0] = 0;
    return buffer;
}

static struct_specifier *find_struct_specifier(struct namespace *namespace, char *id)
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

static declaration *find_declaration(struct namespace *namespace, char *id)
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

static struct namespace *namespace_init(struct namespace *outer)
{
    struct namespace *retptr = (struct namespace *)malloc(sizeof(struct namespace));
    retptr->declaration_list = list_node();
    retptr->struct_s_list = list_node();
    retptr->auto_size = ARITHMETIC(strdup("0"), int_t);
    retptr->outer = outer;
    return retptr;
}

/* in the returned list, first is the storage specifier, the second is the type specifier */
list *parse_specifier(FILE *stream, struct namespace *namespace) {
    char *token;
    void *storage = 0, *specifier = 0;
    list *retptr = list_node();
    int sign = 1;
    enum atypes atype = -1;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "auto")) {
            storage = auto_storage_init();
        }
        else if (!strcmp(token, "char"))
            atype = char_t;
        else if (!strcmp(token, "short"))
            atype = short_t;
        else if (!strcmp(token, "int"))
            atype = int_t;
        else if (!strcmp(token, "long")) {
            if (atype == long_t)
                atype = long_long_t;
            else if (atype == -1 || atype == int_t)
                atype = long_t;
        }
        else if (!strcmp(token, "float"))
            atype = float_t;
        else if (!strcmp(token, "double")) {
            if (atype = long_t)
                atype = long_double_t;
            else if (atype == -1)
                atype = double_t;
        }
        else if (!strcmp(token, "unsigned"))
            sign = 0;
        else if (!strcmp(token, "signed"))
            sign = 1;
        else if (!strcmp(token, "struct")) {
            token = scan(stream);
            if (is_id(token)) {
                char *id = token;
                token = scan(stream);
                if (!strcmp(token, "{")) {
                    struct namespace *s_namespace = namespace_init(namespace);
                    while (strcmp((token = scan(stream)), "}")) {
                        unscan(token, stream);
                        parse_declaration(stream, s_namespace, 1);
                    }
                    specifier = struct_specifier_init(id, s_namespace->declaration_list);
                    list_append(namespace->struct_s_list, specifier);
                }
                else {
                    unscan(token, stream);
                    list *ptr;
                    specifier = find_struct_specifier(namespace, id);
                }
            }
            else if (!strcmp(token, "{")) {
                struct namespace *s_namespace = namespace_init(namespace);
                while (strcmp((token = scan(stream)), "}")) {
                    unscan(token, stream);
                    parse_declaration(stream, s_namespace, 1);
                }
                specifier = struct_specifier_init(0, s_namespace->declaration_list);
            }
        }
        else {
            unscan(token, stream);
            break;
        }
    }
    if (!specifier) {
        if (!sign) {
            switch (atype) {
                case char_t: atype = unsigned_char_t; break;
                case short_t: atype = unsigned_short_t; break;
                case int_t: atype = unsigned_int_t; break;
                case long_t: atype = unsigned_long_t; break;
                case long_long_t: atype = unsigned_long_long_t; break;
            }
        }
        specifier = arithmetic_specifier_init(atype);
    }
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

list *parse_parameter_list(FILE *stream, struct namespace *namespace, int id_must)
{
    list *retptr = list_node();
    int abstract = id_must ? 0 : 2;
    int size = 0;
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, ")"))
        return retptr;
    while (1) {
        list *specifiers = parse_specifier(stream, namespace);
        void *storage = specifiers->next->content;
        void *specifier = specifiers->next->next->content;
        declarator *d = parse_declarator(stream, abstract, namespace, 0);
        list *type_list = d->type_list, *ptr = type_list;
        char *id = d->id;
        while (ptr->next)
            ptr = ptr->next;
        ptr->content = specifier;
        if (id_must) {  // is function definition
            parameter_storage *p_storage = parameter_storage_init();
            p_storage->address = size;
            switch (type(type_list->content)) {
                case array_t: case function_t: case pointer_t: size += 4; break;
                case arithmetic_specifier_t:
                    switch (((arithmetic_specifier *)type_list->content)->atype) {
                        case char_t: case unsigned_char_t: case short_t: case unsigned_short_t:
                        case int_t: case unsigned_int_t: case long_t: case unsigned_long_t:
                            size += 4; break;
                        case long_long_t: case unsigned_long_long_t:
                            size += 8; break;
                    }
                    break;
            }
            storage = p_storage;
        }
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
declarator *parse_declarator(FILE *stream, int abstract, struct namespace *namespace, int is_funcdef)
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
        declarator *dptr = parse_declarator(stream, abstract, namespace, 0);
        token = scan(stream); // token be ")" else error
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

    int id_must = is_funcdef ? 1 : 0;  // whether parameters need an id
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            token = scan(stream);
            if (!strcmp(token, "]")) {
                type_list->prev = list_init(array_init(0));
                tmp = type_list;
                type_list = type_list->prev;
                type_list->next = tmp;
            }
            else {
                unscan(token, stream);
                type_list->prev = list_init(array_init(parse_conditional(stream, namespace)));
                tmp = type_list;
                type_list = type_list->prev;
                type_list->next = tmp;
                token = scan(stream);
            }
        }
        else if (!strcmp(token, "(")) {
            type_list->prev = list_init(function_init(parse_parameter_list(stream, namespace, id_must)));
            tmp = type_list;
            type_list = type_list->prev;
            type_list->next = tmp;
            token = scan(stream);
        }
        else {
            unscan(token, stream);
            return declartor_init(id, type_list);
        }
    }
}

static int address_int(void *address)
{
    switch (type(address)) {
        case arithmetic_t:
            return atoi(((arithmetic *)address)->value);
        case unary_t:
            return (((unary *)address)->op == "+") ? address_int(((unary *)address)->expr) \
                         : -address_int(((unary *)address)->expr);
    }
}

int isize(list *type_list)
{
    list *ptr;
    int value = 1;
    for (ptr = type_list; ptr->next; ptr = ptr->next) {
        switch (type(ptr->content)) {
            case pointer_t:
                return value * 4;
            case array_t:
                value *= address_int(((array *)ptr->content)->size);
                break;
        }
    }
    switch (type(ptr->content)) {
        case arithmetic_specifier_t: {
            arithmetic_specifier *specifier = ptr->content;
            switch (specifier->atype) {
                case char_t: case unsigned_char_t: return value * 1;
                case short_t: case unsigned_short_t: return value * 2;
                case int_t: case unsigned_int_t: return value * 4;
                case long_long_t: case unsigned_long_long_t: return value * 8;
            }
            break;
        }
        case struct_specifier_t: {
            int last_value = 0;
            struct_specifier *specifier = ptr->content;
            list *p;
            for (p = specifier->declaration_list->next; p; p = p->next)
                last_value += isize(get_type_list(p->content));
            return value * last_value;
        }
    }
}

void *parse_declaration(FILE *stream, namespace_t *namespace, int in_struct)
{
    list *specifiers = parse_specifier(stream, namespace);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return 0;
    unscan(token, stream);
    while (1) {
        declarator *dptr = parse_declarator(stream, 0, namespace, 0);
        list *type_list = dptr->type_list;
        list *ptr = type_list;
        while (ptr->next)
            ptr = ptr->next;
        ptr->content = specifier;
        if (type(specifier) == struct_specifier_t) {
            list *ptr;
            struct_specifier *s = specifier;
            if (type(storage) == auto_storage_t) {
                for (ptr = s->declaration_list->next; ptr; ptr = ptr->next) {
                    declaration *d = ptr->content;
                    d->storage = auto_storage_init();
                }
            }
            else if (type(storage) == static_storage_t) {
                for (ptr = s->declaration_list->next; ptr; ptr = ptr->next) {
                    declaration *d = ptr->content;
                    d->storage = static_storage_init();
                }
            }
        }
        char *id = dptr->id;
        declaration *node;
        if (in_struct) {
            node = declaration_init(id, 0, type_list);
            list_append(namespace->declaration_list, node);
        }
        else if (type(storage) == auto_storage_t) {
            auto_storage *a_storage = auto_storage_init();
            a_storage->address = namespace->auto_size;
            node = declaration_init(id, a_storage, type_list);
            namespace->auto_size = BINARY(add, namespace->auto_size, SIZE(node->type_list));
            list_append(namespace->declaration_list, node);
        }
        else if (type(storage) == static_storage_t) {
            static_storage *s_storage = static_storage_init();
            s_storage->address = data_size;
            node = declaration_init(id, s_storage, type_list);
            data_size += isize(node->type_list);
            list_append(namespace->declaration_list, node);
        }

        token = scan(stream);
        if (!strcmp(token, ","))
            continue;
        else if (!strcmp(token, ";"))
            return 0;
    }
}

list *parse_type_name(FILE *stream, struct namespace *namespace)
{
    list *specifiers = parse_specifier(stream, namespace);
    void *specifier = specifiers->next->next->content;
    declarator *dptr = parse_declarator(stream, 1, namespace, 0);
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
    if (is_int(token))
        return ARITHMETIC(token, int_t);
    else if (is_char(token))
        return ARITHMETIC(token, char_t);
    else if (is_float(token))
        return ARITHMETIC(token, float_t);
    else if (is_str(token)) {
        char *value = token + 1;
        value[strlen(value)] = 0;
        string *retptr = STRING(data_size, value);
        data_size += strlen(value)+1;
        return retptr;
    }
    else if (is_id(token)) {
        declaration *d = find_declaration(namespace, token);
        if (d)
            return d;
    }
}

void *parse_postfix(FILE *stream, namespace_t *namespace)
{
    void *primary = parse_primary(stream, namespace);
    char *token;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            int t = type(get_type_list(primary)->content);
            if (t == array_t || t == pointer_t) {
                primary = ARRAY_REF(primary, parse_expression(stream, namespace));
                token = scan(stream);
            }
        }
        else if (!strcmp(token, ".")) {
            token = scan(stream);
            primary = STRUCT_REF(primary, token);
        }
        else if (!strcmp(token, "->")) {
            token = scan(stream);
            primary = STRUCT_REF(INDIRECTION(primary), token);
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
            // type_name or exression;
        }
        else {
            unscan(token, stream);
            return SIZE(parse_unary(stream, namespace));
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
        list *type_list = parse_type_name(stream, namespace);
        token = scan(stream);
        if (!strcmp(token, ")"))
            return CAST(type_list, parse_cast(stream, namespace));
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
void **arithmetic_convertion(void *left, void *right)
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
                void **ptr = arithmetic_convertion(expr, expr2);
                expr = BINARY(btype, *ptr, *(ptr+1));
            }
        }
        else if (!strcmp(token, "%")) {
            void *expr2 = parse_cast(stream, namespace);
            if (type_is_int(expr) && type_is_int(expr2)) {
                void **ptr = arithmetic_convertion(expr, expr2);
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
                void **ptr = arithmetic_convertion(expr, expr2);
                expr = BINARY(btype, *ptr, *(ptr+1));
            }
            else if ((get_type(expr) == pointer_t || get_type(expr) == array_t) && type_is_int(expr2))
                expr = BINARY(btype, expr, BINARY(mul, expr2, SIZE2(INDIRECTION(expr))));
            else if ((get_type(expr2) == pointer_t || get_type(expr) == array_t) && type_is_int(expr))
                expr = BINARY(btype, expr2, BINARY(mul, expr, SIZE2(INDIRECTION(expr2))));
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
                void **ptr = arithmetic_convertion(expr, expr2);
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
                void **ptr = arithmetic_convertion(expr, expr2);
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
                void **ptr = arithmetic_convertion(expr, expr2);
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
                void **ptr = arithmetic_convertion(expr, expr2);
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
                void **ptr = arithmetic_convertion(expr, expr2);
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

void *parse_logical_or(FILE *stream, namespace_t *namespace) {
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

void *parse_conditional(FILE *stream, namespace_t *namespace) {
    return parse_logical_or(stream, namespace);
}

void *parse_assignment(FILE *stream, namespace_t *namespace) {
    void *expr = parse_conditional(stream, namespace);
    char *token = scan(stream);
    if (!strcmp(token, "=")) {
        void *expr2 = parse_assignment(stream, namespace);
        assignment *retptr = (assignment *)malloc(sizeof(assignment));
        retptr->type = assignment_t;
        retptr->expr1 = expr;
        retptr->expr2 = expr2;
        return retptr;
    }
    else {
        unscan(token, stream);
        return expr;
    }
}

void *parse_expression(FILE *stream, namespace_t *namespace) {
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

void *parse_expression_stmt(FILE *stream, namespace_t *namespace) {
    list *assignment_list = list_node();
    char *token;
    while (1) {
        list_append(assignment_list, parse_assignment(stream, namespace));
        token = scan(stream);
        if (!strcmp(token, ";"))
            return EXPRESSION_STMT(assignment_list);
    }
}

void *parse_statement(FILE *stream, struct namespace *namespace)
{
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, "{"))
        return parse_compound_stmt(stream, namespace, 0);
    else
        return parse_expression_stmt(stream, namespace);
}

void *parse_declaration_or_statement(FILE *stream, namespace_t *namespace)
{
    char *token = scan(stream);
    unscan(token, stream);
    if (is_specifier(token) || is_storage(token) || is_qualifier(token))
        return parse_declaration(stream, namespace, 0);
    return parse_statement(stream, namespace);
}

void *parse_compound_stmt(FILE *stream, namespace_t *namespace, list *parameter_list)
{
    char *token = scan(stream);
    if (strcmp(token, "{")) /* error */;
    struct namespace *i_namespace = namespace_init(namespace);
    if (parameter_list)
        i_namespace->declaration_list = parameter_list;
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

static function_definition_t *function_definition_init(char *id, list *return_type_list, list *parameter_list, compound_stmt *body)
{
    function_definition_t *retptr = (function_definition_t *)malloc(sizeof(function_definition_t));
    retptr->id = id;
    retptr->return_type_list = return_type_list;
    retptr->parameter_list = parameter_list;
    retptr->body = body;
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

function_definition_t *parse_function_definition(FILE *stream, namespace_t *namespace)
{
    list *specifiers = parse_specifier(stream, namespace);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    declarator *dcl = parse_declarator(stream, 0, namespace, 1);
    char *id = dcl->id;
    list *type_list = dcl->type_list;
    list *ptr = type_list;
    while (ptr->next)
        ptr = ptr->next;
    ptr->content = specifier;
    list *parameter_list = ((function *)type_list->content)->parameter_list;
    list *return_type_list = type_list->next;
    return_type_list->prev = 0;
    compound_stmt *body = parse_compound_stmt(stream, namespace, list_copy(parameter_list));
    return function_definition_init(id, return_type_list, parameter_list, body);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        puts("<usage> ./scan infile outfile");
        exit(0);
    }

    char *ifile = argv[1], *ofile = argv[2];

    FILE *istream = fopen(ifile, "r");
    FILE *ostream = fopen(ofile, "w");
    data_buff = buff_init();
    bss_buff = buff_init();
    text_buff = buff_init();
    data_size = 0;
    namespace = namespace_init(0);
    tab = 0;

    function_definition_t *function_definition = parse_function_definition(istream, namespace);
    function_definition_gencode(function_definition);
    char *data_section = buff_puts(data_buff);
    char *bss_section = buff_puts(bss_buff);
    char *text_section = buff_puts(text_buff);

    write_code("section .data:\n", ostream);
    write_code(data_section, ostream);
    write_code("section .bss:\n", ostream);
    write_code(bss_section, ostream);
    write_code("section .text:\n", ostream);
    write_code(text_section, ostream);

    fclose(istream);
    fclose(ostream);

    return 0;
}