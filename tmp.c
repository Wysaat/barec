#include "barec.h"
#include <stdlib.h>

aconstant_t *aconstant_init(int is_int, int ival, double fval)
{
    aconstant_t *retptr = malloc(sizeof(aconstant_t));
    retptr->is_int = is_int;
    retptr->ival = ival;
    retptr->fval = fval;
    return retptr;
}

aconstant_t *parse_aconstant(FILE *stream)
{
    long offset = ftell(stream);
    int line = file_info.line;
    int column = file_info.column;

    char *token = scan(stream);
    int is_int_flag;
    int ival;
    double fval;
    if (is_int(token)) {
        ival = atoi(token);
        is_int_flag = 1;
    }
    else if (is_float(token)) {
        fval = atof(token);
        is_int_flag = 0;
    }
    else if (!strcmp(token, "(")) {
        aconstant_t *retptr = parse_a_conditional(stream);
        if (!retptr) {
            unscan(token, stream);
            return 0;
        }
        else {
            token = scan(stream);
            if (!strcmp(token, ")"))
                return retptr;
            else {
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                return 0;
            }
        }
    }
    else {
        unscan(token, stream);
        return 0;
    }
    return aconstant_init(is_int_flag, ival, fval);
}

aconstant_t *parse_a_unary(FILE *stream)
{
    char *token = scan(stream);
    if (!strcmp(token, "+")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else
            return primary;
    }
    else if (!strcmp(token, "-")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else {
            if (primary->is_int)
                primary->ival = -primary->ival;
            else
                primary->fval = -primary->fval;
            return primary;
        }
    }
    else if (!strcmp(token, "!")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else {
            if (primary->is_int)
                return aconstant_init(1, !primary->ival, 0);
            else
                return aconstant_init(1, !primary->fval, 0);
        }
    }
    else if (!strcmp(token, "~")) {
        aconstant_t *primary = parse_aconstant(stream);
        if (!primary) {
            unscan(token, stream);
            return 0;
        }
        else {
            if (primary->is_int)
                return aconstant_init(1, ~primary->ival, 0);
        }
    }
    else {
        unscan(token, stream);
        return parse_aconstant(stream);
    }
}

