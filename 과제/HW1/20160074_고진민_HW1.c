// 1. (2.59)
int combination(int x, int y)
{
	int mask_x, mask_y;
	mask_x = x & 0xFF;
	mask_y = y & 0xFFFFFF00;
	return mask_x | mask_y;
}

// 2. (2.65)
int odd_ones(unsigned x)
{
	x ^= (x >> 16);
	x ^= (x >> 8);
	x ^= (x >> 4);
	x ^= (x >> 2);
	x ^= (x >> 1);
	return x & 1;
}

// 3. (2.69)
unsigned rotate_left(unsigned x, int n)
{
	int part1, part2, mask;
	part1 = x << n;
	mask = ((1 << 31) >> (n - 1));
	mask = ~mask;
	part2 = (x | mask) >> (32 - n);
	return part1 | part2;
}

// 4. (2.74)
int tsub_ok(int x, int y)
{
	int minusY, sub;
	minusY = (~y) + 1;
	sub = x - y;
	return !(!((x >> 31) ^ (minusY >> 31))) & ((x >> 31) ^ (sub >> 31));
}

// 5. (2.80)
int threefourths(int x)
{
	int result, check1, check2, check3;
	result = (x >> 1) + (x >> 2);
	check1 = !(!(x >> 31)) & !(!((~x + 1) & 3));
	check2 = (x >> 31) & (((~x + 1) & 3) == 1);
	check3 = (!(x >> 31)) & ((x & 3) == 3);
	return result + check1 + check2 + check3;
}

// 7. (2.95)
float_bits float_half(float_bits f)
{
	unsigned sign, exp, frac;
	sign = (f >> 31) & 0x80000000;
	exp = (f >> 23) & 0xFF;
	frac = f & 0x7FFFFF;
	if (exp == 0xFF)
		return f;
	else if (exp == 0)
		return sign | (exp << 23) | (frac >> 1);
	else
		return sign | ((exp - 1) << 23) | frac;
}

// 8. (2.97)
float_bits float_i2f(int x) {
  unsigned sign, exp, frac, i, flag = 0, temp, mask_frac, mask_sign, check1, check2, check3, G, R, S;

  mask_sign = 0x80000000;
  mask_frac = 0x7FFFFF;

  // special cases. we can't make -2^31 positive in int form.
  if (x == 0)
    return 0;
  if (x == mask_sign)
    return 0xCF000000;

  if (x < 0)
  {
    sign = 1;
    i = -x;
  }
  else
  {
    sign = 0;
    i = x;
  }

  // finding E
  temp = i;
  while (temp)
  {
    temp = temp >> 1;
    flag++;
  }
  exp = 126 + flag;
  if (flag < 24)
    frac = (i << (24 - flag)) & mask_frac;
  // rounding process	
  else
  {
    check1 = flag - 24;
    check2 = check1 - 1;
    G = i & (1 << check1);
    R = i & (1 << check2);
    S = i & ((1 << check2) - 1);
    check3 = R && (G || S);

    frac = (i >> (flag - 24)) & mask_frac;
    frac = frac + check3;
  }
  return (sign << 31) + (exp << 23) + frac;
}

// 10.

float_bits float_multiply(float_bits x, float_bits y)
{
	unsigned sign_x, sign_y, exp_x, exp_y, frac_x, frac_y, flag, temp;
	unsigned sign, exp, frac;	// total result
	sign_x = (x >> 31) & 1;
	sign_y = (y >> 31) & 1;
	exp_x = (x >> 23) & 0xFF;
	exp_y = (y >> 23) & 0xFF;
	frac_x = x & 0x7FFFFF;
	frac_y = y & 0x7FFFFF;
	sign = sign_x ^ sign_y;
	exp = exp_x + exp_y - 127;
	if (exp > 254)
	{
		exp = 0xFF << 23;
		frac = 0;
		return (sign << 31) | exp | frac;
	}

	frac_x = (frac_x + 0x800000) >> 9;
	frac_y = (frac_y + 0x800000) >> 9;
	frac = frac_x * frac_y;
	if (frac >= 0x20000000)
	{
		frac = frac >> 7;
		exp++;
	}
	else 
		frac = frac >> 6;
	return (sign << 31) | (exp << 23) | frac
}
