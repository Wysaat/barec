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
        else if (ch >= '0' && ch <= '9') {
            i = 0;
            buffer[i++] = ch;
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

/* in the returned list, first is the storage specifier, the second is the type specifier */
list *parse_specifier(FILE *stream) {
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
                    list *_declaration_list = list_node();
                    while (strcmp((token = scan(stream)), "}")) {
                        unscan(token, stream);
                        parse_declaration(stream, _declaration_list, 1);
                    }
                    specifier = struct_specifier_init(id, _declaration_list);
                    list_append(struct_s_list, specifier);
                }
                else {
                    unscan(token, stream);
                    list *ptr;
                    /* CAUTION: list has an empty head */
                    for (ptr = struct_s_list->next; ptr; ptr = ptr->next) {
                        struct_specifier *_specifier = ptr->content;
                        if (!strcmp(id, _specifier->id))
                            specifier = _specifier;
                    }
                }
            }
            else if (!strcmp(token, "{")) {
                list *_declaration_list = list_node();
                while (strcmp((token = scan(stream)), "}")) {
                    unscan(token, stream);
                    parse_declaration(stream, _declaration_list, 1);
                }
                specifier = struct_specifier_init(0, _declaration_list);
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

declarator *parse_declarator(FILE *stream) {
    char *token;
    list *type_list = list_node(), *tmp;
    char *id = 0;

    while (1) {
        token = scan(stream);
        if (!strcmp(token, "*")) {
            type_list->prev = list_init(pointer_init());
            type_list = type_list->prev;
        }
        else {
            unscan(token, stream);
            break;
        }
    }

    token = scan(stream);
    if (!strcmp(token, "(")) {
        declarator *dptr = parse_declarator(stream);
        id = dptr->id;
        list *nlist = dptr->type_list;
        if (nlist->next) {
            list *ptr = nlist;
            while (ptr->next)
                ptr = ptr->next;
            ptr->prev->next = type_list;
            free(ptr);
            type_list = nlist;
        }
    }
    else if (is_id(token))
        id = token;

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
                type_list->prev = list_init(array_init(parse_conditional(stream)));
                tmp = type_list;
                type_list = type_list->prev;
                type_list->next = tmp;
                token = scan(stream);
            }
        }
        else {
            unscan(token, stream);
            return declartor_init(id, type_list);
        }
    }
}

static int address_int(void *address) {
    switch (type(address)) {
        case arithmetic_t:
            return atoi(((arithmetic *)address)->value);
        case unary_t:
            return (((unary *)adress)->op == "+") ? address_int(((unary *)address)->expr) \
                         : -address_int(((unary *)address)->expr);
        case m_expr_t:
            return address_int(((m_expr *)address)->left) * address_int(((m_expr *)address)->right);
        case a_expr_t:
            return address_int(((m_expr *)address)->left) + address_int(((m_expr *)address)->right);
    }
}

int isize(list *type_list) {
    list *ptr;
    int value = 1;
    for (ptr = type_list; ptr; ptr = ptr->next) {
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
            value *= atoi(specifier->value);
            break;
        }
        case struct_specifier_t: {
            struct_specifier *specifier = ptr->content;
            list *p;
            for (p = specifier->declaration_list->next; p; p = p->next)
                value += isize(get_type_list(p->content));
        }
    }
    return M_EXPR("*", retptr, last_size);
}

void parse_declaration(FILE *stream, list *declaration_list, int in_struct) {
    list *specifiers = parse_specifier(stream);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    while (1) {
        declarator *dptr = parse_declarator(stream);
        list *type_list = dptr->type_list;
        if (type_list->next) {
            list *ptr = type_list;
            while (ptr->next)
                ptr = ptr->next;
            ptr->prev->next->content = specifier;
        }
        else
            type_list->content = specifier;
        if (type(specifier == struct_specifier_t)) {
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
            node = declaration_init(id, a_storage, type_list);
            list_append(declaration_list, node);
        }
        else if (type(storage) == auto_storage_t) {
            auto_storage *a_storage = auto_storage_init();
            a_storage->address = auto_size;
            node = declaration_init(id, a_storage, type_list);
            auto_size = A_EXPR("+", auto_size, SIZE(node->type_list));
            list_append(declaration_list, node);
        }
        else if (type(storage) == static_storage_t) {
            static_storage *s_storage = static_storage_init();
            s_storage->address = data_size;
            node = declaration_init(id, s_storage, type_list);
            data_size += isize(node->type_list);
            list_append(declaration_list, node);
        }

        token = scan(stream);
        if (!strcmp(token, ","))
            continue;
        else if (!strcmp(token, ";"))
            return;
    }
}

