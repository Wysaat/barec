typedef struct scope {
    list *declarations;
} scope;

typedef struct list list;
struct list {
    void *content;
    list *prev;
    list *next;
};

typedef struct declaration declaration;
struct declaration {
    char *name;
    char *addr;
    char *type;
};