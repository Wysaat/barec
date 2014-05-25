int main()
{
	int b;
	int *a = &b;
	b = ++*a;
	b = 9 * 4324;
	int c;
	char *d = "Hello";
	d = (char *)43424;
	if (a > 2) {
		d = 5;
		*a = 4;
	} else {
		a = 7;
		b = 43242;
	}
}