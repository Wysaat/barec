#ifndef SCAN_H
#define SCAN_H

#include <stdio.h>

typedef struct list list;
typedef struct buffer buffer;

buffer *data_buff, *bss_buff, *text_buff;

int data_size;
struct namespace *namespace;
int tab;

enum types {
    auto_storage_t,
    static_storage_t,
    extern_storage_t,
    parameter_storage_t,
    arithmetic_specifier_t,
    struct_specifier_t,
    pointer_t,
    array_t,
    function_t,
    parameter_t,
    declarator_t,
    id_declarator_t,
    array_declarator_t,
    declaration_t,
    arithmetic_t,
    string_t,
    struct_ref_t,
    posinc_t,
    preinc_t,
    addr_t,
    indirection_t,
    unary_t,
    cast_t,
    binary_t,
    assignment_t,
    expression_t,
    expression_stmt_t,
    compound_stmt_t,
    list_t,
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

typedef struct namespace {
    list *struct_s_list;
    list *declaration_list;
    struct size *auto_size;
    struct namespace *outer;
} namespace_t;

typedef enum btype {
    mul,
    divi,
    mod,
    add,
    sub,
    lshift,
    rshift,
    lt,
    gt,
    le,
    ge,
    eq,
    neq,
    band,
    bxor,
    bor,
    land,
    lor,
} btype_t;

struct size {
    int constant;
    int ival;
    void *vval;
};

typedef struct auto_storage {
    int type;
    int constant;
    int iaddress;
    void *vaddress;
} auto_storage;

typedef struct static_storage {
    int type;
    int address;
} static_storage;

// storage for function parameters
typedef struct parameter_storage {
    int type;
    int address;
} parameter_storage;

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

typedef struct function {
    int type;
    list *parameter_list;
} function;

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

typedef struct parameter {
    int type;
    void *storage;
    list *type_list;
} parameter;

typedef struct arithmetic {
    int type;
    char *value;
    list *type_list;
} arithmetic;

typedef struct string {
    int type;
    int address;
    char *value;
    list *type_list;
} string;

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

typedef struct cast {
    int type;
    list *type_list;
    struct cast *expr;
} cast;

typedef struct binary {
    int type;
    btype_t btype;
    void *left;
    void *right;
    list *type_list;
} binary;

typedef struct assignment {
    int type;
    void *expr1;
    void *expr2;
} assignment;

typedef struct expression {
    int type;
    list *assignment_list;
    list *type_list;
} expression;

typedef struct expression_stmt {
    int type;
    list *assignment_list;
} expression_stmt;

typedef struct compound_stmt {
    int type;
    list *statement_list;
    namespace_t *namespace;
} compound_stmt;

typedef struct function_definition {
    char *id;
    list *return_type_list;
    list *parameter_list;
    namespace_t namespace;
    compound_stmt *body;
} function_definition_t;

/*
 * scan.c
 */

char *scan(FILE *stream);

list *parse_specifier(FILE *stream, struct namespace *namespace);
declarator *parse_declarator(FILE *stream, int abstract, struct namespace *namespace, int is_funcdef);
void *parse_declaration(FILE *stream, struct namespace *namespace, int in_struct);
list *parse_type_name(FILE *stream, struct namespace *namespace);
void *parse_primary(FILE *stream, namespace_t *namespace);
void *parse_postfix(FILE *stream, namespace_t *namespace);
void *parse_unary(FILE *stream, namespace_t *namespace);
void *parse_cast(FILE *stream, namespace_t *namespace);
void *parse_m_expr(FILE *stream, namespace_t *namespace);
void *parse_a_expr(FILE *stream, namespace_t *namespace);
void *parse_conditional(FILE *stream, namespace_t *namespace);
void *parse_assignment(FILE *stream, namespace_t *namespace);
void *parse_expression(FILE *stream, namespace_t *namespace);
void *pares_expression_stmt(FILE *stream, namespace_t *namespace);
void *parse_statement(FILE *stream, struct namespace *namespace);
void *parse_declaration_or_statement(FILE *stream, struct namespace *namespace);
void *parse_compound_stmt(FILE *stream, struct namespace *namespace, list *parameter_list);
void *size_to_expr(struct size *size);

/*
 * parse.c
 */

int type(void *);
auto_storage *auto_storage_init(int constant, int iaddress, void *vaddress);
static_storage *static_storage_init();
arithmetic_specifier *arithmetic_specifier_init(int atype);
struct_specifier *struct_specifier_init(char *id, list *declaration_list);
pointer *pointer_init();
array *array_init(void *size);
function *function_init(list *parameter_list);
declarator *declartor_init(char *id, list *type_list);
declaration *declaration_init(char *id, void *storage, list *type_list);
parameter *parameter_init(void *storage, list *type_list);
arithmetic *ARITHMETIC(char *value, int atype);
string *STRING(int address, char *value);
indirection *ARRAY_REF(void *primary, void *expr);
declaration *STRUCT_REF(void *primary, char *id);
posinc *POSINC(void *primary, int inc);
preinc *PREINC(void *expr, int inc);
addr *ADDR(void *expr);
indirection *INDIRECTION(void *expr);
unary *UNARY(void *expr, char *op);
cast *CAST(list *type_list, cast *expr);
binary *BINARY(btype_t btype, void *left, void *right);
assignment *ASSIGNMENT(void *expr1, void *expr2);
expression *EXPRESSION(list *assignment_list, list *type_list);
expression_stmt *EXPRESSION_STMT(list *assignment_list);
compound_stmt *COMPOUND_STMT(list *declaration_statement_list, namespace_t *namespace);
arithmetic_specifier *integral_promotion(arithmetic_specifier *s);
list *integral_promotion2(list *type_list);
list *get_type_list(void *vptr);
void set_type_list(list *type_list, void *vptr);
struct size *size_init(int constant, int ival, void *vval);
struct size *size(list *type_list);

/*
 * utils.c
 */

#define is_str(token) (*token == '"')
#define is_char(token) (*token == '\'')

void unscan(char *token, FILE *stream);
int is_id(char *token);
int is_int(char *token);
char *itoa(int value);

char *get_tab();

/* the list has an empty head */
struct list {
    int type;
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
list *list_init(void *content); /* it DOESN'T make a list with an empty head! */
void list_append(list *, void *);
void list_extend(list *first, list *second);
buffer *buff_init();
void buff_add(buffer *buff, char *string);
void buff_addln(buffer *buff, char *string);
char *buff_puts(buffer *buff);
int is_specifier(char *token);
inline int is_storage(char *token);
inline int is_qualifier(char *token);

/*
 * gencode.c
 */

void gencode(void *expr);
void function_definition_gencode(function_definition_t *function_definition);

#endif /* SCAN_H */