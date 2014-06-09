#include "barec.h"

declaration_specifier *declaration_specifier_init(int storage_class, int type_qualifier, void *type_specifier) {
    declaration_specifier *retptr = (declaration_specifier *)malloc(sizeof(declaration_specifier));
    retptr->storage_class = storage_class;
    retptr->type_qualifier = type_qualifier;
    retptr->type_specifier = type_specifier;
    return retptr;
}

integral_type_specifier *integral_type_specifier_init() {
    integral_type_specifier *retptr = (integral_type_specifier *)malloc(sizeof(integral_type_specifier));
    memset(retptr, 0, sizeof(integral_type_specifier));
    retptr->type = integral_type_specifier_t;
    return retptr;
}

declaration *declaration_init(declaration_specifier *dec_spec, list *init_declarator_list) {
    declaration *retptr = (declaration *)malloc(sizeof(declaration));
    retptr->type = declaration_t;
    retptr->dec_spec = dec_spec;
    retptr->init_declarator_list = init_declarator_list;
    return retptr;
}