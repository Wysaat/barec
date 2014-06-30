#include "scan.h"
#include "parse.h"
#include "types.h"

void expression_stmt_gen_code(void *vptr, buffer *buff) {
    expression_stmt *stmt = (expression_stmt *)vptr;
    list *ptr;
    for (ptr = stmt->assignment_list->next; ptr; ptr = ptr->next)
        get_value(ptr->content, buff);
}

void gen_code(void *stmt, buffer *buff) {
    switch (type(stmt)) {
        case expression_stmt_t:
            expression_stmt_gen_code(stmt, buff);
            break;
    }
}