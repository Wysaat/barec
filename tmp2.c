#include "barec.h"
#include <stdlib.h>

void syntax_a_conditional(FILE *stream);

void syntax_aconstant(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        syntax_a_conditional(stream);
        token = scan(stream);
    }
    else if (!is_int(token) && !is_float(token)) {
        unscan(token, stream);
        error(stream, "expression is not an integer constant expression");
    }
}

void syntax_a_unary(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "+") || !strcmp(token, "-") || !strcmp(token, "!") || !strcmp(token, "~"))
        syntax_aconstant(stream);
    else {
        unscan(token, stream);
        syntax_aconstant(stream);
    }
}

void syntax_a_m_expr(FILE *stream)
{
    syntax_a_unary(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%"))
            syntax_a_unary(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_a_expr(FILE *stream)
{
    syntax_a_m_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-"))
            syntax_a_m_expr(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_shift(FILE *stream)
{
    syntax_a_a_expr(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, ">>") || !strcmp(token, "<<"))
            syntax_a_a_expr(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_relational(FILE *stream)
{
    syntax_a_shift(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") || !strcmp(token, "<=") || !strcmp(token, ">="))
            syntax_a_shift(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_equality(FILE *stream)
{
    syntax_a_relational(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!="))
            syntax_a_relational(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_and(FILE *stream)
{
    syntax_a_equality(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&"))
            syntax_a_equality(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_xor(FILE *stream)
{
    syntax_a_and(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^"))
            syntax_a_and(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_or(FILE *stream)
{
    syntax_a_xor(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|"))
            syntax_a_xor(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_logical_and(FILE *stream)
{
    syntax_a_or(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&"))
            syntax_a_or(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_logical_or(FILE *stream)
{
    syntax_a_logical_and(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||"))
            syntax_a_logical_and(stream);
        else {
            unscan(token, stream);
            return;
        }
    }
}

void syntax_a_conditional(FILE *stream)
{
    syntax_a_logical_or(stream);
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        syntax_a_conditional(stream);
        token = scan(stream);
        syntax_a_conditional(stream);
    }
    else {
        unscan(token, stream);
        return;
    }
}
