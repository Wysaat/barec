#include <stdio.h>
#include <stdarg.h>

void
foo(char *fmt, ...)
{
	va_list ap;
	int d;
	char c, *s;

	va_start(ap, fmt);
	while (*fmt) {
		switch (*fmt++) {
			case 's':
			    s = va_arg(ap, char *);
			    printf("string %s\n", s);
			    break;
			case 'd':
			    d = va_arg(ap, int);
			    printf("int %d\n", d);
			    break;
		}
	}
	va_end(ap);
}

int
main()
{
	foo("hello s d s s", "world", 33333, "tom", "en");
	return 0;
}