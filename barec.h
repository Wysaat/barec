#ifndef SCAN_H
#define SCAN_H

#include <stdio.h>

typedef struct list list;
typedef struct buffer buffer;

buffer *data_buff, *bss_buff, *text_buff;

void *auto_size;
int data_size;
list *struct_s_list;
list *declaration_list;

enum types {
    auto_storage_t,
    static_storage_t,
    extern_storage_t,

    arithmetic_specifier_t,
    struct_specifier_t,

    declarator_t,
    id_declarator_t,
    array_declarator_t,
    declaration_t,
    arithmetic_t,
    string_t,
    array_ref_t,
    struct_ref_t,
    posinc_t,
    preinc_t,
    addr_t,
    indirection_t,
    unary_t,
    sizeof_t,
    cast_t,
    m_expr_t,
    a_expr_t,
    assignment_t,
    expression_t,
    expression_stmt_t,

    pointer_t,
    array_t,
};

enum atypes {
    unsigned_char_t = 0,  // 1 byte
    char_t = 1,    // 1 byte
    unsigned_short_t = 2, // 2 bytes
    short_t = 3,   // 2 bytes
    unsigned_int_t = 4,   // 4 bytes
    int_t = 5,     // 4 bytes
    unsigned_long_t = 6,  // 4 bytes
    long_t = 7,    // 4 bytes
    unsigned_long_long_t = 8, // 8 bytes
    long_long_t = 9, // 8 bytes
    float_t = 10,          // 4 bytes
    double_t = 11,         // 8 bytes
    long_double_t = 12,   // 12 bytes
};

typedef struct auto_storage {
    int type;
    void *address;
} auto_storage;

typedef struct static_storage {
    int type;
    int address;
} static_storage;

typedef struct arithmetic_specifier {
    int type;
    enum atypes atype;
} arithmetic_specifier;

typedef struct struct_specifier {
    int type;
    char *id;
    list *declaration_list;
} struct_specifier;

typedef struct pointer {
    int type;
} pointer;

typedef struct array {
    int type;
    void *size;
} array;

typedef struct declarator {
    int type;
    char *id;
    list *type_list;
} declarator;

typedef struct declaration {
    int type;
    char *id;
    void *storage;
    list *type_list;
} declaration;

typedef struct arithmetic {
    int type;
    char *value;
    arithmetic_specifier *specifier;
} arithmetic;

typedef struct string {
    int type;
    int address;
    char *value;
} string;

typedef struct array_ref {
    int type;
    void *primary;
    void *expr;
    list *type_list;
} array_ref;

typedef struct struct_ref {
    int type;
    void *primary;
    char *id;
    list *type_list;
} struct_ref;

typedef struct posinc {
    int type;
    void *primary;
    int inc;
    list *type_list;
} posinc;

typedef struct preinc {
    int type;
    void *expr;
    int inc;
    list *type_list;
} preinc;

typedef struct addr {
    int type;
    void *expr;
    list *type_list;
} addr;

typedef struct indirection {
    int type;
    void *expr;
    list *type_list;
} indirection;

typedef struct unary {
    int type;
    char *op;
    void *expr;
    list *type_list;
} unary;

typedef struct size {
    int type;
    void *expr;
    list *type_list;
} size;

typedef struct cast {
    int type;
    list *type_list;
    struct cast *expr;
} cast;

typedef struct m_expr {
    int type;
    char *op;
    void *left;
    void *right;
    list *type_list;
} m_expr;

typedef struct a_expr {
    int type;
    char *op;
    void *left;
    void *right;
    list *type_list;
} a_expr;

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
void parse_declaration(FILE *stream, list *declaration_list);
list *parse_type_name(FILE *stream);
void *parse_primary(FILE *stream);
void *parse_postfix(FILE *stream);
void *parse_unary(FILE *stream);
void *parse_cast(FILE *stream);
void *parse_m_expr(FILE *stream);
void *parse_a_expr(FILE *stream);
void *parse_conditional(FILE *stream);
void *parse_assignment(FILE *stream);
void *parse_expression(FILE *stream);
void *pares_expression_stmt(FILE *stream);

/*
 * parse.c
 */

int type(void *);

auto_storage *auto_storage_init();
static_storage *static_storage_init();
arithmetic_specifier *arithmetic_specifier_init(int atype);
struct_specifier *struct_specifier_init(char *id, list *declaration_list);
pointer *pointer_init();
array *array_init(void *size);
declarator *declartor_init(char *id, list *type_list);
declaration *declaration_init(char *id, void *storage, list *type_list);
arithmetic *ARITHMETIC(char *value, int atype);
string *STRING(int address, char *value);
array_ref *ARRAY_REF(void *primary, void *expr);
struct_ref *STRUCT_REF(void *primary, char *id);
posinc *POSINC(void *primary, int inc);
preinc *PREINC(void *expr, int inc);
addr *ADDR(void *expr);
indirection *INDIRECTION(void *expr);
unary *UNARY(void *expr, char *op);
size *SIZE(void *expr);
cast *CAST(list *type_list, cast *expr);
m_expr *M_EXPR(char *op, void *expr1, void *expr2);
a_expr *A_EXPR(char *op, void *expr1, void *expr2);
arithmetic_specifier *arithmetic_convertion(arithmetic_specifier *ls, arithmetic_specifier *rs);
list *get_type_list(void *vptr);

/*
 * utils.c
 */

#define is_str(token) (*token == '"')
#define is_char(token) (*token == '\'')

void unscan(char *token, FILE *stream);
int is_id(char *token);
int is_int(char *token);
char *itoa(int value);

/* the list has an empty head */
struct list {
    void *content;
    list *prev;
    list *next;
};

typedef struct buffer {
    char *content;
    buffer *prev;
    buffer *next;
    buffer *head;
} buffer;

list *list_node();
list *list_init(void *content);
void list_append(list *, void *);
void list_extend(list *first, list *second);
buffer *buff_init();
void buff_add(buffer *buff, char *string);
void buff_addln(buffer *buff, char *string);
char *buff_puts(buffer *buff);

/*
 * gencode.c
 */

void gencode(void *expr);

#endif /* SCAN_H */