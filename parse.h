void *parse_constant(FILE *stream);
void *parse_primary_expression(FILE *stream);
void *parse_conditional_expression(FILE *stream);
void *parse_assignment_expression(FILE *stream);

char *storage_class_specifiers[] = {
    "auto", "register", "static", "extern", "typedef",
}

char *type_specifiers[] = {
    "void", "char", "short", "int", "long", "float", "double", "signed",
    "unsigned", /* struct_or_union_specifier, enum_specifier, typedef_name, */
};

char *type_qualifiers[] = {
    "const", "volatile",
};