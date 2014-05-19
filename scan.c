#include <stdio.h>
#include <stdlib.h>
#include "scan.h"

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

void parse_declaration(FILE *stream, scope *local_scope) {
    char *token = scan(stream);
    if (!strcmp(token, "int")) {
        token = scan(stream);
        int addr = declare(token, int_t, local_scope);
        token = scan(stream);
        if (!strcmp(tokenn, ";"))
            ungetc(';', stream);
        else if (!strcmp(token, "=")) {
            parse_expression(stream, local_scope);
            emit("mov [ebp-");
            emit(itoa(addr));
            emit("], eax\n");
        }
    }
}

int declare(char *token, int type, scope *local_scope) {
    int addr = local_scope->next_addr;
    if (type == int_t) {
        local_scope->next_addr += 4;
    }
}

char *itoa(int value) {
    int n = 0;
    int num = value;
    if (num == 0) {
        return "0";
    }
    while (num > 0) {
        num = num / 10;
        n++;
    }

    char *retptr = (char *)malloc(n+1);
    retptr[n] = 0;
    int i;
    for (i = 1; i <= n; i++) {
        retptr[n-i] = value%10 + '0';
        value = value / 10;
    }
    return retptr;
}

int main()
{
    FILE *stream = fopen("test.c", "r");
    char *buffer;
    while (1) {
        buffer = scan(stream);
        if (!*buffer)
            break;
        puts(buffer);
    }
}