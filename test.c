int f(int x, int y, int z)
{
    static int a = 32423;
	static int b;
	static int c[4] = {};
	int e = 3720;
	static int d[3][2] = { {88, 77}, {66, 55}, {44, 33} };
	char *xxx;
	xxx++;
	char yyy[3];
	yyy++;
	f(3, 4, 5);

	z = a * b * e + a;
	z = a * b + d;
	if (3 > 2)
		jimmy: e = 432;
	else
		b = e * 2;
	switch (a) {
		case 3: b = 44444;
		case 4: b = 55555;
		default: b = 66666;
	}

	for (z = 0; z < 10; z = z + 1)
		a = a + 999;
	while (1) {
		while (2) {
			if (3 > 2)
				return 2048;
		}
		return 1024;
	}
	int i;
	for (i = 0; i < 100; i++) {
		if (i == 10)
			break;
	}
	goto jimmy;
	return 33333333;
}

int g(int x, int y, int z)
{
    static int a = 32423;
	static int b;
	static int c[4] = {};
	int e = 3720;
	static int d[3][2] = { {88, 77}, {66, 55}, {44, 33} };
	char *xxx;
	xxx++;
	char yyy[3];
	yyy++;
	f(3, 4, 5);

	z = a * b * e + a;
	z = a * b + d;
	if (3 > 2)
		jimmy: e = 432;
	else
		b = e * 2;
	switch (a) {
		case 3: b = 44444;
		case 4: b = 55555;
		default: b = 66666;
	}

	for (z = 0; z < 10; z = z + 1)
		a = a + 999;
	while (1) {
		while (2) {
			if (3 > 2)
				return 2048;
		}
		return 1024;
	}
	int i;
	for (i = 0; i < 100; i++) {
		if (i == 10)
			break;
	}
	goto jimmy;
	return 33333333;
}

int main()
{
	f();
	return 0;
}