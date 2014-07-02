#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "list.h"
#include "utils.h"
#include "types.h"

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
    void *storage, *specifier = 0;
    list *retptr = list_node();
    int size = 0, stype, sign = 0;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "auto")) {
            storage = (auto_storage *)malloc(sizeof(auto_storage));
            ((auto_storage *)storage)->type = auto_storage_t;
            ((auto_storage *)storage)->stack_position = stack_pointer;
        }
        else if (!strcmp(token, "char")) {
            stype = 0; // int_specifier
            size = 1;
        }
        else if (!strcmp(token, "int")) {
            stype = 0; // int_specifier
            size = 4;
        }
        else if (!strcmp(token, "unsigned")) {
            stype = 0;
            sign = 1;
        }
        else if (!strcmp(token, "signed")) {
            stype = 0;
            sign = 1;
        }
        else if (!strcmp(token, "struct")) {
            struct_specifier *_specifier;
            token = scan(stream);
            if (is_id(token)) {
                char *id = token;
                token = scan(stream);
                if (!strcmp(token, "{")) {
                    lits *_declaration_list = list_node();
                    while (strcmp((token = scan(stream)), "}")) {
                        unscan(token, stream);
                        parse_declaration(stream, _declaration_list);
                    }
                    _specifier = (struct_specifier *)malloc(sizeof(struct_specifier));
                    _specifier->type = struct_specifier;
                    _specifier->id = id;
                    _specifier->declaration_list = _declaration_list;
                }
                else {
                    unscan(token, stream);
                    list *ptr;
                    for (ptr = specifier_list; ptr; ptr = ptr->next) {
                        struct_specifier *__specifier = (struct_specifier *)ptr->content;
                        if (!strcmp(id, __specifier->id))
                            _specifier = __specifier;
                    }
                }
            }
            else if (!strcmp(token, "{")) {
                lits *_declaration_list = list_node();
                while (strcmp((token = scan(stream)), "}")) {
                    unscan(token, stream);
                    parse_declaration(stream, _declaration_list);
                }
                _specifier = (struct_specifier *)malloc(sizeof(struct_specifier));
                _specifier->type = struct_specifier;
                _specifier->id = 0;
                _specifier->declaration_list = _declaration_list;
            }
            specifier = _specifier;
        }
        else {
            unscan(token, stream);
            break;
        }
    }
    if (!specifier) {
        if (stype == 0) {
            int_specifier *_specifier = (int_specifier *)malloc(sizeof(int_specifier));
            _specifier->type = int_specifier_t;
            _specifier->sign = sign;
            _specifier->size = size;
            specifier = _specifier;
        }
    }
    list_append(retptr, storage);
    list_append(retptr, specifier);
    return retptr;
}

declarator *parse_declarator(FILE *stream) {
    char *token;
    declarator *retptr = (declarator *)malloc(sizeof(declarator));
    retptr->pointers = 0;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "*"))
            retptr->pointers++;
        else {
            unscan(token, stream);
            break;
        }
    }
    retptr->direct_declarator = parse_direct_declarator(stream);
    void *vptr = retptr->direct_declarator;
    if (type(vptr) == declarator_t)
        retptr->id = ((declarator *)vptr)->id;
    else if (type(vptr) == id_declarator_t)
        retptr->id = ((id_declarator *)vptr)->id;
    else if (type(vptr) == array_declarator_t)
        retptr->id = ((array_declarator *)vptr)->id;
    return retptr;
}

