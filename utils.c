#include "barec.h"

void unscan(char *token, FILE *stream) {
    char *ptr = token;
    while (*ptr)
        ptr++;

    while (--ptr != token)
        ungetc(*ptr, stream);
    ungetc(*ptr, stream);
}

void emit(char *words) {
    FILE *stream = fopen("out", "a");
    fwrite(words, sizeof(char), strlen(words), stream);
    fclose(stream);
}

void emitln(char *words) {
    FILE *stream = fopen("out", "a");
    fwrite(words, sizeof(char), strlen(words), stream);
    fwrite("\n", sizeof(char), 1, stream);
    fclose(stream);
}

int is_id(char *token) {
    return (*token == '_' || (*token >= 'a' && *token <= 'z') || (*token >= 'A' && *token <= 'Z'));
}

int is_int(char *token) {
    while (*token)
        if (!(*token >= '0' && *token++ <= '9'))
            return 0;
    return 1;
}

int is_char(char *token) {
	return (*token == '\'');
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

int is_storage(char *token) {
    return (!strcmp(token, "auto") || !strcmp(token, "register") || !strcmp(token, "static")
                || !strcmp(token, "extern") || !strcmp(token, "typedef"));
}

int is_type(char *token) {
    if (!strcmp(token, "void") || !strcmp(token, "char") || !strcmp(token, "short") || !strcmp(token, "int")
                || !strcmp(token, "long") || !strcmp(token, "float") || !strcmp(token, "double") || !strcmp(token, "signed")
                    || !strcmp(token, "unsigned"))
        return 1;
    return 0;
}

int is_qualifier(char *token) {
    return (!strcmp(token, "const") || !strcmp(token, "volatile"));
}