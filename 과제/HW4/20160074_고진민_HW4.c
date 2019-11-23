// 20160074 화학공학과 고진민
// HW4

// 1. (5.17)
void *memset(void *s, int c, size_t n)
{
	size_t cnt = 0;
	unsigned long temp = (c || (c << 8) || (c << 16) || (c << 24));
	unsigned long *sint = s;
	int k = sizeof(unsigned long)
	while (cnt < (n / k))
	{
		*sint++ = (unsigned long) temp;
		cnt++;
	}
	cnt = 0;
	while (cnt < (n % k))
	{
		(unsigned char) *sint++ = (unsigned char) c;
		cnt++;
	}
	return s;
}