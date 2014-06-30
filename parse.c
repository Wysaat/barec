#include <stdlib.h>
#include "scan.h"
#include "types.h"

int type(void *stmt) {
    return *(int *)stmt;
}

indirection *INDIRECTION(void *expr) {
    indirection *retptr = (indirection *)malloc(sizeof(indirection));
    retptr->type = indirection_t;
    retptr->expr = expr;
    return retptr;
}

m_expr *M_EXPR(char *op, void *expr1, void *expr2) {
	m_expr *retptr = (m_expr *)malloc(sizeof(m_expr));
	retptr->op = op;
	retptr->left = expr1;
	retptr->right = expr2;
	return retptr;
}