void *parse_direct_declarator(FILE *stream) {
    char *token = scan(stream);
    void *_declarator;
    char *id;
    if (!strcmp(token, "(")) {
        declarator *tmp = parse_declarator(stream);
        token = scan(stream);
        if (!strcmp(token, ")"))
            _declarator = tmp;
        id = tmp->id;
    }
    else if (is_id(token)) {
        id_declarator *tmp = (id_declarator *)malloc(sizeof(id_declarator));
        tmp->type = id_declarator_t;
        tmp->id = token;
        _declarator = tmp;
        id = tmp->id;
    }
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            array_declarator *tmp = (array_declarator *)malloc(sizeof(array_declarator));
            tmp->type = array_declarator_t;
            tmp->id = id;
            tmp->direct_declarator = _declarator;
            token = scan(stream);
            if (!strcmp(token, "]")) {
                tmp->size_expr = 0;
            }
            else {
                unscan(token, stream);
                void *expr = parse_conditional(stream);
                token = scan(stream);
                if (!strcmp(token, "]")) {
                    tmp->size_expr = expr;
                }
            }
            _declarator = tmp;
        }
        else {
            unscan(token, stream);
            return _declarator;
        }
    }
}

expression_stmt *parse_declaration(FILE *stream, list *declaration_list) {
    list *specifiers = parse_specifier(stream);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    char *token;
    list *assignment_list = list_node();
    expression_stmt *retptr;
    while (1) {
        declaration *node = (declaration *)malloc(sizeof(declaration));
        node->type = declaration_t;
        node->storage = storage;
        node->specifier = specifier;
        node->declarator = parse_declarator(stream);
        node->id = node->declarator->id;
        if (node->dec)
        list_append(declaration_list, node);
        if (type(storage) == auto_storage_t) {
            if (node->declarator->pointers)
                stack_pointer += 4;
            else {
                if (type(specifier) == int_specifier_t)
                    stack_pointer += ((int_specifier *)specifier)->size;
            }
        }
        token = scan(stream);
        if (!strcmp(token, "=")) {
            unscan(token, stream);
            unscan(node->declarator->id, stream);
            list_append(assignment_list, parse_assignment(stream));
            token = scan(stream);
            if (!strcmp(token, ";")) {
                retptr = (expression_stmt *)malloc(sizeof(expression_stmt));
                retptr->type = expression_stmt_t;
                retptr->assignment_list = assignment_list;
                return retptr;
            }
        }
        else if (!strcmp(token, ";")) {
            retptr = (expression_stmt *)malloc(sizeof(expression_stmt));
            retptr->type = expression_stmt_t;
            retptr->assignment_list = assignment_list;
            return retptr;
        }
    }
}

void *parse_primary(FILE *stream) {
    char *token = scan(stream);
    if (is_int(token)) {
        arithmetic *retptr = (arithmetic *)malloc(sizeof(arithmetic));
        retptr->type = arithmetic_t;
        arithmetic_specifier *specifier = (arithmetic_specifier *)malloc(sizeof(arithmetic_specifier));
        specifier->type = arithmetic_specifier_t;
        specifier->atype = int_t;
        retptr->specifier = specifier;
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
    return primary;
}

void *parse_unary(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "*")) {
        indirection *expr = (indirection *)malloc(sizeof(indirection));
        expr->type = indirection_t;
        expr->expr = parse_unary(stream);
        expr->specifier = get_specifier(expr->expr);
        return expr;
    }
    else {
        unscan(token, stream);
        return parse_postfix(stream);
    }
}

void *parse_cast(FILE *stream) {
    return parse_unary(stream);
}

void *parse_m_expr(FILE *stream) {
    void *expr = parse_cast(stream);
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%"))
            expr = M_EXPR(token, expr, parse_cast(stream));
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_conditional(FILE *stream) {
    return parse_m_expr(stream);
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
        list_append(assignment_list, parse_assignment(stream));
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            expression *retptr = (expression *)malloc(sizeof(expression));
            retptr->type = expression_t;
            retptr->assignment_list = assignment_list;
            return retptr;
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
    FILE *istream = fopen(argv[1], "r");
    FILE *ostream = fopen(argv[2], "w");
    char *code;
    stack_pointer = 0;
    declaration_list = list_node();

    expression_stmt *stmt = parse_declaration(istream);

    buffer *buff2 = buff_init();
    stmt = parse_expression_stmt(istream);
    gen_code(stmt, buff2);
    code = buff_puts(buff2);
    fwrite(code, 1, strlen(code), ostream);

    fclose(istream);
    fclose(ostream);
    printf("stack pointer is %d\n", stack_pointer);
    return 0;
}