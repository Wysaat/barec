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

expression_stmt *parse_declaration(FILE *stream) {
    void *specifier = parse_specifier(stream);
    declarator *dptr;
    char *token;
    list *assignment_list = list_node();
    expression_stmt *retptr;
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
        integer *retptr = (integer *)malloc(sizeof(integer));
        retptr->type = integer_t;
        retptr->value = token;
        return retptr;
    }
    else if (is_id(token)) {
        list *ptr;
        for (ptr = declarations->next; ptr; ptr = ptr->next) {
            declaration_node *node = (declaration_node *)ptr->content;
            if (!strcmp(node->id, token)) {
                node->type = identifier_t;
                return node;
            }
        }
    }
}

void *parse_conditional(FILE *stream) {
    return parse_primary(stream);
}

void *parse_assignment(FILE *stream) {
    void *expr = parse_conditional(stream);
    char *token = scan(stream);
    if (!strcmp(token, "=")) {
        void *expr2 = parse_assignment(stream);
        assignment *retptr = (assignment *)malloc(sizeof(assignment));
        retptr->expr1 = expr;
        retptr->expr2 = expr2;
        return retptr;
    }
    else {
        unscan(token, stream);
        return expr;
    }
}

void *parse_expression_stmt(FILE *stream) {
    list *assignment_list = list_node();
    char *token;
    while (1) {
        list_append(assignment_list, parse_assignment);
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
    buffer *buff = buff_init();
    stack_pointer = 0;
    declarations = list_node();
    expression_stmt *stmt = parse_declaration(istream);
    if (stmt) {
        genCode(stmt, buff);
        code = buff_puts(buff);
        fwrite(code, 1, strlen(code), ostream);
    }

    fclose(istream);
    fclose(ostream);
    return 0;
}