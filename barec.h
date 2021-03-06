#ifndef SCAN_H
#define SCAN_H

#include <stdio.h>

int is_static_addr;
int is_static_declaration;

typedef struct list list;
typedef struct buffer buffer;

buffer *data_buff, *bss_buff, *text_buff;

int data_size, bss_size;
struct namespace *namespace;
int tab;

list *func_list;

char *data_start_tag, *bss_start_tag;

char *continue_tag, *break_tag, *return_tag;

struct {
    char *file_name;
    int line;
    int lastline_column;
    int column;
    int error;
    int eof;
} file_info;

enum types {
    auto_storage_t,
    static_storage_t,
    parameter_storage_t,
    arithmetic_specifier_t,
    struct_specifier_t,
    union_specifier_t,
    void_specifier_t,
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
    if_stmt_t,
    switch_stmt_t,
    case_stmt_t,
    default_stmt_t,
    id_labeled_stmt_t,
    while_stmt_t,
    for_stmt_t,
    goto_stmt_t,
    continue_stmt_t,
    break_stmt_t,
    return_stmt_t,
    list_t,
    static_initialization_t,
    funcall_t,
    func_t,
    function_definition_type,
    typedef_type,
    typedef_storage_t,
    enum_type,
    union_ref_type,
    conditional_type,
    not_lvalue,
    constant_t,
    init_info_type,
    declarator_info_type,
    funcall_struct_ref_type,
    static_addr_type,
    static_offsetted_storage_type,
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

typedef struct static_addr {
    int type;
    int value;
    int initialized;
} static_addr_t;

typedef struct static_offsetted_storage {
    int type;
    struct static_storage *base;
    int offset;
} static_offsetted_storage_t;

typedef struct funcall funcall;

typedef struct {
    int type;
    funcall *primary;
    char *id;
} funcall_struct_ref_t;

typedef struct void_specifier {
    int type;
} void_specifier;

typedef struct namespace {
    list *struct_s_list;
    list *union_s_list;
    list *declaration_list;
    struct size *auto_size;
    struct namespace *outer;
    list *labels;  // nonzero iff the namespace is that of a function definition
    list *typedefs;
    list *enums;
    int is_func_ns;
    list *return_type_list;
} namespace_t;

typedef struct aconstant {
    int is_int;
    int ival;
    double fval;
} aconstant_t;

typedef struct {
    int type;
    void *expr1;
    void *expr2;
    void *expr3;
    list *type_list;
} conditional_t;

typedef struct {
    int type;
    void *primary;
    list *type_list;
} union_ref_t;

typedef struct {
    int type;
    char *id;
    list *type_list;
} typedef_t;

typedef struct {
    char *id;
    int value;
} enumerator_t;

typedef struct {
    int type;
    char *id;
    list *enumerators;
} enum_t;

typedef struct translation_unit {
    list *external_declarations;
} translation_unit_t;

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

typedef struct static_initialization {
    int type;
    struct compound_stmt *body;
    int initialized;
} static_initialization;

struct size {
    int constant;
    int ival;
    void *vval;
};

typedef struct typedef_storage {
    int type;
} typedef_storage;

typedef struct auto_storage {
    int type;
    int constant;
    int ival;
    void *vval;
} auto_storage;

typedef struct static_storage {
    int type;
    int ival;
    int initialized;
} static_storage;

// storage for function parameters
typedef struct parameter_storage {
    int type;
    int address;
} parameter_storage;

typedef struct arithmetic_specifier {
    int type;
    enum atypes atype;
    int bits;
} arithmetic_specifier;

typedef struct struct_specifier {
    int type;
    char *id;
    list *declaration_list;
} struct_specifier;

typedef struct union_specifier {
    int type;
    char *id;
    list *declaration_list;
} union_specifier;

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

typedef struct funcall {
    int type;
    void *primary;
    list *argument_expression_list;
    list *type_list;
} funcall;

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
    void *expr;
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

typedef struct if_stmt {
    int type;
    void *expr;
    void *statement1;
    void *statement2;
} if_stmt;

typedef struct switch_stmt {
    int type;
    void *expr;
    void *statement;
    char *break_tag;
} switch_stmt;

typedef struct case_stmt {
    int type;
    char *tag;
    char *value;
    void *statement;
} case_stmt;

typedef struct default_stmt {
    int type;
    char *tag;
    void *statement;
} default_stmt;

typedef struct id_labeled_stmt {
    int type;
    char *tag;
    char *id;
    void *statement;
} id_labeled_stmt;

typedef struct while_stmt {
    int type;
    void *expr;
    void *statement;
    int flag;  // 0 while, 1 do-while
    char *continue_tag;
    char *break_tag;
} while_stmt;

typedef struct for_stmt {
    int type;
    void *expr1;
    void *expr2;
    void *expr3;
    void *statement;
    char *continue_tag;
    char *break_tag;
} for_stmt;

typedef struct goto_stmt {
    int type;
    char *id;
    namespace_t *func_namespace;
} goto_stmt;

typedef struct continue_stmt {
    int type;
    char *tag;
} continue_stmt;

typedef struct break_stmt {
    int type;
    char *tag;
} break_stmt;

typedef struct return_stmt {
    int type;
    void *expr;
    char *tag;
} return_stmt;

typedef struct function_definition {
    int type;
    char *id;
    list *return_type_list;
    list *parameter_list;
    namespace_t namespace;
    compound_stmt *body;
    char *return_tag;
    char *function_tag;
} function_definition_t;

typedef struct func {
    int type;
    char *id;
    char *tag;
    list *type_list;
} func;

/*
 * scan.c
 */

char *scan(FILE *stream);

list *parse_specifier(FILE *stream, struct namespace *namespace);
declarator *parse_declarator(FILE *stream, int abstract, struct namespace *namespace);
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
void *parse_compound_stmt(FILE *stream, struct namespace *namespace, list *parameter_list, list *return_type_list);
void *parse_if_stmt(FILE *stream, namespace_t *namespace);
void *parse_switch_stmt(FILE *stream, namespace_t *namespace);
void *parse_case_stmt(FILE *stream, namespace_t *namespace);
void *parse_default_stmt(FILE *stream, namespace_t *namespace);
void *parse_id_labeled_stmt(FILE *stream, namespace_t *namespace);
void *parse_while_stmt(FILE *stream, namespace_t *namespace);
void *parse_do_while_stmt(FILE *stream, namespace_t *namespace);
void *parse_for_stmt(FILE *stream, namespace_t *namespace);
void *parse_goto_stmt(FILE *stream, namespace_t *namespace);
void *parse_continue_stmt(FILE *stream, namespace_t *namespace);
void *parse_break_stmt(FILE *stream, namespace_t *namespace);
void *parse_return_stmt(FILE *stream, namespace_t *namespace);
void error(FILE *stream, char *message);
void error_message(char *message);
void statement_error(char *token, FILE *stream);
void *size_to_expr(struct size *size);
void *size_expr(list *type_list);
aconstant_t *parse_a_conditional(FILE *stream);
arithmetic *parse_arithmetic_constant(FILE *stream);
enumerator_t *enumerator_init(char *id, int value);
typedef_storage *typedef_storage_init();
struct namespace *namespace_init(namespace_t *outer);
struct_specifier *find_struct_specifier(namespace_t *namespace, char *id);
union_specifier *find_union_specifier(namespace_t *namespace, char *id);
declaration *find_declaration(namespace_t *namespace, char *id);
void *find_identifier(namespace_t *namespace, char *id);
typedef_t *find_typedef(namespace_t *namespace, char *id);
enum_t *find_enum(namespace_t *namespace, char *id);
enumerator_t *find_enumerator(namespace_t *namespace, char *id);
declaration *find_declaration_cn(namespace_t *namespace, char *id);
typedef_t *find_typedef_cn(namespace_t *namespace, char *id);
enum_t *enum_init(char *id, list *enumerators);
list *parse_parameter_list(FILE *stream, struct namespace *namespace);
typedef_t *typedef_init(char *id, list *type_list);
void errorh();
func *func_init(char *id, char *tag, list *type_list);
inline struct size *size_cpy(struct size *ptr);
func *find_func(namespace_t *namespace, char *id);
void size_add(struct size *left, struct size *right);

/*
 * parse.c
 */

int type(void *);
auto_storage *auto_storage_init(int constant, int ival, void *vval);
static_storage *static_storage_init(int ival, int initialized);
void_specifier *void_specifier_init();
arithmetic_specifier *arithmetic_specifier_init(int atype);
struct_specifier *struct_specifier_init(char *id, list *declaration_list);
union_specifier *union_specifier_init(char *id, list *declaration_list);
pointer *pointer_init();
array *array_init(void *size);
function *function_init(list *parameter_list);
declarator *declarator_init(char *id, list *type_list);
declaration *declaration_init(char *id, void *storage, list *type_list);
parameter *parameter_init(void *storage, list *type_list);
arithmetic *ARITHMETIC(char *value, int atype);
string *STRING(int address, char *value);
void *ARRAY_REF(void *primary, void *expr);
void *STRUCT_REF(void *primary, char *id);
posinc *POSINC(void *primary, int inc);
funcall *FUNCALL(void *primary, list *argument_expression_list);
preinc *PREINC(void *expr, int inc);
void *ADDR(void *expr);
void *INDIRECTION(void *expr);
unary *UNARY(void *expr, char *op);
cast *CAST(list *type_list, void *expr);
binary *BINARY(btype_t btype, void *left, void *right);
assignment *ASSIGNMENT(void *expr1, void *expr2);
expression *EXPRESSION(list *assignment_list, list *type_list);
expression_stmt *EXPRESSION_STMT(list *assignment_list);
compound_stmt *COMPOUND_STMT(list *declaration_statement_list, namespace_t *namespace);
if_stmt *IF_STMT(void *expr, void *statement1, void *statement2);
switch_stmt *SWITCH_STMT(void *expr, void *statement, char *break_tag);
case_stmt *CASE_STMT(char *tag, char *value, void *statement);
default_stmt *DEFAULT_STMT(char *tag, void *statement);
id_labeled_stmt *ID_LABELED_STMT(char *tag, char *id, void *statement);
while_stmt *WHILE_STMT(void *expr, void *statement, int flag, char *continue_tag, char *break_tag);
for_stmt *FOR_STMT(void *expr1, void *expr2, void *expr3, void *statement, char *continue_tag, char *break_tag);
goto_stmt *GOTO_STMT(char *id, namespace_t *func_namespace);
continue_stmt *CONTINUE_STMT(char *tag);
break_stmt *BREAK_STMT(char *tag);
return_stmt *RETURN_STMT(void *expr, char *tag);
arithmetic_specifier *integral_promotion(arithmetic_specifier *s);
list *integral_promotion2(list *type_list);
list *get_type_list(void *vptr);
void set_type_list(list *type_list, void *vptr);
struct size *size_init(int constant, int ival, void *vval);
struct size *size(list *type_list);
void storage_add_size(auto_storage *left, struct size *right);
auto_storage *auto_storage_add_size_nip(auto_storage *left, struct size *right);
auto_storage *auto_storage_sub_size_nip(auto_storage *left, struct size *right);
static_addr_t *static_addr_init(int value, int initialized);
inline declaration *declaration_copy(declaration *node);
static_offsetted_storage_t *static_offsetted_storage_init(static_storage *base,  int offset);

/*
 * utils.c
 */

#define is_str(token) (*token == '"')
#define is_char(token) (*token == '\'')

void unscan(char *token, FILE *stream);
int is_id(char *token);
int is_int(char *token);
inline int is_float(char *token);
char *itoa(int value);

char *get_tag();

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
list *list_init_wh(void *first_content); /* it has an empty head */
void list_append(list *, void *);
void list_extend(list *first, list *second);
buffer *buff_init();
void buff_add(buffer *buff, char *string);
void buff_addln(buffer *buff, char *string);
char *buff_puts(buffer *buff);
int is_specifier(char *token);
inline int is_storage(char *token);
inline int is_qualifier(char *token);
int list_len(list *);
list *list_copy(list *head);

/*
 * gencode.c
 */

void gencode(void *expr);
void function_definition_gencode(function_definition_t *function_definition);

/*
 * check.c
 */

typedef struct {
    int type;
    char *id;
    list *type_list;
    int line;
    int column;
} declarator_info_t;

declarator_info_t *syntax_declarator(FILE *stream, namespace_t *namespace, int abstract);
void syntax_declaration(FILE *stream, namespace_t *namespace, int in_struct);
list *syntax_postfix(FILE *stream, namespace_t *namespace);
list *syntax_unary(FILE *stream, namespace_t *namespace);
list *syntax_cast(FILE *stream, namespace_t *namespace);
list *syntax_conditional(FILE *stream, namespace_t *namespace);
list *syntax_assignment(FILE *stream, namespace_t *namespace);
list *syntax_expression(FILE *stream, namespace_t *namespace);
void syntax_expression_stmt(FILE *stream, namespace_t *namespace);
void syntax_statement(FILE *stream, namespace_t *namespace);
void syntax_declaration_or_statement(FILE *stream, namespace_t *namespace);
void syntax_compound_stmt(FILE *stream, namespace_t *namespace, list *parameter_list, int is_func);
void syntax_if_stmt(FILE *stream, namespace_t *namespace);
void syntax_switch_stmt(FILE *stream, namespace_t *namespace);
void syntax_case_stmt(FILE *stream, namespace_t *namespace);
void syntax_default_stmt(FILE *stream, namespace_t *namespace);
void syntax_id_labeled_stmt(FILE *stream, namespace_t *namespace);
void syntax_while_stmt(FILE *stream, namespace_t *namespace);
void syntax_do_while_stmt(FILE *stream, namespace_t *namespace);
void syntax_for_stmt(FILE *stream, namespace_t *namespace);
void syntax_goto_stmt(FILE *stream, namespace_t *namespace);
void syntax_continue_stmt(FILE *stream, namespace_t *namespace);
void syntax_break_stmt(FILE *stream, namespace_t *namespace);
void syntax_return_stmt(FILE *stream, namespace_t *namespace);
void syntax_external_declaration(FILE *stream, namespace_t *namespace);
void syntax_translation_unit(FILE *stream);
list *syntax_a_conditional(FILE *stream, namespace_t *namespace);
void error_message_np(int line, int column, char *message);
void errorh_np(int line, int column);
int type_list_eq(list *left, list *right);
char *type_list_to_str(list *type_list);
inline list *type_list_copy(list *type_list);

#endif /* SCAN_H */