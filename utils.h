#ifndef UTILS_H
#define UTILS_H

void unscan(char *token, FILE *stream);
void emit(char *words);
void emitln(char *words);
int is_id(char *token);
int is_int(char *token);
int is_char(char *token);
char *itoa(int value);
int is_sto_class_spe(char *token);
int is_type_spe(char *token);
int is_type_qua(char *token);

#endif /* UTILS_H */