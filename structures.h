typedef struct integer_constant integer_constant;
typedef struct character_constant character_constant;
typedef struct string string;

struct integer_constant {
    int type;
    char *value;
};

struct character_constant {
    int type;
    char *value;
};

struct string {
    int type;
    char *value;
    int hash;
};

struct identifier {
    int type;
    char *value;
    int hash;
    char *addr;
};

struct multiplicative_expression {
    int type;
    char *op;
    void *left;
    void *right;
}

struct assignment_expression {
    int type;
    void *left;
    void *right;
}