#include "barec.h"

list *list_node() {
    list *retptr = (list *)malloc(sizeof(list));
    memset(retptr, 0, sizeof(list));
    return retptr;
}

void list_append(list *head, void *content) {
    list *node = list_node();
    node->content = content;
    if (!head->next)
        head->next = node;
    else {
        list *ptr = head->next;
        while (ptr->next)
            ptr = ptr->next;
        ptr->next = node;
    }
}

buffer *buff_init() {
    buffer *retptr = (buffer *)malloc(sizeof(buffer));
    retptr->next = 0;
    return retptr;
}

// add to the head
void buff_add(buffer *buff, char *string) {
    buff->prev = buff_init();
    buff->prev->next = buff;
    buff->prev->content = strdup(string);
}

void buff_addln(buffer *buff, char *string) {
    int len = strlen(string);
    char new_string[len+2];
    strcpy(new_string, string);
    new_string[len] = '\n';
    new_string[len+1] = 0;
    buff_add(buff, new_string);
}

char *buff_puts(buffer *buff) {
    buffer *ptr = buff;
    int len = 0, offs = 0;
    while (ptr->next) {
        len += strlen(ptr->content);
        ptr = ptr->next;
    }
    char *retptr = (char *)malloc(len+1);
    for ( ; ptr->prev; ptr = ptr->prev) {
        strcpy(retptr+offs, ptr->content);
        offs += strlen(ptr->content);
    }
    retptr[len] = 0;
    return retptr;
}