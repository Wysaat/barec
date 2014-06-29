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

/* in the returned list, first if the storage specifier, the second is the type specifier */
list *parse_specifier(FILE *stream) {
    char *token;
    void *storage, *specifier;
    list *retptr = list_node();
    int size, stype, sign = 0;
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
        else {
            unscan(token, stream);
            break;
        }
    }
    if (stype == 0) {
        int_specifier *_specifier = (int_specifier *)malloc(sizeof(int_specifier));
        _specifier->type = int_specifier_t;
        _specifier->sign = sign;
        _specifier->size = size;
        specifier = _specifier;
    }
    list_append(storage);
    list_append(specifier);
    return retptr;
}

char *get_declarator_id(void *vptr) {
    return *(char **)((int *)vptr+1);
}

declarator *parse_declarator(FILE *stream) {
    char *token;
    declarator *retptr = (declarator *)malloc(sizeof(declarator));
    retptr->type = declarator_t;
    retptr-pointers = 0;
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
    retptr->id = get_declarator_id(retptr->direct_declarator);
    return retptr;
}

void *parse_direct_declarator(FILE *stream) {
    char *token = scan(stream);
    void *_declarator;
    if (!strcmp(token, "(")) {
        void *tmp = parse_declarator(stream);
        token = scan(stream);
        if (!strcmp(token, ")"))
            _declarator = tmp;
    }
    else if (is_id(token)) {
        id_declarator *tmp = (id_declarator *)malloc(sizeof(id_declarator));
        tmp->type = id_declarator_t;
        tmp->id = token;
        _declarator = tmp;
    }
    char *id = get_declarator_id(_declarator);
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
            unscan(token);
            return _declarator;
        }
    }
}

expression_stmt *parse_declaration(FILE *stream) {
    list *specifiers = parse_specifier(stream);
    void *storage = specifiers->next->content;
    void *specifier = specifiers->next->next->content;
    declarator *dptr;
    char *token;
    list *assignment_list = list_node();
    expression_stmt *retptr;
    while (1) {
        declaration *node = (declaration *)malloc(sizeof(declaration));
        node->type = declaration_t;
        node->storage = storage;
        node->specifier = specifier;
        node->declarator = parse_declarator(stream);
        node->id = get_declarator_id(node->declarator);
        list_append(declaration_list, node);
        if (type(storage) == auto_storage_t) {
            if (dptr->pointers)
                stack_pointer += 4;
            else {
                if (type(specifier) == int_specifier_t)
                    stack_pointer += ((int_specifier *)specifier)->size;
            }
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
        for (ptr = declaration_list->next; ptr; ptr = ptr->next) {
            declaration *node = (declaration *)ptr->content;
            if (!strcmp(token, node->id))
                return node;
        }
    }
}

void *parse_postfix(FILE *stream) {
    void *primary = parse_primary(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "[")) {
            void *expr2 = parse_expression(stream);
            char *token = scan(stream);
            if (!strcmp(token, "]")) {
                array_ref *expr = (array_ref *)malloc(sizeof(array_ref));
                expr->type = array_ref_t;
                expr->primary = primary;
                expr->expr2 = expr2;
                primary = expr;
            }
        }
        else {
            unscan(token, stream);
            return primary;
        }
    }
}

void *parse_unary(FILE *stream) {
    void *expr = parse_postfix(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*")) {
            indirection *new_expr = (indirection *)malloc(sizeof(indirection));
            new_expr->type = indirection_t;
            new_expr->expr = expr;
            expr = new_expr;
        }
        else {
            unscan(token, stream);
            return expr;
        }
    }
}

void *parse_conditional(FILE *stream) {
    return parse_unary(stream);
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

    buffer *buff = buff_init();
    expression_stmt *stmt = parse_declaration(istream);
    if (stmt) {
        genCode(stmt, buff);
        code = buff_puts(buff);
        fwrite(code, 1, strlen(code), ostream);
    }

    buffer *buff2 = buff_init();
    stmt = parse_expression_stmt(istream);
    if (stmt) {
        genCode(stmt, buff2);
        code = buff_puts(buff2);
        fwrite(code, 1, strlen(code), ostream);
    }

    fclose(istream);
    fclose(ostream);
    printf("stack pointer is %d\n", stack_pointer);
    return 0;
}