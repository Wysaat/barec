#ifndef PARSE_H
#define PARSE_H

#include "scan.h"
#include "list.h"

indirection *INDIRECTION(void *expr);
m_expr *M_EXPR(char *op, void *expr1, void *expr2);

void *arithmetic_get_value(void *vptr, buffer *buff);
void *declaration_get_value(void *vptr, buffer *buff);
void *indirection_get_value(void *vptr, buffer *buff);
void *assignment_get_value(void *vptr, buffer *buff);
void *expression_get_value(void *vptr, buffer *buff);
void *expression_stmt_get_value(void *vptr, buffer *buff);
void *get_value(void *expr, buffer *buff);

void declaration_get_addr(void *vptr, buffer *buff);
void indirection_get_addr(void *vptr, buffer *buff);
void get_addr(void *expr, buffer *buff);

void expression_stmt_gen_code(void *vptr, buffer *buff);
void gen_code(void *stmt, buffer *buff);

#endif /* PARSE_H */