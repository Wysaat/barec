#include "../barec.h"

int main()
{
    buffer *buff = buff_init();
    buff_add(buff, "hello");
    buff_add(buff, " ");
    buff_add(buff, "world");
    buff_addln(buff, "!");
    printf("%s", buff_puts(buff));

    return 0;
}