int f(int x, int y, int z)
{
    static int a = 32423;
	static int b;
	static int c[4] = {};
	int e = 3720;
	static int d[3][2] = { {88, 77}, {66, 55}, {44, 33} };

	// z = a * b * e + a;
	z = a * b + d;
	if (3 > 2)
		e = 432;
	else
		b = e * 2;
	switch (a) {
		case 3: b = 44444;
		case 4: b = 55555;
		default: b = 66666;
	}
}