aconstant_t *parse_a_m_expr(FILE *stream)
{
    aconstant_t *left = parse_a_unary(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "*")) {
            aconstant_t *right = parse_a_unary(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival * right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival * right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval * right->ival);
                    else
                        return aconstant_init(0, 0, left->fval * right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "/")) {
            aconstant_t *right = parse_a_unary(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival / right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival / right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval / right->ival);
                    else
                        return aconstant_init(0, 0, left->fval / right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "%")) {
            aconstant_t *right = parse_a_unary(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival * right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}


aconstant_t *parse_a_a_expr(FILE *stream)
{
    aconstant_t *left = parse_a_m_expr(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "+")) {
            aconstant_t *right = parse_a_m_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival + right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival + right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval + right->ival);
                    else
                        return aconstant_init(0, 0, left->fval + right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "-")) {
            aconstant_t *right = parse_a_m_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        return aconstant_init(1, left->ival - right->ival, 0);
                    else
                        return aconstant_init(0, 0, left->ival - right->fval);
                }
                else {
                    if (right->is_int)
                        return aconstant_init(0, 0, left->fval - right->ival);
                    else
                        return aconstant_init(0, 0, left->fval - right->fval);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_shift(FILE *stream)
{
    aconstant_t *left = parse_a_a_expr(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, ">>")) {
            aconstant_t *right = parse_a_a_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival >> right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "<<")) {
            aconstant_t *right = parse_a_a_expr(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival << right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_relational(FILE *stream)
{
    aconstant_t *left = parse_a_shift(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "<")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival < right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival < right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval < right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval < right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, ">")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival > right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival > right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval > right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval > right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, "<=")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival <= right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival <= right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval <= right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval <= right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else if (!strcmp(token, ">=")) {
            aconstant_t *right = parse_a_shift(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival >= right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival >= right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval >= right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval >= right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_equality(FILE *stream)
{
    aconstant_t *left = parse_a_relational(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!=")) {
            aconstant_t *right = parse_a_relational(stream);
            if (right) {
                if (!strcmp(token, "==")) {
                    if (left->is_int) {
                        if (right->is_int)
                            left = aconstant_init(1, left->ival == right->ival, 0);
                        else
                            left = aconstant_init(1, left->ival == right->fval, 0);
                    }
                    else {
                        if (right->is_int)
                            left = aconstant_init(1, left->fval == right->ival, 0);
                        else
                            left = aconstant_init(1, left->fval == right->fval, 0);
                    }
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_and(FILE *stream)
{
    aconstant_t *left = parse_a_equality(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&")) {
            aconstant_t *right = parse_a_equality(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival & right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_xor(FILE *stream)
{
    aconstant_t *left = parse_a_and(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "^")) {
            aconstant_t *right = parse_a_and(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival ^ right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_or(FILE *stream)
{
    aconstant_t *left = parse_a_xor(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "|")) {
            aconstant_t *right = parse_a_xor(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival | right->ival, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_logical_and(FILE *stream)
{
    aconstant_t *left = parse_a_or(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "&&")) {
            aconstant_t *right = parse_a_or(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival && right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival && right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval && right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval && right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_logical_or(FILE *stream)
{
    aconstant_t *left = parse_a_logical_and(stream);
    if (!left)
        return 0;
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "||")) {
            aconstant_t *right = parse_a_logical_and(stream);
            if (right) {
                if (left->is_int) {
                    if (right->is_int)
                        left = aconstant_init(1, left->ival || right->ival, 0);
                    else
                        left = aconstant_init(1, left->ival || right->fval, 0);
                }
                else {
                    if (right->is_int)
                        left = aconstant_init(1, left->fval || right->ival, 0);
                    else
                        left = aconstant_init(1, left->fval || right->fval, 0);
                }
            }
            else {
                unscan(token, stream);
                return left;
            }
        }
        else {
            unscan(token, stream);
            return left;
        }
    }
}

aconstant_t *parse_a_conditional(FILE *stream)
{
    aconstant_t *cond = parse_a_logical_or(stream);
    if (!cond)
        return 0;
    char *token = scan(stream);
    if (!strcmp(token, "?")) {
        long offset = ftell(stream);
        int line = file_info.line;
        int column = file_info.column;
        aconstant_t *first = parse_a_conditional(stream);
        if (first) {
            token = scan(stream);
            if (!strcmp(token, ":")) {
                aconstant_t *second = parse_a_conditional(stream);
                if (second) {
                    token = scan(stream);
                    char *atks[] = {
                        "?", "||", "&&", "|", "^", "&", "==", "!=", "<", ">",
                        "<=", ">=", "<<", ">>", "+", "-", "*", "/", "%", 0,
                    };
                    char **ptr;
                    for (ptr = atks; *ptr; ptr++) {
                        if (!strcmp(token, *ptr)) {
                            fseek(stream, offset, SEEK_SET);
                            file_info.line = line;
                            file_info.column = column;
                            return cond;
                        }
                    }
                    unscan(token, stream);
                    if (cond->is_int) {
                        if (cond->ival)
                            return first;
                        else
                            return second;
                    }
                    else {
                        if (cond->fval)
                            return first;
                        else
                            return second;
                    }
                }
                else {
                    fseek(stream, offset, SEEK_SET);
                    file_info.line = line;
                    file_info.column = column;
                    return cond;
                }
            }
            else {
                fseek(stream, offset, SEEK_SET);
                file_info.line = line;
                file_info.column = column;
                return cond;
            }
        }
        else {
            unscan(token, stream);
            return cond;
        }
    }
    else {
        unscan(token, stream);
        return 0;
    }
}

arithmetic *parse_arithmetic_constant(FILE *stream)
{
    aconstant_t *aconstant = parse_a_conditional(stream);
    char *value = malloc(1024);
    int atype;
    if (aconstant->is_int) {
        atype = int_t;
        sprintf(value, "%d", aconstant->ival);
    }
    else {
        atype = double_t;
        sprintf(value, "%f", aconstant->fval);
    }
    return ARITHMETIC(value, atype);
}
