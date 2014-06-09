#include "barec.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "scan.h"
// #include "list.h"

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

void *parse_declaration_specifier(FILE *stream) {
    int storage_class;
    int type_qualifier = 0;
    void *type_specifier = 0;
    char *token;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "auto"))
            storage_class = 1;
        else if (!strcmp(token, "register"))
            storage_class = 2;
        else if (!strcmp(token, "static"))
            storage_class = 3;
        else if (!strcmp(token, "extern"))
            storage_class = 4;
        else if (!strcmp(token, "const"))
            type_qualifier |= 0b01;
        else if (!strcmp(token, "volatile"))
            type_qualifier |= 0b10;
        else if (!strcmp(token, "unsigned"))
            ;
        else if (!strcmp(token, "signed")) {
            if (type_specifier)
                ((integral_type_specifier *)type_specifier)->sign = 1;
            else {
                type_specifier = integral_type_specifier_init();
                ((integral_type_specifier *)type_specifier)->sign = 1;
            }
        }
        else if (!strcmp(token, "char")) {
            if (type_specifier)
                ((integral_type_specifier *)type_specifier)->size = 8;
            else {
                type_specifier = integral_type_specifier_init();
                ((integral_type_specifier *)type_specifier)->size = 8;
            }
        }
        else if (!strcmp(token, "short")) {
            if (type_specifier)
                ((integral_type_specifier *)type_specifier)->size = 16;
            else {
                type_specifier = integral_type_specifier_init();
                ((integral_type_specifier *)type_specifier)->size = 8;
            }
        }
        else if (!strcmp(token, "int")) {
            if (type_specifier)
                if (!((integral_type_specifier *)type_specifier)->size)
                    ((integral_type_specifier *)type_specifier)->size = 32;
            else {
                type_specifier = integral_type_specifier_init();
                ((integral_type_specifier *)type_specifier)->size = 8;
            }
        }
        else if (!strcmp(token, "long")) {
            if (type_specifier)
                ((integral_type_specifier *)type_specifier)->size = 64;
            else {
                type_specifier = integral_type_specifier_init();
                ((integral_type_specifier *)type_specifier)->size = 8;
            }
        }
        else {
            unscan(token, stream);
            return declaration_specifier_init(storage_class, type_qualifier, type_specifier);
        }
    }
}

void *parse_init_declarator(FILE *stream) {
    init_declarator *retptr = (init_declarator *)malloc(sizeof(init_declarator));
    retptr->declarator = (declarator *)malloc(sizeof(declarator));
    retptr->declarator->pointers = 0;
    char *token;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "*"))
            retptr->declarator->pointers++;
        else {
            unscan(token, stream);
            break;
        }
    }
    token = scan(stream);
    if (is_id(token)) {
        retptr->declarator->id = token;
        token = scan(stream);
        if (!strcmp(token, "="))
            retptr->initializer = parse_assignment(stream);
        else {
            unscan(token, stream);
            retptr->initializer = 0;
        }
        return retptr;
    }
}

list *parse_init_declarator_list(FILE *stream) {
    list *init_declarator_list = list_node();
    char *token;
    while (1) {
        list_append(init_declarator_list, parse_init_declarator(stream));
        token = scan(stream);
        if (!strcmp(token, ";")) {
            unscan(token, stream);
            return init_declarator_list;
        }
    }
}

void *parse_declaration(FILE *stream) {
    void *declaration_specifier = parse_declaration_specifier(stream);
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return declaration_init(declaration_specifier, 0);
    unscan(token, stream);
    list *init_declarator_list = parse_init_declarator_list(stream);
    token = scan(stream);
    if (!strcmp(token, ";"))
        return declaration_init(declaration_specifier, init_declarator_list);
}


void *parse_assignment(FILE *stream) {
}

// void *parse_expr_stmt(FILE *stream) {
//     char *token = scan(stream);
//     if (!strcmp(token, ";"))
//         return 0;
//     else {
//         unscan(token, stream);
//         void *retptr = pare_expression(stream);
//         token = scan(stream);
//         if (!strcmp(token, ";"))
//             return retptr;
//     }
// }

void *parse_stmt(FILE *stream) {
    parse_expr_stmt(stream);
}

void *parse_compound_stmt(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "{")) {
        list *stmt_list = list_node();
        while (1) {
            token = scan(stream);
            if (!strcmp(token, "}"))
                return compund_stmt_init(stmt_list);
            unscan(token, stream);
            if (is_sto_class_spe(token) || is_type_spe(token) || is_type_qua(token))
                list_append(stmt_list, parse_declaration(stream));
            else
                list_append(stmt_list, parse_stmt(stream));
        }
    }
}