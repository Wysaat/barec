#ifndef SCAN_H
#define SCAN_H

#include "barec.h"

int stack_pointer;
list *declarations;

typedef struct auto_storage {
    int type;
    int stack_position;
} auto_storage;

typedef struct int_specifier {
    int type;
    void *storage;  // storage class
    // int qualifier; // type qualifier
    int sign;
    int size;
} int_specifier;  // integral declaration specifier

typedef struct declarator {
    int pointers;
    char *id;
} declarator;

typedef struct init_declarator {
    declarator *declarator;
    void *initializer;  // can be 0
} init_declarator;

typedef struct declaration {
    void *specifier;
    list *init_declarator_list;
} declaration;

typedef struct integer {
    int type;
    char *value;
} integer;

typedef struct array_ref {
    int type;
    void *primary;
    void *expr2;
} array_ref;

typedef struct declaration_node {
    int type;
    int pointers;
    char *id;
    void *specifier;
    int addr;
} declaration_node;

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

void *parse_specifierr(FILE *stream);
declarator *parse_declarator(FILE *stream);
expression_stmt *parse_declaration(FILE *stream);
void *parse_primary(FILE *stream);
void *parse_assignment(FILE *stream);
void *parse_expression(FILE *stream);
void *pares_expression_stmt(FILE *stream);

int type(void *);

#endif /* SCAN_H */