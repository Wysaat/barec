// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <setjmp.h>
// #include "barec.h"

list *end_stack;

void end_stack_push(char *token, jmp_buf, *env a);
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
    list *ptr;
    printf("%s:%d:%d: error: ", file_info.file_name, file_info.line, file_info.column);
    if (message)
        printf("%s\n", message);
    else
        printf("expected '%s'\n", ((end_t *)end_stack->content)->token);
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
                printf("%s:%d:%d: error: expected '%s'\n", file_info.file_name, file_info.line, file_info.column, end->token);
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
                            printf("%s:%d:%d: error: expected '%s'\n", file_info.file_name, file_info.line, file_info.column, end->token);
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

void syntax_enumerator_body(FILE *stream)
{
    char *token = scan(stream);  // should be "{"
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
            syntax_conditional(stream);
            token = scan(stream);
            if (strcmp(token, ",") && strcmp(token, "}")) {
                unscan(token, stream);
                error(stream, "expected '}' or ','");
            }
        }

        if (!strcmp(token, ",")) {
            token = scan(stream);
            if (!strcmp(token, "}")) {
                end_stack_pop();
                end_stack_pop();
                return;
            }
            else
                unscan(token, stream);
            continue;
        }
        else if (!strcmp(token, "}")) {
            end_stack_pop();
            end_stack_pop();
            return;
        }
        t4: if (!setjmp(env2)) goto t3;
        token = scan(stream);
        if (!strcmp(token, "}"))
            return;
        unscan(token, stream);
    }
    t2: if (!setjmp(env)) goto t1;
}


/*
 * flag == 0 : internal
 * flag == 1 : external
 * flag == 2 : struct-declaration-list
 * flag == 3 : parameter list
 * flag == 4 : external-declaration
 */
void syntax_specifier(FILE *stream, int flag) {
    char *token;
    int storage = 0, specifier = 0;
    int sign = -1;
    int atype = -1;
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "typedef"))
            storage = 3;
        else if (!strcmp(token, "auto"))
            storage = 1;
        else if (!strcmp(token, "static"))
            storage = 2;
        else if (!strcmp(token, "void"))
            specifier = 2;
        else if (!strcmp(token, "char")) {
            if (atype >= 0) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            atype = char_t;
        }
        else if (!strcmp(token, "short")) {
            if (atype >= 0) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            atype = short_t;
        }
        else if (!strcmp(token, "int")) {
            if (atype >= 0) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            atype = int_t;
        }
        else if (!strcmp(token, "long")) {
            if (atype == long_t)
                atype = long_long_t;
            else if (atype == -1 || atype == int_t)
                atype = long_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "float")) {
            if (atype >= 0) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            atype = float_t;
        }
        else if (!strcmp(token, "double")) {
            if (atype == long_t)
                atype = long_double_t;
            else if (atype == -1)
                atype = double_t;
            else {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
        }
        else if (!strcmp(token, "unsigned"))
            sign = 0;
        else if (!strcmp(token, "signed"))
            sign = 1;
        else if (!strcmp(token, "struct") || !strcmp(token, "union")) {
            int is_struct = !strcmp(token, "struct");
            if (atype >= 0) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
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
                    if (!strcmp(token, "}"))
                        specifier = 1;
                    else {
                        unscan(token, stream);
                        jmp_buf env;
                        goto t2; t1: end_stack_push("}", &env);
                        while (strcmp((token = scan(stream)), "}")) {
                            unscan(token, stream);
                            syntax_declaration(stream, 2);
                        }
                        end_stack_pop(); longjmp(env, 1);
                        t2: if (!setjmp(env)) goto t1;
                    }
                }
                else
                    unscan(token, stream);
            }
            else if (!strcmp(token, "{")) {
                token = scan(stream);
                if (!strcmp(token, "}"))
                    continue;
                else {
                    unscan(token, stream);
                    jmp_buf env;
                    goto t4; t3: end_stack_push("}", &env);
                    while (strcmp((token = scan(stream)), "}")) {
                        unscan(token, stream);
                        syntax_declaration(stream, 2);
                    }
                    end_stack_pop(); longjmp(env, 1);
                    t4: if (!setjmp(env)) goto t3;
                }
            }
        }
        else if (!strcmp(token, "enum")) {
            token = scan(stream);
            if (is_id(token)) {
                char *id = token;
                token = scan(stream);
                if (!strcmp(token, "{")) {
                    unscan(token, stream);
                    syntax_enumerator_body(stream);
                }
                else
                    unscan(token, stream);
            }
            else
                syntax_enumerator_body(stream);
        }
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_parameter_list(FILE *stream)
{
    int abstract = 2;
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, ")"))
        return;
    while (1) {
        syntax_specifier(stream, 3);
        syntax_declarator(stream, abstract);
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
    printf("%s:%d:%d: error: %s\n", file_info.file_name, file_info.line, file_info.column, message);
}

