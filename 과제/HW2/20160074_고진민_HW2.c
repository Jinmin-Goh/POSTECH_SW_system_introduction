// 20160074 화학공학과 고진민
// HW2 source code


// 4.
/* Enumerated type creates set of constants numbered 0 and upward */
typedef enum {MODE_A, MODE_B, MODE_C, MODE_D, MODE_E} mode_t;

long switch3(long *p1, long *p2, mode_t action)
{	
	long result = 0;
	switch(action) {
		case MODE_A:
		result = *p2;
		*p2 = *p1;
		break;

		case MODE_B:
		result = *p1;
		result = result + *p2;
		*p1 = result;
		break;

		case MODE_C:
		*p1 = 59;
		result = *p2;
		break;

		case MODE_D:
		*p1 = *p2;
		result = 27;
		break;

		case MODE_E:
		result = 27;
		break;

		default:
		result = 12;
		break;
	}
	return result;
}

// 5. 

long switch_prob(long x, long n) {
	long result = x;
	n = n - 0x3c;
	switch(n) {
		case 0:
		case 2:
		result = 8 * x;
		break;

		case 3:
		result = x;
		result = result >> 3;
		break;

		case 4:
		result = x;
		result = result << 4;
		result = result - x;

		case 5:
		result = result * result;

		default:	// case 1 is same with default, so we can erase it
		result = x + 0x4b;
		break;
	}
	return result;
}