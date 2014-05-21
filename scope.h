#ifndef SCOPE_H
#define SCOPE_H

typedef struct scope scope;
typedef struct val_list val_list;

struct scope {
    val_list *vals;
    int next_addr;
};

struct val_list {
    char *name;
    int addr;
    int index;
    val_list *prev;
    val_list *next;
};

scope *scope_init();

val_list *val_list_node(char *name, int addr);
void val_list_append(val_list *head, char *name, int addr);

#endif /* SCOPE_H */