/* 
 * abstract == 0 : id
 * abstract == 1 : no id
 * abstract == 2 : id or no id
 */
void syntax_declarator(FILE *stream, int abstract)
{
    char *token;
    while (1) {
        token = scan(stream);
        if (strcmp(token, "*")) {
            unscan(token, stream);
            break;
        }
    }
    token = scan(stream);
    if (!strcmp(token, "(")) {
        jmp_buf env;
        goto t2; t1: end_stack_push(")", &env);
        syntax_declarator(stream, abstract);
        token = scan(stream);
        if (strcmp(token, ")")) {
            unscan(token, stream);
            error(stream, 0);
        }
        end_stack_pop(")"); longjmp(env, 1);
        t2: if (!setjmp(env)) goto t1;
    }
    else {
        if (abstract == 0) {
            if (!is_id(token)) {
                unscan(token, stream);
                error(stream, "expected identifier or '('");
            }
        }
        else if (abstract == 1)
            unscan(token, stream);
        else if (!is_id(token))
            unscan(token, stream);
    }

    while (1) {
        token = scan(stream);
        if (!strcmp(token, "[")) {
            token = scan(stream);
            if (strcmp(token, "]")) {
                unscan(token, stream);
                jmp_buf env;
                goto t5; t4: end_stack_push("]", &env);
                syntax_conditional(stream);
                token = scan(stream);
                if (strcmp(token, "]")) {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop(); longjmp(env, 1);
                t5: if (!setjmp(env)) goto t4;
            }
        }
        else if (!strcmp(token, "(")) {
            jmp_buf env;
            goto t8; t7: end_stack_push(")", &env);
            syntax_parameter_list(stream);
            token = scan(stream);
            if (strcmp(token, ")")) {
                unscan(token, stream);
                error(stream, 0);
            }
            end_stack_pop(); longjmp(env, 1);
            t8: if (!setjmp(env)) goto t7;
        }
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_initializer(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "{")) {
        token = scan(stream);
        if (!strcmp(token, "}"))
            return;
        unscan(token, stream);
        jmp_buf env;
        goto t2; t1: end_stack_push("}", &env);
        while (1) {
            syntax_initializer(stream);
            token = scan(stream);
            if (!strcmp(token, ",")) {
                token = scan(stream);
                if (!strcmp(token, "}")) {
                    end_stack_pop();
                    return;
                }
                unscan(token, stream);
            }
            else if (!strcmp(token, "}")) {
                end_stack_pop();
                return;
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
        syntax_assignment(stream);
    }
}

/*
 * flag == 0 : internal declaration
 * flag == 1 : external declaration
 * flag == 2 : struct-declaration
 */
void syntax_declaration(FILE *stream, int flag)
{
    syntax_specifier(stream, flag);
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        syntax_declarator(stream, 0);
        token = scan(stream);
        if (!strcmp(token, ","))
            continue;
        else if (!strcmp(token, "=")) {
            syntax_initializer(stream);
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
        else if (!strcmp(token, ";"))
            return;
        else {
            unscan(token, stream);
            error(stream, 0);
        }
    }
    longjmp(env, 1);
    t2: if (!setjmp(env)) goto t1;
    return;
}

void syntax_type_name(FILE *stream)
{
    syntax_specifier(stream, 2);
    syntax_declarator(stream, 1);
}

void syntax_primary(FILE *stream)
{
    char *token = scan(stream);
    if (is_int(token))
        return;
    else if (is_char(token))
        return;
    else if (is_float(token))
        return;
    else if (is_str(token))
        return;
    else if (is_id(token))
        return;
    // else {
    //     unscan(token, stream);
    //     error_message("expected expression");
    //     error(stream, 0);
    // }
    else {
        unscan(token, stream);
        error(stream, "expected expression");
    }
}

static inline void syntax_argument_expression_list(FILE *stream)
{
    char *token;
    while (1) {
        syntax_assignment(stream);
        token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_postfix(FILE *stream)
{
    syntax_primary(stream);
    char *token;
    while (1) {
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
                syntax_expression(stream);
                token = scan(stream);
                if (strcmp(token, "]")) {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop();
                continue;
                t2: if (!setjmp(env)) goto t1;
            }
        }
        else if (!strcmp(token, "(")) {
            token = scan(stream);
            if (!strcmp(token, ")"))
                ;
            else {
                jmp_buf env;
                goto t4; t3: end_stack_push(")", &env);
                unscan(token, stream);
                syntax_argument_expression_list(stream);
                token = scan(stream);
                if (strcmp(token, ")")) {
                    unscan(token, stream);
                    error(stream, 0);
                }
                end_stack_pop();
                continue;
                t4: if (!setjmp(env)) goto t3;
            }
        }
        else if (!strcmp(token, ".")) {
            token = scan(stream);
            if (!is_id(token)) {
                unscan(token, stream);
                error(stream, "expected identifier");
            }
        }
        else if (!strcmp(token, "->")) {
            token = scan(stream);
            if (!is_id(token)) {
                unscan(token, stream);
                error(stream, "expected identifier");
            }
        }
        else if (!strcmp(token, "++"))
            ;
        else if (!strcmp(token, "--"))
            ;
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_unary(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "++"))
        syntax_unary(stream);
    else if (!strcmp(token, "--"))
        syntax_unary(stream);
    else if (!strcmp(token, "&"))
        syntax_cast(stream);
    else if (!strcmp(token, "*"))
        syntax_cast(stream);
    else if (!strcmp(token, "+") || !strcmp(token, "-")
                   || !strcmp(token, "~") || !strcmp(token, "!"))
        syntax_cast(stream);
    else if (!strcmp(token, "sizeof")) {
        token = scan(stream);
        if (!strcmp(token, "(")) {
            // type_name or expression;
        }
        else {
            unscan(token, stream);
            syntax_unary(stream);
            return;
        }
    }
    else {
        unscan(token, stream);
        syntax_postfix(stream);
    }
}

void syntax_cast(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        jmp_buf env;
        goto t2; t1: end_stack_push(")", &env);
        syntax_type_name(stream);
        token = scan(stream);
        if (!strcmp(token, ")")) {
            end_stack_pop(); goto t3;
            t2: if (!setjmp(env)) goto t1;
            t3:
            token = scan(stream);
            unscan(token, stream);
            if (!strcmp(token, ";")) {
                error_message("expected expression");
                return;
            }
            else
                syntax_cast(stream);
        }
        else {
            unscan(token, stream);
            error(stream, 0);
        }
    }
    else {
        unscan(token, stream);
        syntax_unary(stream);
    }
}

void syntax_m_expr(FILE *stream) {
    syntax_cast(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%"))
            syntax_cast(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}
void syntax_a_expr(FILE *stream) {
    syntax_m_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-"))
            syntax_m_expr(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_shift(FILE *stream) {
    syntax_a_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<<") || !strcmp(token, ">>"))
            syntax_a_expr(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_relational(FILE *stream) {
    syntax_shift(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") ||
                  !strcmp(token, "<=") || !strcmp(token, ">="))
            syntax_shift(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_equality(FILE *stream) {
    syntax_relational(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!="))
            syntax_relational(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_and(FILE *stream) {
    syntax_equality(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&"))
            syntax_equality(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_xor(FILE *stream) {
    syntax_and(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^"))
            syntax_and(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_or(FILE *stream) {
    syntax_xor(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|"))
            syntax_xor(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_logical_and(FILE *stream) {
    syntax_or(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&"))
            syntax_or(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_logical_or(FILE *stream) {
    syntax_logical_and(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||"))
            syntax_logical_and(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_conditional(FILE *stream) {
    syntax_logical_or(stream);
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        syntax_expression(stream);
        token = scan(stream);
        if (strcmp(token, ":")) {
            unscan(token, stream);
            error_message("expected ':'");
        }
        syntax_conditional(stream);
    }
    else
        unscan(token, stream);
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

void syntax_assignment(FILE *stream) {
    syntax_conditional(stream);
    char *token = scan(stream);
    if (is_assignment_operator(token))
        syntax_assignment(stream);
    else
        unscan(token, stream);
}

void syntax_expression(FILE *stream) {
    while (1) {
        syntax_assignment(stream);
        char *token = scan(stream);
        if (!strcmp(token, ","))
            continue;
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_expression_stmt(FILE *stream) {
    char *token;
    token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        syntax_assignment(stream);
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

void syntax_statement(FILE *stream)
{
    char *token = scan(stream);
    unscan(token, stream);
    if (!strcmp(token, "{"))
        syntax_compound_stmt(stream);
    else if (!strcmp(token, "if"))
        syntax_if_stmt(stream);
    else if (!strcmp(token, "switch"))
        syntax_switch_stmt(stream);
    else if (!strcmp(token, "case"))
        syntax_case_stmt(stream);
    else if (!strcmp(token, "default"))
        syntax_default_stmt(stream);
    else if (!strcmp(token, "while"))
        syntax_while_stmt(stream);
    else if (!strcmp(token, "do"))
        syntax_do_while_stmt(stream);
    else if (!strcmp(token, "for"))
        syntax_for_stmt(stream);
    else if (!strcmp(token, "goto"))
        syntax_goto_stmt(stream);
    else if (!strcmp(token, "continue"))
        syntax_continue_stmt(stream);
    else if (!strcmp(token, "break"))
        syntax_break_stmt(stream);
    else if (!strcmp(token, "return"))
        syntax_return_stmt(stream);
    else if (is_id(token)) {
        token = scan(stream);
        char *token2 = scan(stream);
        unscan(token2, stream);
        unscan(token, stream);
        if (!strcmp(token2, ":"))
            syntax_id_labeled_stmt(stream);
        else
            syntax_expression_stmt(stream);
    }
    else
        syntax_expression_stmt(stream);
}

void syntax_declaration_or_statement(FILE *stream)
{
    char *token = scan(stream);
    unscan(token, stream);
    if (is_specifier(token) || is_storage(token) || is_qualifier(token))
        return syntax_declaration(stream, 0);
    return syntax_statement(stream);
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

void syntax_compound_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "{"
    jmp_buf env;
    goto t2; t1: end_stack_push("}", &env);
    while (1) {
        token = scan(stream);
        if (!strcmp(token, "}")) {
            end_stack_pop();
            return;
        }
        unscan(token, stream);
        syntax_declaration_or_statement(stream);
    }
    longjmp(env, 1);
    t2: if (!setjmp(env)) goto t1;
}

void syntax_if_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "if"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'if'");
    }
    syntax_expression(stream);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    syntax_statement(stream);
    goto t3;
    t2: if (!setjmp(env)) goto t1;
    t3: token = scan(stream);
    if (!strcmp(token, "else"))
        syntax_statement(stream);
    else
        unscan(token, stream);
}

void syntax_switch_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "switch"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'switch'");
    }
    syntax_expression(stream);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    syntax_statement(stream);
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_case_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "case"
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    syntax_conditional(stream);
    token = scan(stream);
    if (strcmp(token, ":")) {
        unscan(token, stream);
        error(stream, "expected ':' after 'case'");
    }
    end_stack_pop();
    syntax_statement(stream);
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_default_stmt(FILE *stream)
{
    char *token = scan(stream);  // it should be "default"
    token = scan(stream);
    if (strcmp(token, ":")) {
        unscan(token, stream);
        error_message("expected ':' after 'default'");
    }
    syntax_statement(stream);
}

void syntax_id_labeled_stmt(FILE *stream)
{
    char *token = scan(stream);
    token = scan(stream);
    syntax_statement(stream);
}

void syntax_while_stmt(FILE *stream)
{
    char *token = scan(stream);  // it should be "while"
    token = scan(stream);
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    if (strcmp(token, "(")) {
        unscan(token, stream);
        error(stream, "expected '(' after 'while'");
    }
    syntax_expression(stream);
    token = scan(stream);
    if (strcmp(token, ")")) {
        unscan(token, stream);
        error(stream, "expected ')'");
    }
    end_stack_pop();
    syntax_statement(stream);
    return;
    t2: if (!setjmp(env)) goto t1;
}

void syntax_do_while_stmt(FILE *stream)
{
    char *token = scan(stream);  // it should be "do"
    syntax_statement(stream);
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
    syntax_expression(stream);
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

void syntax_for_stmt(FILE *stream)
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
            syntax_expression(stream);
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
        syntax_expression(stream);
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
    t3: syntax_statement(stream);
}

void syntax_goto_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "goto"
    char *id = scan(stream);
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

void syntax_continue_stmt(FILE *stream)
{
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

void syntax_break_stmt(FILE *stream)
{
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

void syntax_return_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "return"
    token = scan(stream);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        jmp_buf env;
        goto t2; t1: end_stack_push(";", &env);
        syntax_expression(stream);
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

void syntax_external_declaration(FILE *stream)
{
    syntax_specifier(stream, 4);
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    int first_loop = 1;
    jmp_buf env;
    goto t2; t1: end_stack_push(";", &env);
    while (1) {
        syntax_declarator(stream, 0);
        token = scan(stream);
        if (!strcmp(token, "{")) {
            unscan(token, stream);
            if (!first_loop)
                error(stream, 0);  // already unscanned!
            end_stack_pop();
            syntax_compound_stmt(stream);
            return;
        }
        else if (!strcmp(token, ","))
            ;
        else if (!strcmp(token, "=")) {
            syntax_initializer(stream);
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
    char *token;
    end_stack = list_node();
    while (1) {
        token = scan(stream);
        if (!*token)
            return;
        unscan(token, stream);
        syntax_external_declaration(stream);
    }
}
