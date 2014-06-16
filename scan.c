#include "barec.h"

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

void *parse_specifier(FILE *stream) {
    char *token;
    int storage, size, stype, sign = 0;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "auto"))
            storage = 1;
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
        else {
            unscan(token, stream);
            break;
        }
    }
    if (stype == 0) {
        int_specifier *specifier = (int_specifier *)malloc(sizeof(int_specifier));
        specifier->type = int_specifier_t;
        specifier->storage = storage;
        specifier->sign = sign;
        specifier->size = size;
        return specifier;
    }
}

void *parse_declarator(FILE *stream) {
    char *token;
    int pointers;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "*"))
            pointers++;
        else {
            unscan(token, stream);
            break;
        }
    }
    token = scan(stream);
    if (is_id(token)) {
        declarator *retptr = (declarator *)malloc(sizeof(declarator));
        retptr->pointers = pointers;
        retptr->id = token;
        return retptr;
    }
}

list *parse_declaration(FILE *stream) {
    void *specifier = parse_specifier(stream);
    declarator *dptr;
    char *token;
    list *assignments = list_node();
    while (1) {
        dptr = parse_declarator(stream);
        if (dptr->pointers)
            stack_pointer += 4;
        else {
            if (type(specifier) == int_specifier_t)
                stack_pointer += ((int_specifier *)specifier)->size;
        }
        token = scan(stream);
        if (!strcmp(token, "=")) {
        }
    }
}

void *parse_pirmary_expr(FILE *stream) {
    char *token = scan(stream);
    if (is_id(token)) {
        identifier *retptr = (identifier *)malloc(sizeof(identifier));
        retptr->value = token;
        return retptr;
    }
    if (is_int(token)) {
        int_expr *retptr = (int_expr *)malloc(sizeof(int_expr));
        retptr->value = token;
        return retptr;
    }
}

void *parse_conditional_expression(FILE *stream) {
    return parse_primary_expr(stream);
}

void *parse_assignment(FILE *stream) {
    void expr =  parse_conditional_expression(stream);
    char *token = scan(stream);
    if (!strcmp(token, "=")) {
        void *retptr = (assignment *)malloc(sizeof(assignment));
        retptr->type = assignment_t;
        retptr->expr1 = expr;
        retptr->expr2 = parse_assignment_expression(stream);
        return retptr;
    }
    else {
        unscan(token, stream);
        return expr;
    }
}

void *parse_expression(FILE *stream) {
    return parse_assignment(stream);
}

void *parse_stmt(FILE *stream) {
    void *retptr = parse_expression(stream);
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return retptr;
}

void *parse_compound_stmt(FILE *stream) {
    char *token = scan(stream);
    compound_stmt *retptr = (compound_stmt *)malloc(sizeof(compound_stmt));
    retptr->stmt_list = list_node();
    if (!strcmp(token, "{")) {
        while (1) {
            token = scan(stream);
            if (!strcmp(token, "}"))
                return retptr;
            unscan(token, stream);
            if (is_storage(token) || is_type(token) || is_qualifier(token))
                list_append(retptr->stmt_list, parse_declaration(stream));
            else
                list_append(retptr->stmt_list, parse_stmt(stream));
        }
    }
}

int main()
{
    return 0;
}