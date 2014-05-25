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

void parse_primary_expression(FILE *stream, scope *local_scope) {
    char *token = scan(stream);
    // convention: put the address on ebx, for assignments
    if (is_identifier(token)) {
        char *next_token = scan(stream);
        unscan(next_token, stream);
        if (local_scope->decl_type && !strcmp(next_token, "=") || !strcmp(next_token, ",") || !strcmp(next_token, ";"))
            declare(local_scope, token);
        emit("lea    ebx, ");
        emit("[ebp-");
        emit(itoa(find_addr(local_scope, token)));
        emit("]\n");
        emit("mov    eax, [ebx]\n");
    }
    else if (is_integer_const(token)) {
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
                emit("mul    ebx\n");
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
            emit("cmp    eax, ebx\n");
            if (!strcmp(token, ">"))
                emit("jng    tag");
            else if (!strcmp(token, "<"))
                emit("jnl    tag");
            else if (!strcmp(token, ">="))
                emit("jl     tag");
            else
                emit("jg     tag");
            int tag1 = tag++;
            emit(itoa(tag1));
            emit("\nmov    eax, 1\n");
            emit("jmp    tag");
            int tag2 = tag++;
            emit(itoa(tag2));
            emit("\ntag");
            emit(itoa(tag1));
            emit(":\nmov    eax, 0\ntag");
            emit(itoa(tag2));
            emit(":\n");
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

void parse_equality_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_relational_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            emit("cmp    eax, ebx\n");
            if (!strcmp(token, "=="))
                emit("jne    tag");
            else
                emit("je     tag");
            int tag1 = tag++;
            emit(itoa(tag1));
            emit("\nmov    eax, 1\n");
            emit("jmp    tag");
            int tag2 = tag++;
            emit(itoa(tag2));
            emit("\ntag");
            emit(itoa(tag1));
            emit(":\nmov    eax, 0\ntag");
            emit(itoa(tag2));
            emit(":\n");
        }
        token = scan(stream);
        if (!strcmp(token, "==") || !strcmp(token, "!="))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_and_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_equality_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            emit("and    eax, ebx\n");
        }
        token = scan(stream);
        if (!strcmp(token, "&"))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_exclusive_or_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_and_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            emit("xor    eax, ebx\n");
        }
        token = scan(stream);
        if (!strcmp(token, "^"))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_inclusive_or_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_exclusive_or_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            emit("or    eax, ebx\n");
        }
        token = scan(stream);
        if (!strcmp(token, "|"))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_logical_and_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_inclusive_or_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            emit("cmp    eax, 0\n");
            emit("je     tag");
            int tag1 = tag++;
            emit(itoa(tag1));
            emit("\ncmp    ebx, 0\n");
            emit("jne    tag");
            int tag2 = tag++;
            emit(itoa(tag2));
            emit("\ntag");
            emit(itoa(tag1));
            emit(":\nmov    eax, 0\n");
            emit("jmp    tag");
            int tag3 = tag++;
            emit(itoa(tag3));
            emit("\ntag");
            emit(itoa(tag2));
            emit(":\nmov    eax, 1\n");
            emit("tag");
            emit(itoa(tag3));
            emit(":\n");
        }
        token = scan(stream);
        if (!strcmp(token, "&&"))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}

void parse_logical_or_expression(FILE *stream, scope *local_scope) {
    char *token = 0;
    while (1) {
        parse_logical_and_expression(stream, local_scope);
        if (token) {
            emit("pop    ebx\n");
            emit("xchg   eax, ebx\n");
            emit("cmp    eax, 0\n");
            emit("jne    tag");
            int tag1 = tag++;
            emit(itoa(tag1));
            emit("\ncmp    ebx, 0\n");
            emit("je     tag");
            int tag2 = tag++;
            emit(itoa(tag2));
            emit("\ntag");
            emit(itoa(tag1));
            emit(":\nmov    eax, 1\n");
            emit("jmp    tag");
            int tag3 = tag++;
            emit(itoa(tag3));
            emit("\ntag");
            emit(itoa(tag2));
            emit(":\nmov    eax, 0\n");
            emit("tag");
            emit(itoa(tag3));
            emit(":\n");
        }
        token = scan(stream);
        if (!strcmp(token, "||"))
            emit("push   eax\n");
        else {
            unscan(token, stream);
            break;
        }
    }
}


void parse_conditional_expression(FILE *stream, scope *local_scope) {
    parse_logical_or_expression(stream, local_scope);
}

void parse_assignment_expression(FILE *stream, scope *local_scope) {
    parse_conditional_expression(stream, local_scope);
    char *token = scan(stream);
    if (!strcmp(token, "=")) {
        emit("push   ebx\n");
        parse_assignment_expression(stream, local_scope);
        emit("pop    ebx\n");
        emit("mov    [ebx], eax\n");
    }
    else
        unscan(token, stream);
}

void parse_expression(FILE *stream, scope *local_scope) {
    char *token;
    while (1) {
        parse_assignment_expression(stream, local_scope);
        token = scan(stream);
        if (!strcmp(token, ";")) {
            unscan(token, stream);
            break;
        }
    }
}

void parse_expression_statement(FILE *stream, scope *local_scope) {
    char *token = scan(stream);
    if (!strcmp(token, ";")) {
        emit("");
        return;
    }
    unscan(token, stream);
    parse_expression(stream, local_scope);
    token = scan(stream);
    if (!strcmp(token, ";"))
        return;
}

void parse_statement(FILE *stream, scope *local_scope) {
    parse_expression_statement(stream, local_scope);
}

void parse_declaration(FILE *stream, scope *local_scope, int decl) {
    char *token = scan(stream);
    if (decl) {
        if (!strcmp(token, "int"))
            local_scope->decl_type = int_t;
    }
    parse_expression(stream, local_scope);
    token = scan(stream);
    if (!strcmp(token, ";"))
        local_scope->decl_type = 0;
}

int main()
{
    system("touch out && rm out");
    FILE *stream = fopen("test.c", "r");
    scope *local_scope = scope_init();
    local_scope->next_addr += 4;  /* push   ebp */
    char *token;
    while (1) {
        token = scan(stream);
        if (!*token)
            break;
        unscan(token, stream);
        if (!strcmp(token, "int"))
            parse_declaration(stream, local_scope, 1);
        else
            parse_statement(stream, local_scope);
    }
    fclose(stream);

    printf("local_scope->next_addr is %d\n", local_scope->next_addr);

    system("touch out && rm out");

    emit("push   ebp\n");
    emit("mov    ebp, esp\n");
    emit("sub    esp, ");
    emit(itoa(local_scope->next_addr));
    emit("\n");

    stream = fopen("test.c", "r");
    while (1) {
        token = scan(stream);
        if (!*token)
            break;
        unscan(token, stream);
        if (!strcmp(token, "int"))
            parse_declaration(stream, local_scope, 0);
        else
            parse_statement(stream, local_scope);
    }

    return 0;
}