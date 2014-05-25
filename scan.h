#ifndef SCAN_H
#define SCAN_H

#include "scope.h"

int tag = 0;

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
void parse_equality_expression(FILE *stream, scope *local_scope);
void parse_and_expression(FILE *stream, scope *local_scope);
void parse_exclusive_or_expression(FILE *stream, scope *local_scope);
void parse_inclusive_or_expression(FILE *stream, scope *local_scope);
void parse_logical_and_expression(FILE *stream, scope *local_scope);
void parse_logical_or_expression(FILE *stream, scope *local_scope);
void parse_conditional_expression(FILE *stream, scope *local_scope);
void parse_assignment_expression(FILE *stream, scope *local_scope);
void parse_expression(FILE *stream, scope *local_scope);
void parse_expression_statement(FILE *stream, scope *local_scope);
void parse_statement(FILE *stream, scope *local_scope);
void parse_declaration(FILE *stream, scope *local_scope, int decl);
void emit(char *words);

#endif /* SCAN_H */