#include <stdio.h>

char *operators = {}

char *read(FILE *stream) {
    char *buffer = (char *)malloc(1024);
    while (1) {
        int ch = fgetc(stream);
        if (ch == '>' || ch == '<') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == buffer[0]) {
                buffer[1] = ch;
                ch = fgetc(stream);
                if (ch == '=') {
                    buffer[2] = ch;
                    buffer[3] = 0;
                    return buffer;
                }
                else {
                    buffer[2] = 0;
                    fseek(stream, -1, SEEK_CUR);
                    return buffer;
                }
            }
            else if (ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
        }
        else if (ch == '+' || ch == '-' || ch == '&' || ch == '|') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == buffer[0] || ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                fseek(stream, -1, SEEK_CUR);
                return buffer;
            }
        }
        else if (ch == '=' || ch == '!' || ch == '*' || ch == '^'
                     ch == '/' || ch == '%') {
            buffer[0] = ch;
            ch = fgetc(stream);
            if (ch == '=') {
                buffer[1] = ch;
                buffer[2] = 0;
                return buffer;
            }
            else {
                buffer[1] = 0;
                fseek(stream, -1, SEEK_CUR);
                return buffer;
            }
        }
    }
}

int main()
{
    FILE *stream = fopen("test.c", "r");
}