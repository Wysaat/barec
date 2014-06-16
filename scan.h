#ifndef SCAN_H
#define SCAN_H

#include "barec.h"

int stack_pointer;

typedef struct int_specifier {
    int type;
    int storage;  // storage classs
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

typedef struct int_expr {
    char *value;
} int_expr;

typedef struct compound_stmt {
    int type;
    list *stmt_list;
} compound_stmt;

char *scan(FILE *stream);

void *parse_specifierr(FILE *stream);
void *parse_declarator(FILE *stream);
void *parse_declaration(FILE *stream);
void *parse_compound_stmt(FILE *stream);

void *parse_assignment(FILE *stream);

#endif /* SCAN_H */