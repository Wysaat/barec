#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

void unscan(char *token, FILE *stream);
void emit(char *words);
void emitln(char *words);
int is_id(char *token);
int is_int(char *token);
int is_char(char *token);
char *itoa(int value);
int is_storage(char *token);
int is_type(char *token);
int is_qualifier(char *token);

#endif /* UTILS_H */