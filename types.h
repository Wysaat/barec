#ifndef TYPES_H
#define TYPES_H

enum types {
    auto_storage_t,
    int_specifier_t,
    declarator_t,
    id_declarator_t,
    array_declarator_t,
    declaration_t,
    integer_t,
    indirection_t,
    assignment_t,
    expression_t,
    expression_stmt_t,
};

enum atypes {
    unsigned_char_t = 0,  // 1 byte
    char_t = 1,    // 1 byte
    unsigned_short_t = 2, // 2 bytes
    short_t = 3,   // 2 bytes
    unsigned_int_t = 4,   // 4 bytes
    int_t = 5,     // 4 bytes
    unsigned_long_t = 6,  // 8 bytes
    long_t = 7,    // 8 bytes
    float_t = 8,          // 4 bytes
    double_t = 9,         // 8 bytes
    long_double_t = 10,   // 12 bytes
};

#endif /* TYPES_H */