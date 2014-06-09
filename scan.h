#ifndef SCAN_H
#define SCAN_H

#include "barec.h"

typedef struct integral_type_specifier {
    int type;
    int sign;
    int size;
} integral_type_specifier;

typedef struct declaration_specifier {
    int storage_class;
    int type_qualifier;
    void *type_specifier;
} declaration_specifier;

typedef struct declarator {
    int pointers;
    char *id;
} declarator;

typedef struct init_declarator {
    declarator *declarator;
    void *initializer;
} init_declarator;

typedef struct declaration {
    int type;
    declaration_specifier *dec_spec;
    list *init_declarator_list;
} declaration;

char *scan(FILE *stream);

void *pare_declaration_specifier(FILE *stream);
void *pare_declaration(FILE *stream);
void *parse_compound_stmt(FILE *stream);
declaration_specifier *declaration_specifier_init(int storage_class, int type_qualifier, void *type_specifier);
integral_type_specifier *integral_type_specifier_init();
declaration *declaration_init(declaration_specifier *, list *);

void *parse_assignment(FILE *stream);

#endif /* SCAN_H */