char *integer_constant_code_gen(void *vptr, scope *s) {
    integer_constant *ptr = (integer_constant *)vptr;
    char *ret = (char *)malloc(strlen("mov    eax, \n")+strlen(ptr->value)+1);
    sprintf(ret, "mov    eax, %s\n", ptr->value);
    return ret;
}

char *char_constant_code_gen(void *vptr, scope *s) {
    char_constant *ptr = (char_constant *)vptr;
    char *ret = (char *)malloc(strlen("mov    eax, \n")+strlen(ptr->value)+1);
    sprintf(ret, "mov    eax, %s\n", ptr->value);
    return ret;
}

char *identifier_code_gen(void *vptr, scope *s) {
    identifier *ptr = (identifier *)vptr;
    declaration *d = get_declaration(s, vptr);
    char *ret = (char *)malloc(strlen("lea    eax, []\n")+strlen(d->addr)+1);
    sprintf(ret, "lea    eax, [%s]\n", d->addr);
    return ret;
}

char *string_code_gen(void *vptr, scope *s) {
    string *ptr = (string *)vptr;
    char *ret = (char *)malloc(strlen("mov    eax, tag\n")+strlen(itoa(ptr->hash)));
    sprintf(ret, "mov    eax, tag%d\n", ptr->hash);
    return ret;
}

char *multiplicative_expression_code_gen(void *vptr, scope *s) {
    multiplicative_expressioin *ptr = (multiplicative_expressioin *)vptr;
    char *ret = code_gen(ptr->left);
    ret = stradd(ret, "push    eax\n");
    ret = stradd(ret, code_gen(ptr->right));
    if (!strcmp(ptr->op, "*")) {
        ret = stradd(ret, "mul    ebx\n");
    }
    else if (!strcmp(ptr->op, "/")) {
        ret = stradd(ret, "div    ebx\n");
    }
    else if (!strcmp(ptr->op, "%")) {
        ret = stradd(ret, "div    ebx\n"
                          "xchg   eax, ebx\n");
    }
}

char *assignment_code_gen(void *vptr, scope *s) {
    assignment *ptr = (assignment *)vptr;
    char *addr = get_addr(ptr->left);
    char *type = get_type(ptr->left);
    char *ret = stradd("mov    [", addr);
    if (!strcmp(type, "int")) {
        ret = stradd(ret, "], dword eax");
    }
    else if (!strcmp(type, "char")) {
        ret = stradd(ret, "], byte eax");
    }
    ret = stradd(code_gen(ptr->right), ret);
    return ret;
}
