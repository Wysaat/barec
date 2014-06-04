#include <stdlib.h>
#include <string.h>

integer_constant *INTEGER_CONSTANT(char *token) {
    integer_constant *retptr = (integer_constant *)malloc(sizeof(integer_constant));
    memset(retptr, 0, sizeof(*retptr));
    retptr->type = integer_constant_t;
    retptr->value = strdup(token);
    return retptr;
}

character_constant *CHARACTER_CONSTANT(char *token) {
    character_constant *retptr = (character_constant *)malloc(sizeof(character_constant));
    memset(retptr, 0, sizeof(*retptr));
    retptr->type = character_constant_t;
    retptr->value = strdup(token);
    return retptr;
}

identifier *IDENTIFIER(char *token, scope *s) {
    identifier *retptr = (identifier *)malloc(sizeof(identifier));
    memset(retptr, 0, sizeof(*retptr));
    retptr->type = identifier_t;
    retptr->value = strdup(token);
    retptr->hash = tag++;
    retptr->declaration = find_declaration(s, retptr->value);
    return retptr;
}

string *STRING(char *token) {
    string *retptr = (string *)malloc(sizeof(string));
    memset(retptr, 0, sizeof(*retptr));
    retptr->type = string_t;
    retptr->value = strdup(token+1);
    retptr->value[strlen(retptr->value)-2] = 0;
    retptr->hash = tag++;
    return retptr;
}

assignment_expression *ASSIGNMENT_EXPRESSION(void *left, void *right) {
    assignment_expression *retptr = (assignment_expression *)malloc(sizeof(assignment_expression));
    retptr->type = assignment_expression_t;
    retptr->left = left;
    retptr->right = right;
    return retptr;
}