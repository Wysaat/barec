#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "barec.h"

list *end_stack;


void end_stack_push(char *token, jmp_buf *env);
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
        if (!strcmp(token, "auto")) {
            storage = 1;
        }
        else if (!strcmp(token, "static")) {
            storage = 2;
        }
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
        else if (!strcmp(token, "struct")) {
            if (atype >= 0) {
                unscan(token, stream);
                error_message("two or more data types in declaration specifiers");
                scan(stream);
            }
            if (sign >= 0) {
                unscan(token, stream);
                error_message("'struct' cannot be signed or unsigned");
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

// void *parse_unary(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);
//     if (!strcmp(token, "++"))
//         return PREINC(parse_unary(stream, namespace), 1);
//     else if (!strcmp(token, "--"))
//         return PREINC(parse_unary(stream, namespace), 0);
//     else if (!strcmp(token, "&"))
//         return ADDR(parse_cast(stream, namespace));
//     else if (!strcmp(token, "*"))
//         return INDIRECTION(parse_cast(stream, namespace));
//     else if (!strcmp(token, "+") || !strcmp(token, "-")
//                    || !strcmp(token, "~") || !strcmp(token, "!"))
//         return UNARY(parse_cast(stream, namespace), token);
//     else if (!strcmp(token, "sizeof")) {
//         token = scan(stream);
//         if (!strcmp(token, "(")) {
//             // type_name or expression;
//         }
//         else {
//             unscan(token, stream);
//             list *type_list = get_type_list(parse_unary(stream, namespace));
//             return size_expr(type_list);
//         }
//     }
//     else {
//         unscan(token, stream);
//         return parse_postfix(stream, namespace);
//     }
// }

// void *parse_cast(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);
//     if (!strcmp(token, "(")) {
//         list *type_list = parse_type_name(stream, namespace);
//         token = scan(stream);
//         if (!strcmp(token, ")"))
//             return CAST(type_list, parse_cast(stream, namespace));
//     }
//     else {
//         unscan(token, stream);
//         return parse_unary(stream, namespace);
//     }
// }

// void *parse_m_expr(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_cast(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "*") || !strcmp(token, "/")) {
//             btype_t btype = !strcmp(token, "*") ? mul : divi;
//             void *expr2 = parse_cast(stream, namespace);
//             if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(btype, *ptr, *(ptr+1));
//             }
//         }
//         else if (!strcmp(token, "%")) {
//             void *expr2 = parse_cast(stream, namespace);
//             if (type_is_int(expr) && type_is_int(expr2)) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(mod, *ptr, *(ptr+1));
//             }
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_a_expr(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_m_expr(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "+") || !strcmp(token, "-")) {
//             btype_t btype = !strcmp(token, "+") ? add : sub;
//             void *expr2 = parse_m_expr(stream, namespace);
//             if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(btype, *ptr, *(ptr+1));
//             }
//             else if ((get_type(expr) == pointer_t || get_type(expr) == array_t) && type_is_int(expr2))
//                 expr = BINARY(btype, expr, BINARY(mul, expr2, size_expr(get_type_list(INDIRECTION(expr)))));
//             else if ((get_type(expr2) == pointer_t || get_type(expr2) == array_t) && type_is_int(expr))
//                 expr = BINARY(btype, expr2, BINARY(mul, expr, size_expr(get_type_list(INDIRECTION(expr2)))));
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_shift(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_a_expr(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "<<") || !strcmp(token, ">>")) {
//             btype_t btype = !strcmp(token, "<<") ? lshift : rshift;
//             void *expr2 = parse_a_expr(stream, namespace);
//             if (type_is_int(expr) && type_is_int(expr2)) {
//                 set_type_list(expr, integral_promotion2(get_type_list(expr)));
//                 set_type_list(expr2, integral_promotion2(get_type_list(expr2)));
//                 expr = BINARY(btype, expr, expr2);
//             }
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_relational(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_shift(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "<") || !strcmp(token, ">") ||
//                   !strcmp(token, "<=") || !strcmp(token, ">=")) {
//             btype_t btype = !strcmp(token, "<") ? lt :
//                             !strcmp(token, ">") ? gt :
//                             !strcmp(token, "<=") ? le : ge;
//             void *expr2 = parse_shift(stream, namespace);
//             if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(btype, *ptr, *(ptr+1));
//             }
//             else if ((get_type(expr) == pointer_t ||get_type(expr) == array_t) &&
//                            (get_type(expr2) == pointer_t || get_type(expr2) == array_t))
//                 expr = BINARY(btype, expr, expr2);
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_equality(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_relational(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "==") || !strcmp(token, "!=")) {
//             btype_t btype = !strcmp(token, "==") ? eq : neq;
//             void *expr2 = parse_relational(stream, namespace);
//             if (get_type(expr) == arithmetic_specifier_t && get_type(expr2) == arithmetic_specifier_t) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(btype, *ptr, *(ptr+1));
//             }
//             else if ((get_type(expr) == pointer_t ||get_type(expr) == array_t) &&
//                            (get_type(expr2) == pointer_t || get_type(expr2) == array_t))
//                 expr = BINARY(btype, expr, expr2);
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_and(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_equality(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "&")) {
//             void *expr2 = parse_equality(stream, namespace);
//             if (type_is_int(expr) && type_is_int(expr2)) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(band, *ptr, *(ptr+1));
//             }
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_xor(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_and(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "^")) {
//             void *expr2 = parse_and(stream, namespace);
//             if (type_is_int(expr) && type_is_int(expr2)) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(bxor, *ptr, *(ptr+1));
//             }
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

// void *parse_or(FILE *stream, namespace_t *namespace) {
//     void *expr = parse_xor(stream, namespace);
//     while (1) {
//         char *token = scan(stream);
//         if (!strcmp(token, "|")) {
//             void *expr2 = parse_xor(stream, namespace);
//             if (type_is_int(expr) && type_is_int(expr2)) {
//                 void **ptr = arithmetic_convertion(expr, expr2);
//                 expr = BINARY(bor, *ptr, *(ptr+1));
//             }
//         }
//         else {
//             unscan(token, stream);
//             return expr;
//         }
//     }
// }

void syntax_or(FILE *stream) {
    syntax_postfix(stream);
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
    if (is_assignment_operator(token)) {
        syntax_assignment(stream);
    }
    else {
        unscan(token, stream);
    }
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
    // else if (!strcmp(token, "if"))
    //     parse_if_stmt(stream, namespace);
    // else if (!strcmp(token, "switch"))
    //     parse_switch_stmt(stream, namespace);
    // else if (!strcmp(token, "case"))
    //     parse_case_stmt(stream, namespace);
    // else if (!strcmp(token, "default"))
    //     parse_default_stmt(stream, namespace);
    // else if (!strcmp(token, "while"))
    //     parse_while_stmt(stream, namespace);
    // else if (!strcmp(token, "for"))
    //     parse_for_stmt(stream, namespace);
    // else if (!strcmp(token, "goto"))
    //     parse_goto_stmt(stream, namespace);
    // else if (!strcmp(token, "continue"))
    //     parse_continue_stmt(stream, namespace);
    // else if (!strcmp(token, "break"))
    //     parse_break_stmt(stream, namespace);
    // else if (!strcmp(token, "return"))
    //     parse_return_stmt(stream, namespace);
    // else if (is_id(token)) {
    //     token = scan(stream);
    //     char *token2 = scan(stream);
    //     unscan(token2, stream);
    //     unscan(token, stream);
    //     if (!strcmp(token2, ":"))
    //         parse_id_labeled_stmt(stream, namespace);
    //     else
    //         parse_expression_stmt(stream, namespace);
    // }
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

// void *parse_if_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // should be "if"
//     token = scan(stream);
//     if (strcmp(token, "(")) {
//         error("expected '(' after if");
//         statement_error(token, stream);
//         return 0;
//     }
//     void *expr = parse_expression(stream, namespace);
//     token = scan(stream);
//     if (strcmp(token, ")")) {
//         error("expected ')'");
//         statement_error(token, stream);
//         return 0;
//     }
//     void *statement = parse_statement(stream, namespace), *statement2 = 0;
//     token = scan(stream);
//     if (!strcmp(token, "else"))
//         statement2 = parse_statement(stream, namespace);
//     else
//         unscan(token, stream);
//     return IF_STMT(expr, statement, statement2);
// }

// void *parse_switch_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // should be "switch"
//     token = scan(stream);
//     if (strcmp(token, "(")) {
//         error("expected '(' after switch");
//         statement_error(token, stream);
//         return 0;
//     }
//     void *expr = parse_expression(stream, namespace);
//     token = scan(stream);
//     if (strcmp(token, ")")) {
//         error("expected ')'");
//         statement_error(token, stream);
//         return 0;
//     }
//     char *old_break_tag = break_tag;
//     break_tag = get_tag();
//     void *statement = parse_statement(stream, namespace);
//     switch_stmt *retptr = SWITCH_STMT(expr, statement, break_tag);
//     break_tag = old_break_tag;
//     return retptr;
// }

// void *parse_case_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // should be "case"
//     void *expression = parse_conditional(stream, namespace);
//     char *value, *tag;
//     int err = 0;
//     if (!is_integral_constant(expression)) {
//         error("case label does not reduce to an integer constant");
//         err = 1;
//     }
//     else {
//         value = ((arithmetic *)expression)->value;
//         tag = get_tag();
//     }
//     token = scan(stream);
//     if (strcmp(token, ":")) {
//         error("expected ':' after case");
//         unscan(token, stream);
//         err = 1;
//     }
//     void *statement = parse_statement(stream, namespace);
//     return err? 0 : CASE_STMT(tag, value, statement);
// }

// void *parse_default_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // it should be "default"
//     token = scan(stream);  // it should be ":"
//     void *statement = parse_statement(stream, namespace);
//     return DEFAULT_STMT(get_tag(), statement);
// }

// void *parse_id_labeled_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);
//     char *id = token;
//     token = scan(stream);  // it should be ":"
//     void *statement = parse_statement(stream, namespace);
//     id_labeled_stmt *retptr = ID_LABELED_STMT(get_tag(), id, statement);
//     namespace_t *nptr = namespace;
//     do {
//         if (nptr->labels)
//             list_append(nptr->labels, retptr);
//         nptr = nptr->outer;
//     } while (nptr);
//     return retptr;
// }

// void *parse_while_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // it should be "while"
//     token = scan(stream);
//     if (strcmp(token, "(")) {
//         error("expected '(' after while");
//         statement_error(token, stream);
//         return 0;
//     }
//     void *expr = parse_expression(stream, namespace);
//     token = scan(stream);
//     if (strcmp(token, ")")) {
//         error("expected ')'");
//         statement_error(token, stream);
//         return 0;
//     }
//     char *old_continue_tag = continue_tag;
//     char *old_break_tag = break_tag;
//     continue_tag = get_tag();
//     break_tag = get_tag();
//     void *statement = parse_statement(stream, namespace);
//     while_stmt *retptr = WHILE_STMT(expr, statement, 0, continue_tag, break_tag);
//     continue_tag = old_continue_tag;
//     break_tag = old_break_tag;
//     return retptr;
// }

// void *parse_do_while_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // it should be "do"
//     char *old_continue_tag = continue_tag;
//     char *old_break_tag = break_tag;
//     continue_tag = get_tag();
//     break_tag = get_tag();
//     void *statement = parse_statement(stream, namespace);
//     token = scan(stream);
//     if (strcmp(token, "while")) {
//         error("expected 'while'");
//         statement_error(token, stream);
//         return 0;
//     }
//     token = scan(stream);
//     if (strcmp(token, "(")) {
//         error("expected '(' after while");
//         statement_error(token, stream);
//         return 0;
//     }
//     void *expr = parse_expression(stream, namespace);
//     token = scan(stream);
//     if (strcmp(token, ")")) {
//         error("expected ')'");
//         statement_error(token, stream);
//         return 0;
//     }
//     token = scan(stream);
//     if (strcmp(token, ";")) {
//         error("expected ';'");
//         statement_error(token, stream);
//         return 0;
//     }
//     while_stmt *retptr = WHILE_STMT(expr, statement, 1, continue_tag, break_tag);
//     continue_tag = old_continue_tag;
//     break_tag = old_break_tag;
//     return retptr;
// }

// void *parse_for_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // it should be "for"
//     token = scan(stream);
//     if (strcmp(token, "(")) {
//         error("expected '(' after for");
//         statement_error(token, stream);
//         return 0;
//     }
//     void *exprs[3];
//     char *etks[] = { ";", ";", ")" };
//     int i;
//     for (i = 0; i < 3; i++) {
//         token = scan(stream);
//         if (!strcmp(token, etks[i]))
//             exprs[i] = 0;
//         else {
//             unscan(token, stream);
//             exprs[i] = parse_expression(stream, namespace);
//             token = scan(stream);
//             if (strcmp(token, exprs[i]))
//                 /* error */;
//         }
//     }
//     char *old_continue_tag = continue_tag;
//     char *old_break_tag = break_tag;
//     continue_tag = get_tag();
//     break_tag = get_tag();
//     void *statement = parse_statement(stream, namespace);
//     for_stmt *retptr = FOR_STMT(exprs[0], exprs[1], exprs[2], statement, continue_tag, break_tag);
//     continue_tag = old_continue_tag;
//     break_tag = old_break_tag;
//     return retptr;
// }

// void *parse_goto_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // should be "goto"
//     char *id = scan(stream);
//     token = scan(stream);
//     if (strcmp(token, ";")) {
//         error("expected ';'");
//         statement_error(token, stream);
//         return 0;
//     }
//     namespace_t *nptr = namespace;
//     do {
//         if (nptr->labels)
//             return GOTO_STMT(id, nptr);
//         nptr = nptr->outer;
//     } while (nptr);
// }

// void *parse_continue_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // should be "continue"
//     token = scan(stream);
//     if (strcmp(token, ";")) {
//         error("expected ';'");
//         statement_error(token, stream);
//         return 0;
//     }
//     return CONTINUE_STMT(continue_tag);
// }

// void *parse_break_stmt(FILE *stream, namespace_t *namespace)
// {
//     char *token = scan(stream);  // should be "break"
//     token = scan(stream);
//     if (strcmp(token, ";")) {
//         error("expected ';'");
//         statement_error(token, stream);
//         return 0;
//     }
//     return BREAK_STMT(break_tag);
// }

void syntax_return_stmt(FILE *stream)
{
    char *token = scan(stream);  // should be "return"
    token = scan(stream);
    if (strcmp(token, ";")) {
        unscan(token, stream);
        syntax_expression(stream);
        token = scan(stream);
        if (strcmp(token, ";")) {
            unscan(token, stream);
            error_message("expected ';'");
        }
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
