char *scan(FILE *stream);

typedef struct scope scope;
typedef struct val_list val_list;

struct scope {
    val_list *vals;
    int next_addr;
};

struct val_list {
    char *name;
    int addr;
    val_list *prev;
    val_list *next;
};