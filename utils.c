#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "barec.h"

void unscan(char *token, FILE *stream) {
    if (!*token)
        return;
    char *ptr = token;
    while (*ptr)
        ptr++;

    while (--ptr != token)
        f_ungetc(*ptr, stream);
    f_ungetc(*ptr, stream);
}

inline int is_id(char *token) {
    return (*token == '_' || (*token >= 'a' && *token <= 'z') || (*token >= 'A' && *token <= 'Z'));
}

inline int is_int(char *token) {
    while (*token) {
        if (!(*token >= '0' && *token <= '9'))
            return 0;
        token++;
    }
    return 1;
}

inline int is_float(char *token) {
    if (!(*token >= '0' && *token <= '9') && *token != '.')
        return 0;
    while (*token) {
        if (*token == '.' || *token == 'e' || *token == 'E')
            return 1;
        *token++;
    }
    return 0;
}

char *itoa(int value) {
    int n = 0;
    int num = value;
    if (num == 0) {
        return "0";
    }
    while (num > 0) {
        num /= 10;
        n++;
    }

    char *retptr = malloc(n+1);
    retptr[n] = 0;
    int i;
    for (i = 1; i <= n; i++) {
        retptr[n-i] = value%10 + '0';
        value /= 10;
    }
    return retptr;
}

list *list_node() {
    list *retptr = (list *)malloc(sizeof(list));
    memset(retptr, 0, sizeof(list));
    retptr->type = list_t;
    return retptr;
}

list *list_init(void *content) {
    list *retptr = (list *)malloc(sizeof(list));
    retptr->type = list_t;
    retptr->content = content;
    retptr->prev = 0;
    retptr->next = 0;
    return retptr;
}

/* init a list with an empty head */
list *list_init_wh(void *first_content) {
    list *retptr = list_node();
    retptr->next = list_init(first_content);
    return retptr;
}

void list_append(list *head, void *content) {
    list *node = list_node();
    node->content = content;
    while (head->next)
        head = head->next;
    head->next = node;
    node->prev = head;
}

void list_extend(list *first, list *second) {
    list *ptr = first;
    while (ptr->next)
        ptr = ptr->next;
    ptr->next = second;
    second->prev = ptr;
}

int list_len(list *head) {
    int len = 0;
    while(head = head->next)
        len++;
    return len;
}

list *list_copy(list *head)
{
    list *retptr = list_node(), *ptr, *ptr2 = retptr;
    for (ptr = head->next; ptr; ptr = ptr->next) {
        ptr2->next = list_node();
        ptr2->next->prev = ptr2;
        ptr2->next->content = ptr->content;
        ptr2 = ptr2->next;
    }
    return retptr;
}

buffer *buff_init() {
    buffer *retptr = (buffer *)malloc(sizeof(buffer));
    retptr->prev = 0;
    retptr->next = 0;
    retptr->head = retptr;
    return retptr;
}

// add to the head, empty tail
void buff_add(buffer *buff, char *string) {
    buff->head->prev = buff_init();
    buff->head->prev->next = buff->head;
    buff->head->prev->content = strdup(string);
    buff->head = buff->head->prev;
}

void buff_addln(buffer *buff, char *string) {
    int len = strlen(string);
    char new_string[len+3];
    strcpy(new_string, string);
    new_string[len] = '\n';
    new_string[len+1] = 0;
    buff_add(buff, new_string);
}

char *buff_puts(buffer *buff) {
    buffer *ptr = buff->head;
    int len = 0, offs = 0;
    if (!ptr->next)
        return strdup("");
    while (ptr->next) {
        len += strlen(ptr->content);
        ptr = ptr->next;
    }
    char *retptr = (char *)malloc(len+1);
    for (ptr = buff->prev; ptr; ptr = ptr->prev) {
        strcpy(retptr+offs, ptr->content);
        offs += strlen(ptr->content);
    }
    retptr[len] = 0;
    return retptr;
}

char *get_tag()
{
    char *t = itoa(tab++);
    char *retptr = (char *)malloc(strlen(t) + 1 + 1);
    *retptr = 't';
    strcpy(retptr+1, t);
    retptr[strlen(t)+1] = 0;
    return retptr;
}

static char *type_specifiers[] = {
    "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", 0
};

int is_specifier(char *token)
{
    char **ptr;
    for (ptr = type_specifiers; *ptr; ptr++)
        if (!strcmp(token, *ptr))
            return 1;
    if (!strcmp(token, "struct") || !strcmp(token, "union") || !strcmp(token, "enum"))
        return 1;
    return 0;
}

inline int is_storage(char *token)
{
    return !strcmp(token, "auto") || !strcmp(token, "register") || !strcmp(token, "static")
                 || !strcmp(token, "extern") || !strcmp(token, "typedef");
}

inline int is_qualifier(char *token)
{
    return !strcmp(token, "const") || !strcmp(token, "volatile");
}
