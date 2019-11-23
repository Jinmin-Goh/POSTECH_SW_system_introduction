/*
* bitOr – x|y using only ~ and &
* Example: bitOr(6, 3) = 7
* Legal ops: ~ &
*/
int bitOr(int x, int y) {
	int temp;
	temp = ~(~x & ~y);
	return temp;
}

/*
* addOK - Determine if can compute x+y without overflow
* Example: addOK(0x80000000,0x80000000) = 0,
* addOK(0x80000000,0x70000000) = 1,
* Legal ops: ! ~ & ^ | + << >>
*/
int addOK(int x, int y){
	int check, sum;
	sum = x + y;
	check = sum >> 31;
	return !(!((x >> 31) ^ (y >> 31)) & ((x >> 31) ^ check));
}

/*
* negate – return -x
* Example: negate(1) = -1.
* Legal ops: ! ~ & ^ | + << >>
*/
int negate(int x) {
	return ~x + 1;
}

/*
* logicalShift - shift x to the right by n, using a logical shift
* Can assume that 0 <= n <= 31
* Examples: logicalShift(0x87654321,4) = 0x08765432
* Legal ops: ~ & ^ | + << >>
*/
int logicalShift(int x, int n) {
	int check;
	check = ((0x01 << 31) >> n) << 1;
	return ~check & (x >> n);
}

/*
* bitCount - returns count of number of 1's in word
* Examples: bitCount(5) = 2, bitCount(7) = 3
* Legal ops: ! ~ & ^ | + << >>
* Max ops: 40
*/
int bitCount(int x) {
	int sum, check1, check2, check3;
	check1 = 0x11 | (0x11 << 8);
	check2 = check1 | (check1 << 16);
	sum = (x & check2) + ((x >> 1) & check2) + ((x >> 2) & check2) + ((x >> 3) & check2);
	sum = sum + (sum >> 16);
	check3 = 0x0F | (0x0F << 8);
	sum = (sum & check3) + ((sum >> 4) & check3);
	sum = sum + (sum >> 8);
	return sum & 0x3F;
}