#ifndef SCAN_H
#define SCAN_H

#include "barec.h"

int stack_pointer = 0;
list *declaration_list = list_node();

typedef struct auto_storage {
    int type;
    int stack_position;
} auto_storage;

typedef struct int_specifier {
    int type;
    int sign;
    int size;
} int_specifier;  // integral declaration specifier

typedef struct declarator {
    int type;
    char *id;
    int pointers;
    void *direct_declarator;
} declarator;

typedef struct id_declarator {
    int type;
    char *id;
} id_declarator;

typedef struct array_declarator {
    int type;
    char *id;
    void *direct_declarator;
    void *size_expr;
} array_declarator;

typedef struct init_declarator {
    declarator *declarator;
    void *initializer;  // can be 0
} init_declarator;

typedef struct integer {
    int type;
    char *value;
} integer;

typedef struct array_ref {
    int type;
    void *primary;
    void *expr2;
} array_ref;

typedef struct indirection {
    int type;
    void *expr;
} indirection;

typedef struct declaration {
    int type;
    char *id;
    void *storage;
    void *specifier;
    declarator *declarator;
} declarator;

typedef struct assignment {
    int type;
    void *expr1;
    void *expr2;
} assignment;

typedef struct expression {
    int type;
    list *assignment_list;
} expression;

typedef struct expression_stmt {
    int type;
    list *assignment_list;
} expression_stmt;

char *scan(FILE *stream);

list *parse_specifier(FILE *stream);
declarator *parse_declarator(FILE *stream);
void *parse_direct_declarator(FILE *stream);
expression_stmt *parse_declaration(FILE *stream);
void *parse_primary(FILE *stream);
void *parse_assignment(FILE *stream);
void *parse_expression(FILE *stream);
void *pares_expression_stmt(FILE *stream);

int type(void *);

#endif /* SCAN_H */