list *parse_type_name(FILE *stream) {
    list *specifiers = parse_specifier(stream);
    void *specifier = specifiers->next->next->content;
    declarator *dptr = parse_declarator(stream);
    list *type_list = dptr->type_list;
    list *ptr = type_list;
    while (ptr->next)
        ptr = ptr->next;
    ptr->prev->next = specifier;
    return type_list;
}

void *parse_primary(FILE *stream) {
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
        list *ptr;
        for (ptr = declaration_list->next; ptr; ptr = ptr->next) {
            declaration *node = (declaration *)ptr->content;
            if (!strcmp(token, node->id))
                return node;
        }
    }
}

void *parse_postfix(FILE *stream) {
    void *primary = parse_primary(stream);
    char *token;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            int t = type(get_type_list(primary)->content);
            if (t == array_t || t == pointer_t) {
                primary = ARRAY_REF(primary, parse_expression(stream));
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

void *parse_unary(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "++"))
        return PREINC(parse_unary(stream), 1);
    else if (!strcmp(token, "--"))
        return PREINC(parse_unary(stream), 0);
    else if (!strcmp(token, "&"))
        return ADDR(parse_cast(stream));
    else if (!strcmp(token, "*"))
        return INDIRECTION(parse_cast(stream));
    else if (!strcmp(token, "+") || !strcmp(token, "-")
                   || !strcmp(token, "~") || !strcmp(token, "!"))
        return UNARY(parse_cast(stream), token);
    else if (!strcmp(token, "sizeof")) {
        token = scan(stream);
        if (!strcmp(token, "(")) {
            // type_name or exression;
        }
        else {
            unscan(token, stream);
            return SIZE(parse_unary(stream));
        }
    }
    else {
        unscan(token, stream);
        return parse_postfix(stream);
    }
}

void *parse_cast(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        list *type_list = parse_type_name(stream);
        token = scan(stream);
        if (!strcmp(token, ")"))
            return CAST(type_list, parse_cast(stream));
    }
    else {
        unscan(token, stream);
        return parse_unary(stream);
    }
}

void *parse_m_expr(FILE *stream) {
    void *expr = parse_cast(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%"))
            expr = M_EXPR(token, expr, parse_cast(stream));
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_a_expr(FILE *stream) {
    void *expr = parse_m_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+")|| !strcmp(token, "-"))
            expr = A_EXPR(token, expr, parse_m_expr(stream));
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_conditional(FILE *stream) {
    return parse_a_expr(stream);
}

void *parse_assignment(FILE *stream) {
    void *expr = parse_conditional(stream);
    char *token = scan(stream);
    if (!strcmp(token, "=")) {
        void *expr2 = parse_assignment(stream);
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

void *parse_expression(FILE *stream) {
    list *assignment_list = list_node();
    char *token;
    while (1) {
        void *assignment = parse_assignment(stream);
        list_append(assignment_list, assignment);
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            return EXPRESSION(assignment_list, get_type_list(assignment));
        }
    }
}

void *parse_expression_stmt(FILE *stream) {
    list *assignment_list = list_node();
    char *token;
    while (1) {
        list_append(assignment_list, parse_assignment(stream));
        token = scan(stream);
        if (!strcmp(token, ";")) {
            expression_stmt *retptr = (expression_stmt *)malloc(sizeof(expression_stmt));
            retptr->type = expression_stmt_t;
            retptr->assignment_list = assignment_list;
            return retptr;
        }
    }
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
    auto_size = ARITHMETIC(strdup("0"), int_t);
    data_size = 0;
    struct_s_list = list_node();
    declaration_list = list_node();

    parse_declaration(istream, declaration_list, 0);
    parse_declaration(istream, declaration_list, 0);
    // void *stmt = parse_expression_stmt(istream);
    // gencode(stmt);
    // stmt = parse_expression_stmt(istream);
    // gencode(stmt);
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