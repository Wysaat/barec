void *parse_constant(FILE *stream) {
    char *token = scan(stream);
    if (is_integer_const(token))
        return INTEGER_CONSTANT(token);
    else if (is_character_const(token))
        return CHARACTER_CONSTANT(token);
    else if (is_floating_const(token))
        return FLOATING_CONSTANT(token);
}

void *parse_primary_expression(FILE *stream) {
    char *token = scan(stream);
    if (is_identifier(token))  // it may also be an enumeration_constant
        return IDENTIFIER(token);
    else if (is_string(token))
        return STRING(token);
    else if (!strcmp(token, "(")) {
        void *retptr = parse_expression(stream);
        token = scan(stream);  // it should be ")"
        return retptr;
    }
    else
        return parse_constant(stream);
}

void parse_argument_expression_list(FILE *stream) {
    list *expression_list = list_node();
    while (1) {
        list_append(expression_list, parse_assignment_expression(stream));
        char *token = scan(stream);
        if (strcmp(token, ",")) {
            unscan(token);
            return (ARGUMENT_EXPRESSION_LIST(expression_list));
        }
    }
}

void *parse_postfix_expression(FILE *stream) {
    void *expression = parse_primary_expression(stream);
    while (1) {
        char *token = scan(stream);
        if (!strcmp(token, "[")) {
            expression = SUBSCRIPTION_EXPRESSION(expression, parse_expression(stream));
            token = scan(stream);  // it should be "]"
        }
        else if (!strcmp(token, "(")) {
            token = scan(stream);
            if (!strcmp(token, ")")) {
                return FUNCCALL_EXPRESSION(expression, 0);
            }
            unscan(token, stream);
            expression = FUNCCALL_EXPRESSION(expression, parse_argument_expression_list(stream));
            token = scan(stream);  // it should be ")"
        }
        else if (!strcmp(token, ".")) {
            token = scan(stream);
            expression = POINTREF_EXPRESSION(expression, token);
        }
        else if (!strcmp(token, "->")) {
            token = scan(stream);
            expression = ARROWREF_EXPRESSION(expression, token);
        }
        else if (!strcmp(token, "++")) {
            expression = POSTFIXINC_EXPRESSION(expression);
        }
        else if (!strcmp(token, "--")) {
            expression = POSTFIXDEC_EXPRESSION(expression);
        }
        else {
            unscan(token, stream);
            return expression;
        }
    }
}

void *parse_unary_expression(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "++")) {
        return PREFIXINC_EXPRESSION(parse_unary_expression(stream));
    }
    else if (!strcmp(token, "--")) {
        return PREFIXDEC_EXPRESSION(parse_unary_expression(stream));
    }
    if (!strcmp(token, "&")) {
        return ADDRESS_EPXRESION(parse_cast_expression(stream));
    }
    else if (!strcmp(token, "*")) {
        return INDIRECTION_EXPRESSION(parse_cast_expresion(stream));
    }
    else if (!strcmp(token, "+")) {
        return UNARYPLUS_EXPRESSION(parse_cast_expression(stream));
    }
    else if (!strcmp(token, "-")) {
        return UNARYMINUS_EXPRESSION(parse_cast_expression(stream));
    }
    else if (!strcmp(token, "~")) {
        return ONESCOMP_EXPRESSION(parse_cast_expression(stream));
    }
    else if (!strcmp(token, "!")) {
        return LOGICALNEG_EXPRESSION(parse_cast_expression(stream));
    }
    else if (!strcmp(token, "sizeof")) {
        token = scan(stream);
        if (!strcmp(token, "(")) {
            token = scan(stream);
            if (is_typename(token) || !strcmp(token, "struct") || !strcmp(token, "enum")) {
                return SIZEOF_EXPRESSION(parse_typename(stream));
            }
            else {
                unscan(token, stream);
                return SIZEOF_EXPRESSION(parse_unary_expression(stream));
            }
        }
    }
}

void *parse_cast_expression(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "(")) {
        token = scan(stream);
        if (is_basic_typename(token)) {
            char *typename = token;
            token = scan(stream);
            if (!strcmp(token, ")")) {
                return CAST_EXPRESSION(typename, parse_cast_expression(stream));
            }
        }
    }
    else {
        unscan(token, stream);
        return parse_unary_expression(stream);
    }
}

void *parse_multiplicative_expression(FILE *stream) {
    void *expression = parse_cast_expression(stream);
    while (1) {
        char *token = scan(stream);
        if (strcmp(token, "*") && strcmp(token, "/") && strcmp(token, "%")) {
            unscan(token, stream);
            return expression;
        }
        expression = MULTIPLICATIVE_EXPRESSION(expression, parse_cast_expression, token);
    }
}

void *parse_conditional_expression(FILE *stream) {
    return parse_primary_expression(stream);
}

void *parse_assignment_expression(FILE *stream) {
    void *expression1 = parse_conditional_expression(stream);
    while (1) {
        char *token = scan(stream);
        if (strcmp(token, "=")) {
            unscan(token, stream);
            return expression1;
        }
        expression1 = ASSIGNMENT_EXPRESSION(expression1, parse_conditional_expression(stream));
    }
}

void *parse_expression(FILE *stream) {
}

void *parse_compound_statement(FILE *stream) {
    char *token = scan(stream);
    if (!strcmp(token, "{")) {
        while (1) {
            parse_statement(stream);
        }
    }
}
