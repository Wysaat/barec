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

void *get_specifier(void *vptr) {
    switch (tpye(vptr)) {
        case arithmetic_t:
            return ((arithmetic *)vptr)->specifier;
        case declaration_t:
            return ((declaration *)vptr)->specifier;
        case indirection_t:
            return ((indirection *)vptr)->specifier;
        case m_expr_t:
            return ((m_expr *)vptr)->specifier;
    }
}