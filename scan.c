#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "scope.h"
#include "types.h"
#include "utils.h"

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

void parse_constant(FILE *stream, scope *local_scope) {
    char *token = scan(stream);
    if (is_integer_const(token)) {
        emit("mov    eax, ");
        emit(token);
        emit("\n");
    }
    else if (is_character_const(token)) {
        emit("mov    eax, ");
        emit(token);
        emit("\n");
    }
}

void parse_primary_expression(FILE *stream, scope *local_scope) {
    parse_constant(stream, local_scope);
}

void parse_postfix_expression(FILE *stream, scope *local_scope) {
    parse_primary_expression(stream, local_scope);
}

void parse_unary_expression(FILE *stream, scope *local_scope) {
    parse_postfix_expression(stream, local_scope);
}

void parse_cast_expression(FILE *stream, scope *local_scope) {
    parse_unary_expression(stream, local_scope);
}

void parse_multiplicative_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_cast_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            if (!strcmp(token, "*"))
                emit("mul    eax\n");
            else if (!strcmp(token, "/"))
                emit("div    ebx\n");
            else if (!strcmp(token, "%")) {
                emit("div    ebx\n");
                emit("mov    eax, edx\n");
            }
        }
        token = scan(stream);
        if (!strcmp(token, "*") || !strcmp(token, "/") || !strcmp(token, "%")) {
            emit("push   eax\n");
        }
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_additive_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_multiplicative_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            if (!strcmp(token, "+"))
                emit("add    eax, ebx\n");
            else
                emit("sub    eax, ebx\n");
        }
        token = scan(stream);
        if (!strcmp(token, "+") || !strcmp(token, "-")) {
            emit("push   eax\n");
        }
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_shift_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_additive_expression(stream, local_scope);
        if (token) {
            emit("pop    ecx\n");
            emit("xchg   eax, ecx\n");
            if (!strcmp(token, "<<"))
                emit("shl    eax, cl\n");
            else
                emit("shr    eax, cl\n");
        }
        token = scan(stream);
        if (!strcmp(token, "<<") || !strcmp(token, ">>"))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_relational_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_shift_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            if (!strcmp(token, "<")) {
                emit("cmp    eax, ebx\n");
                emit("jg     ");
            }
        }
        token = scan(stream);
        if (!strcmp(token, "<") || !strcmp(token, ">") || !strcmp(token, "<=") || !strcmp(token, ">="))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_conditional_expression(FILE *stream, scope *local_scope) {
    parse_relational_expression(stream, local_scope);
}

void parse_assignment_expression(FILE *stream, scope *local_scope) {
    parse_conditional_expression(stream, local_scope);
}

void parse_expression(FILE *stream, scope *local_scope) {
    char *token;
    while (1) {
        parse_assignment_expression(stream, local_scope);
        token = scan(stream);
        if (!strcmp(token, ";")) {
            break;
        }
    }
}

void parse_expression_statement(FILE *stream, scope *local_scope) {
    char *token = scan(stream);
    if (!strcmp(token, ";"))
        return;
    unscan(token, stream);
    parse_expression(stream, local_scope);
    token = scan(stream);
    if (!strcmp(token, ";"))
        return;
}

void parse_statement(FILE *stream, scope *local_scope) {
    parse_expression_statement(stream, local_scope);
}

int parse_declaration(FILE *stream, scope *local_scope) {
    char *token = scan(stream);
    if (!*token)
        return -1;
    if (!strcmp(token, "int")) {
        token = scan(stream);
        int addr = declare(token, int_t, local_scope);
        token = scan(stream);
        if (!strcmp(token, ";"))
            return 0;
        else if (!strcmp(token, "=")) {
            parse_expression(stream, local_scope);
            emit("mov dword [ebp-");
            emit(itoa(addr));
            emit("], eax\n");
            token = scan(stream);
            if (!strcmp(token, ";"))
                return 0;
        }
    }
}

int declare(char *token, int type, scope *local_scope) {
    int addr = local_scope->next_addr;
    if (type == int_t) {
        local_scope->next_addr += 4;
        if (local_scope->vals)
            val_list_append(local_scope->vals, token, addr);
        else
            local_scope->vals = val_list_node(token, addr);
    }
    return addr;
}

int main()
{
    system("rm out");
    FILE *stream = fopen("test.c", "r");
    scope *local_scope = scope_init();
    parse_statement(stream, local_scope);

    return 0;
}