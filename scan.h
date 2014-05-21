#ifndef SCAN_H
#define SCAN_H

#include "scope.h"

char *scan(FILE *stream);
void parse_constant(FILE *stream, scope *local_scope);
void parse_primary_expression(FILE *stream, scope *local_scope);
void parse_postfix_expression(FILE *stream, scope *local_scope);
void parse_unary_expression(FILE *stream, scope *local_scope);
void parse_cast_expression(FILE *stream, scope *local_scope);
void parse_multiplicative_expression(FILE *stream, scope *local_scope);
void parse_additive_expression(FILE *stream, scope *local_scope);
void parse_shift_expression(FILE *stream, scope *local_scope);
void parse_relational_expression(FILE *stream, scope *local_scope);
void parse_conditional_expression(FILE *stream, scope *local_scope);
void parse_assignment_expression(FILE *stream, scope *local_scope);
void parse_expression(FILE *stream, scope *local_scope);
void parse_expression_statement(FILE *stream, scope *local_scope);
void parse_statement(FILE *stream, scope *local_scope);
int parse_declaration(FILE *stream, scope *local_scope);
void emit(char *words);
int declare(char *token, int type, scope *local_scope);

#endif /* SCAN_H */