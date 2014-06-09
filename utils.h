#ifndef UTILS_H
#define UTILS_H

void unscan(char *token, FILE *stream);
void emit(char *words);
void emitln(char *words);
int is_id(char *token);
int is_integer_const(char *token);
int is_character_const(char *token);
char *itoa(int value);

#endif /* UTILS_H */