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

declarator *parse_declarator(FILE *stream) {
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
    list *assignment_list = list_node();
    while (1) {
        dptr = parse_declarator(stream);
        declaration_node *node = (declaration_node *)malloc(sizeof(declaration_node));
        node->pointers = dptr->pointers;
        node->id = dptr->id;
        node->specifier = specifier;
        list_append(declarations, node);
        if (dptr->pointers)
            stack_pointer += 4;
        else {
            if (type(specifier) == int_specifier_t)
                stack_pointer += ((int_specifier *)specifier)->size;
        }
        token = scan(stream);
        if (!strcmp(token, "=")) {
            unscan(token, stream);
            unscan(dptr->id, stream);
            list_append(assignment_list, parse_assignment(stream));
            token = scan(stream);
            if (strcmp(token, ",")) {
                unscan(token, stream);
                return assignment_list;
            }
        }
        else if (strcmp(token, ",")) {
            unscan(token, stream);
            return assignment_list;
        }
    }
}

void *parse_primary(FILE *stream) {
    char *token = scan(stream);
    if (is_int(token)) {
        integer *retptr = (integer *)malloc(sizeof(integer));
        retptr->type = integer_t;
        retptr->value = token;
        return retptr;
    }
}

void *parse_assignment(FILE *stream) {
}

int main()
{
    return 0;
}