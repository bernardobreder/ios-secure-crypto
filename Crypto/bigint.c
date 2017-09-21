#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "bigint.h"

static const fp_digit primes[256] = { 0x0002, 0x0003, 0x0005, 0x0007, 0x000B, 0x000D, 0x0011, 0x0013, 0x0017, 0x001D, 0x001F, 0x0025, 0x0029, 0x002B, 0x002F, 0x0035, 0x003B, 0x003D, 0x0043, 0x0047, 0x0049, 0x004F, 0x0053, 0x0059, 0x0061, 0x0065, 0x0067, 0x006B, 0x006D, 0x0071, 0x007F, 0x0083, 0x0089, 0x008B, 0x0095, 0x0097, 0x009D, 0x00A3, 0x00A7, 0x00AD, 0x00B3, 0x00B5, 0x00BF, 0x00C1, 0x00C5, 0x00C7, 0x00D3, 0x00DF, 0x00E3, 0x00E5, 0x00E9, 0x00EF, 0x00F1, 0x00FB, 0x0101, 0x0107, 0x010D,
		0x010F, 0x0115, 0x0119, 0x011B, 0x0125, 0x0133, 0x0137, 0x0139, 0x013D, 0x014B, 0x0151, 0x015B, 0x015D, 0x0161, 0x0167, 0x016F, 0x0175, 0x017B, 0x017F, 0x0185, 0x018D, 0x0191, 0x0199, 0x01A3, 0x01A5, 0x01AF, 0x01B1, 0x01B7, 0x01BB, 0x01C1, 0x01C9, 0x01CD, 0x01CF, 0x01D3, 0x01DF, 0x01E7, 0x01EB, 0x01F3, 0x01F7, 0x01FD, 0x0209, 0x020B, 0x021D, 0x0223, 0x022D, 0x0233, 0x0239, 0x023B, 0x0241, 0x024B, 0x0251, 0x0257, 0x0259, 0x025F, 0x0265, 0x0269, 0x026B, 0x0277, 0x0281, 0x0283, 0x0287,
		0x028D, 0x0293, 0x0295, 0x02A1, 0x02A5, 0x02AB, 0x02B3, 0x02BD, 0x02C5, 0x02CF, 0x02D7, 0x02DD, 0x02E3, 0x02E7, 0x02EF, 0x02F5, 0x02F9, 0x0301, 0x0305, 0x0313, 0x031D, 0x0329, 0x032B, 0x0335, 0x0337, 0x033B, 0x033D, 0x0347, 0x0355, 0x0359, 0x035B, 0x035F, 0x036D, 0x0371, 0x0373, 0x0377, 0x038B, 0x038F, 0x0397, 0x03A1, 0x03A9, 0x03AD, 0x03B3, 0x03B9, 0x03C7, 0x03CB, 0x03D1, 0x03D7, 0x03DF, 0x03E5, 0x03F1, 0x03F5, 0x03FB, 0x03FD, 0x0407, 0x0409, 0x040F, 0x0419, 0x041B, 0x0425, 0x0427,
		0x042D, 0x043F, 0x0443, 0x0445, 0x0449, 0x044F, 0x0455, 0x045D, 0x0463, 0x0469, 0x047F, 0x0481, 0x048B, 0x0493, 0x049D, 0x04A3, 0x04A9, 0x04B1, 0x04BD, 0x04C1, 0x04C7, 0x04CD, 0x04CF, 0x04D5, 0x04E1, 0x04EB, 0x04FD, 0x04FF, 0x0503, 0x0509, 0x050B, 0x0511, 0x0515, 0x0517, 0x051B, 0x0527, 0x0529, 0x052F, 0x0551, 0x0557, 0x055D, 0x0565, 0x0577, 0x0581, 0x058F, 0x0593, 0x0595, 0x0599, 0x059F, 0x05A7, 0x05AB, 0x05AD, 0x05B3, 0x05BF, 0x05C9, 0x05CB, 0x05CF, 0x05D1, 0x05D5, 0x05DB, 0x05E7,
		0x05F3, 0x05FB, 0x0607, 0x060D, 0x0611, 0x0617, 0x061F, 0x0623, 0x062B, 0x062F, 0x063D, 0x0641, 0x0647, 0x0649, 0x064D, 0x0653 };

#define CLEAR_CARRY \
   c0 = c1 = c2 = 0;

#define COMBA_STORE(x) \
   x = c0;

#define COMBA_STORE2(x) \
   x = c1;

#define CARRY_FORWARD \
   do { c0 = c1; c1 = c2; c2 = 0; } while (0);

/* clear the chaining variables */
#define COMBA_CLEAR \
   c0 = c1 = c2 = 0;

/* forward the carry to the next digit */
#define COMBA_FORWARD \
   do { c0 = c1; c1 = c2; c2 = 0; } while (0);

/* store the first sum */
#define COMBA_STORE(x) \
   x = c0;

/* store the second sum [carry] */
#define COMBA_STORE2(x) \
   x = c1;

/* multiplies point i and j, updates carry "c1" and digit c2 */
#define SQRADD(i, j)                                 \
   do { fp_word t;                                   \
   t = c0 + ((fp_word)i) * ((fp_word)j);  c0 = t;    \
   t = c1 + (t >> DIGIT_BIT);             c1 = t; c2 += t >> DIGIT_BIT; \
   } while (0);

/* for squaring some of the terms are doubled... */
#define SQRADD2(i, j)                                                 \
   do { fp_word t,tt;                                                    \
   t  = ((fp_word)i) * ((fp_word)j);                                  \
   tt = (fp_word)c0 + t;                 c0 = tt;                              \
   tt = (fp_word)c1 + (tt >> DIGIT_BIT); c1 = tt; c2 += tt >> DIGIT_BIT;       \
   tt = (fp_word)c0 + t;                 c0 = tt;                              \
   tt = (fp_word)c1 + (tt >> DIGIT_BIT); c1 = tt; c2 += tt >> DIGIT_BIT;       \
   } while (0);

#define SQRADDSC(i, j)                                                         \
   do { fp_word t;                                                             \
      t =  ((fp_word)i) * ((fp_word)j);                                        \
      sc0 = (fp_digit)t; sc1 = (t >> DIGIT_BIT); sc2 = 0;                      \
   } while (0);

#define SQRADDAC(i, j)                                                         \
   do { fp_word t;                                                             \
   t = sc0 + ((fp_word)i) * ((fp_word)j);  sc0 = t;                            \
   t = sc1 + (t >> DIGIT_BIT);             sc1 = t; sc2 += t >> DIGIT_BIT;     \
   } while (0);

#define MULADD(i, j)                                                              \
   do { fp_word t;                                                                \
   t = (fp_word)c0 + ((fp_word)i) * ((fp_word)j); c0 = t;                         \
   t = (fp_word)c1 + (t >> DIGIT_BIT);            c1 = t; c2 += t >> DIGIT_BIT;   \
   } while (0);

#define SQRADDDB                                                               \
   do { fp_word t;                                                             \
   t = ((fp_word)sc0) + ((fp_word)sc0) + c0; c0 = t;                                                 \
   t = ((fp_word)sc1) + ((fp_word)sc1) + c1 + (t >> DIGIT_BIT); c1 = t;                              \
   c2 = c2 + ((fp_word)sc2) + ((fp_word)sc2) + (t >> DIGIT_BIT);                                     \
   } while (0);
/* generic comba squarer */
static const int lnz[16] = { 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0 };

#define PROPCARRY \
   do { fp_digit t = _c[0] += cy; cy = (t < cy); } while (0)

int fp_cmp_mag(struct fp_int *a, struct fp_int *b) {
	int x;
	if (a->used > b->used) {
		return FP_GT;
	} else if (a->used < b->used) {
		return FP_LT;
	} else {
		for (x = a->used - 1; x >= 0; x--) {
			if (a->dp[x] > b->dp[x]) {
				return FP_GT;
			} else if (a->dp[x] < b->dp[x]) {
				return FP_LT;
			}
		}
	}
	return FP_EQ;
}

/* compare against a single digit */
int fp_cmp_d(struct fp_int *a, fp_digit b) {
	/* compare based on sign */
	if ((b && a->used == 0) || a->sign == FP_NEG) {
		return FP_LT;
	}

	/* compare based on magnitude */
	if (a->used > 1) {
		return FP_GT;
	}

	/* compare the only digit of a to b */
	if (a->dp[0] > b) {
		return FP_GT;
	} else if (a->dp[0] < b) {
		return FP_LT;
	} else {
		return FP_EQ;
	}

}

int fp_cmp(struct fp_int *a, struct fp_int *b) {
	if (a->sign == FP_NEG && b->sign == FP_ZPOS) {
		return FP_LT;
	} else if (a->sign == FP_ZPOS && b->sign == FP_NEG) {
		return FP_GT;
	} else {
		/* compare digits */
		if (a->sign == FP_NEG) {
			/* if negative compare opposite direction */
			return fp_cmp_mag(b, a);
		} else {
			return fp_cmp_mag(a, b);
		}
	}
}

/* c = a + b */
void fp_add(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	int sa, sb;
	sa = a->sign;
	sb = b->sign;
	if (sa == sb) {
		c->sign = sa;
		s_fp_add(a, b, c);
	} else {
		if (fp_cmp_mag(a, b) == FP_LT) {
			c->sign = sb;
			s_fp_sub(b, a, c);
		} else {
			c->sign = sa;
			s_fp_sub(a, b, c);
		}
	}
}

/* unsigned addition */
void s_fp_add(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	int x, y, oldused;
	register fp_word t;

	y = MAX(a->used, b->used);
	oldused = c->used;
	c->used = y;

	t = 0;
	for (x = 0; x < y; x++) {
		t += ((fp_word) a->dp[x]) + ((fp_word) b->dp[x]);
		c->dp[x] = (fp_digit) t;
		t >>= DIGIT_BIT;
	}
	if (t != 0 && x < FP_SIZE) {
		c->dp[c->used++] = (fp_digit) t;
		++x;
	}

	c->used = x;
	for (; x < oldused; x++) {
		c->dp[x] = 0;
	}
	while ((c)->used && (c)->dp[(c)->used - 1] == 0) {
		--((c)->used);
	}
	(c)->sign = (c)->used ? (c)->sign : 0;
}

/* c = a + b */
void fp_add_d(struct fp_int *a, fp_digit b, struct fp_int *c) {
	struct fp_int tmp;
	fp_set(&tmp, b);
	fp_add(a, &tmp, c);
}

/* d = a + b (mod c) */
int fp_addmod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d) {
	struct fp_int tmp;
	fp_zero(&tmp);
	fp_add(a, b, &tmp);
	return fp_mod(&tmp, c, d);
}

/* c = a - b */
void fp_sub(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	int sa, sb;

	sa = a->sign;
	sb = b->sign;

	if (sa != sb) {
		/* subtract a negative from a positive, OR */
		/* subtract a positive from a negative. */
		/* In either case, ADD their magnitudes, */
		/* and use the sign of the first number. */
		c->sign = sa;
		s_fp_add(a, b, c);
	} else {
		/* subtract a positive from a positive, OR */
		/* subtract a negative from a negative. */
		/* First, take the difference between their */
		/* magnitudes, then... */
		if (fp_cmp_mag(a, b) != FP_LT) {
			/* Copy the sign from the first */
			c->sign = sa;
			/* The first has a larger or equal magnitude */
			s_fp_sub(a, b, c);
		} else {
			/* The result has the *opposite* sign from */
			/* the first number. */
			c->sign = (sa == FP_ZPOS) ? FP_NEG : FP_ZPOS;
			/* The second has a larger magnitude */
			s_fp_sub(b, a, c);
		}
	}
}

/* d = a - b (mod c) */
int fp_submod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d) {
	struct fp_int tmp;
	fp_zero(&tmp);
	fp_sub(a, b, &tmp);
	return fp_mod(&tmp, c, d);
}

/* c = a - b */
void fp_sub_d(struct fp_int *a, fp_digit b, struct fp_int *c) {
	struct fp_int tmp;
	fp_set(&tmp, b);
	fp_sub(a, &tmp, c);
}

/* unsigned subtraction ||a|| >= ||b|| ALWAYS! */
void s_fp_sub(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	int x, oldbused, oldused;
	fp_word t = 0;
	oldused = c->used;
	oldbused = b->used;
	c->used = a->used;
	for (x = 0; x < oldbused; x++) {
		t = ((fp_word) a->dp[x]) - (((fp_word) b->dp[x]) + t);
		c->dp[x] = (fp_digit) t;
		t = (t >> DIGIT_BIT) & 1;
	}
	for (; x < a->used; x++) {
		t = ((fp_word) a->dp[x]) - t;
		c->dp[x] = (fp_digit) t;
		t = (t >> DIGIT_BIT);
	}
	for (; x < oldused; x++) {
		c->dp[x] = 0;
	}
	fp_clamp(c);
}

/* c = a mod b, 0 <= c < b  */
int fp_mod(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	struct fp_int t;
	int err;
	fp_zero(&t);
	if ((err = fp_div(a, b, NULL, &t)) != FP_OKAY) {
		return err;
	}
	if (t.sign != b->sign) {
		fp_add(&t, b, c);
	} else {
		fp_copy(&t, c);
	}
	return FP_OKAY;
}

void fp_set(struct fp_int *a, fp_digit b) {
	fp_zero(a);
	a->dp[0] = b;
	a->used = a->dp[0] ? 1 : 0;
}

/* a/b => cb + d == a */
int fp_div(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d) {
	struct fp_int q, x, y, t1, t2;
	int n, t, i, norm, neg;
	/* is divisor zero ? */
	if (fp_iszero (b) == 1) {
		return FP_VAL;
	}
	/* if a < b then q=0, r = a */
	if (fp_cmp_mag(a, b) == FP_LT) {
		if (d != NULL) {
			fp_copy(a, d);
		}
		if (c != NULL) {
			fp_zero(c);
		}
		return FP_OKAY;
	}
	fp_init(&q);
	q.used = a->used + 2;
	fp_init(&t1);
	fp_init(&t2);
	fp_init_copy(&x, a);
	fp_init_copy(&y, b);
	/* fix the sign */
	neg = (a->sign == b->sign) ? FP_ZPOS : FP_NEG;
	x.sign = y.sign = FP_ZPOS;
	/* normalize both x and y, ensure that y >= b/2, [b == 2**DIGIT_BIT] */
	norm = fp_count_bits(&y) % DIGIT_BIT;
	if (norm < (int) (DIGIT_BIT - 1)) {
		norm = (DIGIT_BIT - 1) - norm;
		fp_mul_2d(&x, norm, &x);
		fp_mul_2d(&y, norm, &y);
	} else {
		norm = 0;
	}
	/* note hac does 0 based, so if used==5 then its 0,1,2,3,4, e.g. use 4 */
	n = x.used - 1;
	t = y.used - 1;
	/* while (x >= y*b**n-t) do { q[n-t] += 1; x -= y*b**{n-t} } */
	fp_lshd(&y, n - t); /* y = y*b**{n-t} */
	while (fp_cmp(&x, &y) != FP_LT) {
		++(q.dp[n - t]);
		fp_sub(&x, &y, &x);
	}
	/* reset y by shifting it back down */
	fp_rshd(&y, n - t);
	/* step 3. for i from n down to (t + 1) */
	for (i = n; i >= (t + 1); i--) {
		if (i > x.used) {
			continue;
		}
		/* step 3.1 if xi == yt then set q{i-t-1} to b-1,
		 * otherwise set q{i-t-1} to (xi*b + x{i-1})/yt */
		if (x.dp[i] == y.dp[t]) {
			q.dp[i - t - 1] = ((((fp_word) 1) << DIGIT_BIT) - 1);
		} else {
			fp_word tmp;
			tmp = ((fp_word) x.dp[i]) << ((fp_word) DIGIT_BIT);
			tmp |= ((fp_word) x.dp[i - 1]);
			tmp /= ((fp_word) y.dp[t]);
			q.dp[i - t - 1] = (fp_digit) (tmp);
		}
		/* while (q{i-t-1} * (yt * b + y{t-1})) >
		 xi * b**2 + xi-1 * b + xi-2
		 do q{i-t-1} -= 1;
		 */
		q.dp[i - t - 1] = (q.dp[i - t - 1] + 1);
		do {
			q.dp[i - t - 1] = (q.dp[i - t - 1] - 1);
			/* find left hand */
			fp_zero(&t1);
			t1.dp[0] = (t - 1 < 0) ? 0 : y.dp[t - 1];
			t1.dp[1] = y.dp[t];
			t1.used = 2;
			fp_mul_d(&t1, q.dp[i - t - 1], &t1);

			/* find right hand */
			t2.dp[0] = (i - 2 < 0) ? 0 : x.dp[i - 2];
			t2.dp[1] = (i - 1 < 0) ? 0 : x.dp[i - 1];
			t2.dp[2] = x.dp[i];
			t2.used = 3;
		} while (fp_cmp_mag(&t1, &t2) == FP_GT);
		/* step 3.3 x = x - q{i-t-1} * y * b**{i-t-1} */
		fp_mul_d(&y, q.dp[i - t - 1], &t1);
		fp_lshd(&t1, i - t - 1);
		fp_sub(&x, &t1, &x);
		/* if x < 0 then { x = x + y*b**{i-t-1}; q{i-t-1} -= 1; } */
		if (x.sign == FP_NEG) {
			fp_copy(&y, &t1);
			fp_lshd(&t1, i - t - 1);
			fp_add(&x, &t1, &x);
			q.dp[i - t - 1] = q.dp[i - t - 1] - 1;
		}
	}
	/* now q is the quotient and x is the remainder
	 * [which we have to normalize]
	 */
	/* get sign before writing to c */
	x.sign = x.used == 0 ? FP_ZPOS : a->sign;
	if (c != NULL) {
		fp_clamp(&q);
		fp_copy(&q, c);
		c->sign = neg;
	}
	if (d != NULL) {
		fp_div_2d(&x, norm, &x, NULL);
		/* the following is a kludge, essentially we were seeing the right remainder but
		 with excess digits that should have been zero
		 */
		for (i = b->used; i < x.used; i++) {
			x.dp[i] = 0;
		}
		fp_clamp(&x);
		fp_copy(&x, d);
	}
	return FP_OKAY;
}

/* c = a / 2**b */
void fp_div_2d(struct fp_int *a, int b, struct fp_int *c, struct fp_int *d) {
	fp_digit D, r, rr;
	int x;
	struct fp_int t;

	/* if the shift count is <= 0 then we do no work */
	if (b <= 0) {
		fp_copy(a, c);
		if (d != NULL) {
			fp_zero(d);
		}
		return;
	}

	fp_init(&t);

	/* get the remainder */
	if (d != NULL) {
		fp_mod_2d(a, b, &t);
	}

	/* copy */
	fp_copy(a, c);

	/* shift by as many digits in the bit count */
	if (b >= (int) DIGIT_BIT) {
		fp_rshd(c, b / DIGIT_BIT);
	}

	/* shift any bit count < DIGIT_BIT */
	D = (fp_digit) (b % DIGIT_BIT);
	if (D != 0) {
		register fp_digit *tmpc, mask, shift;

		/* mask */
		mask = (((fp_digit) 1) << D) - 1;

		/* shift for lsb */
		shift = DIGIT_BIT - D;

		/* alias */
		tmpc = c->dp + (c->used - 1);

		/* carry */
		r = 0;
		for (x = c->used - 1; x >= 0; x--) {
			/* get the lower  bits of this word in a temp */
			rr = *tmpc & mask;

			/* shift the current word and mix in the carry bits from the previous word */
			*tmpc = (*tmpc >> D) | (r << shift);
			--tmpc;

			/* set the carry to the carry bits of the current word found above */
			r = rr;
		}
	}
	fp_clamp(c);
	if (d != NULL) {
		fp_copy(&t, d);
	}
}

int fp_count_bits(struct fp_int * a) {
	int r;
	fp_digit q;

	/* shortcut */
	if (a->used == 0) {
		return 0;
	}

	/* get number of digits and add that */
	r = (a->used - 1) * DIGIT_BIT;

	/* take the last digit and count the bits in it */
	q = a->dp[a->used - 1];
	while (q > ((fp_digit) 0)) {
		++r;
		q >>= ((fp_digit) 1);
	}
	return r;
}

void fp_lshd(struct fp_int *a, int x) {
	int y;

	/* move up and truncate as required */
	y = MIN(a->used + x - 1, (int)(FP_SIZE-1));

	/* store new size */
	a->used = y + 1;

	/* move digits */
	for (; y >= x; y--) {
		a->dp[y] = a->dp[y - x];
	}

	/* zero lower digits */
	for (; y >= 0; y--) {
		a->dp[y] = 0;
	}

	/* clamp digits */
	fp_clamp(a);
}

/* c = a * 2**d */
void fp_mul_2d(struct fp_int *a, int b, struct fp_int *c) {
	fp_digit carry, carrytmp, shift;
	int x;

	/* copy it */
	fp_copy(a, c);

	/* handle whole digits */
	if (b >= DIGIT_BIT) {
		fp_lshd(c, b / DIGIT_BIT);
	}
	b %= DIGIT_BIT;

	/* shift the digits */
	if (b != 0) {
		carry = 0;
		shift = DIGIT_BIT - b;
		for (x = 0; x < c->used; x++) {
			carrytmp = c->dp[x] >> shift;
			c->dp[x] = (c->dp[x] << b) + carry;
			carry = carrytmp;
		}
		/* store last carry if room */
		if (carry && x < FP_SIZE) {
			c->dp[c->used++] = carry;
		}
	}
	fp_clamp(c);
}

/* c = a * b */
void fp_mul_d(struct fp_int *a, fp_digit b, struct fp_int *c) {
	fp_word w;
	int x, oldused;

	oldused = c->used;
	c->used = a->used;
	c->sign = a->sign;
	w = 0;
	for (x = 0; x < a->used; x++) {
		w = ((fp_word) a->dp[x]) * ((fp_word) b) + w;
		c->dp[x] = (fp_digit) w;
		w = w >> DIGIT_BIT;
	}
	if (w != 0 && (a->used != FP_SIZE)) {
		c->dp[c->used++] = w;
		++x;
	}
	for (; x < oldused; x++) {
		c->dp[x] = 0;
	}
	fp_clamp(c);
}

void fp_rshd(struct fp_int *a, int x) {
	int y;

	/* too many digits just zero and return */
	if (x >= a->used) {
		fp_zero(a);
		return;
	}

	/* shift */
	for (y = 0; y < a->used - x; y++) {
		a->dp[y] = a->dp[y + x];
	}

	/* zero rest */
	for (; y < a->used; y++) {
		a->dp[y] = 0;
	}

	/* decrement count */
	a->used -= x;
	fp_clamp(a);
}

/* c = a mod 2**d */
void fp_mod_2d(struct fp_int *a, int b, struct fp_int *c) {
	int x;

	/* zero if count less than or equal to zero */
	if (b <= 0) {
		fp_zero(c);
		return;
	}

	/* get copy of input */
	fp_copy(a, c);

	/* if 2**d is larger than we just return */
	if (b >= (DIGIT_BIT * a->used)) {
		return;
	}

	/* zero digits above the last digit of the modulus */
	for (x = (b / DIGIT_BIT) + ((b % DIGIT_BIT) == 0 ? 0 : 1); x < c->used; x++) {
		c->dp[x] = 0;
	}
	/* clear the digit that is not completely outside/inside the modulus */
	c->dp[b / DIGIT_BIT] &= ~((fp_digit) 0) >> (DIGIT_BIT - b);
	fp_clamp(c);
}

/* c = (a, b) */
void fp_gcd(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	struct fp_int u, v, r;

	/* either zero than gcd is the largest */
	if (fp_iszero (a) == 1 && fp_iszero (b) == 0) {
		fp_abs(b, c);
		return;
	}
	if (fp_iszero (a) == 0 && fp_iszero (b) == 1) {
		fp_abs(a, c);
		return;
	}

	/* optimized.  At this point if a == 0 then
	 * b must equal zero too
	 */
	if (fp_iszero (a) == 1) {
		fp_zero(c);
		return;
	}

	/* sort inputs */
	if (fp_cmp_mag(a, b) != FP_LT) {
		fp_init_copy(&u, a);
		fp_init_copy(&v, b);
	} else {
		fp_init_copy(&u, b);
		fp_init_copy(&v, a);
	}

	fp_zero(&r);
	while (fp_iszero(&v) == FP_NO) {
		fp_mod(&u, &v, &r);
		fp_copy(&v, &u);
		fp_copy(&r, &v);
	}
	fp_copy(&u, c);
}

/* a few primes */
int fp_isprime(struct fp_int *a) {
	struct fp_int b;
	fp_digit d;
	int r, res;

	/* do trial division */
	for (r = 0; r < 256; r++) {
		fp_mod_d(a, primes[r], &d);
		if (d == 0) {
			return FP_NO;
		}
	}

	/* now do 8 miller rabins */
	fp_init(&b);
	for (r = 0; r < 8; r++) {
		fp_set(&b, primes[r]);
		fp_prime_miller_rabin(a, &b, &res);
		if (res == FP_NO) {
			return FP_NO;
		}
	}
	return FP_YES;
}

static int fp_invmod_slow(struct fp_int * a, struct fp_int * b, struct fp_int * c) {
	struct fp_int x, y, u, v, A, B, C, D;
	int res;

	/* b cannot be negative */
	if (b->sign == FP_NEG || fp_iszero(b) == 1) {
		return FP_VAL;
	}

	/* init temps */
	fp_init(&x);
	fp_init(&y);
	fp_init(&u);
	fp_init(&v);
	fp_init(&A);
	fp_init(&B);
	fp_init(&C);
	fp_init(&D);

	/* x = a, y = b */
	if ((res = fp_mod(a, b, &x)) != FP_OKAY) {
		return res;
	}
	fp_copy(b, &y);

	/* 2. [modified] if x,y are both even then return an error! */
	if (fp_iseven (&x) == 1 && fp_iseven (&y) == 1) {
		return FP_VAL;
	}

	/* 3. u=x, v=y, A=1, B=0, C=0,D=1 */
	fp_copy(&x, &u);
	fp_copy(&y, &v);
	fp_set(&A, 1);
	fp_set(&D, 1);

	top:
	/* 4.  while u is even do */
	while (fp_iseven (&u) == 1) {
		/* 4.1 u = u/2 */
		fp_div_2(&u, &u);

		/* 4.2 if A or B is odd then */
		if (fp_isodd (&A) == 1 || fp_isodd (&B) == 1) {
			/* A = (A+y)/2, B = (B-x)/2 */
			fp_add(&A, &y, &A);
			fp_sub(&B, &x, &B);
		}
		/* A = A/2, B = B/2 */
		fp_div_2(&A, &A);
		fp_div_2(&B, &B);
	}

	/* 5.  while v is even do */
	while (fp_iseven (&v) == 1) {
		/* 5.1 v = v/2 */
		fp_div_2(&v, &v);

		/* 5.2 if C or D is odd then */
		if (fp_isodd (&C) == 1 || fp_isodd (&D) == 1) {
			/* C = (C+y)/2, D = (D-x)/2 */
			fp_add(&C, &y, &C);
			fp_sub(&D, &x, &D);
		}
		/* C = C/2, D = D/2 */
		fp_div_2(&C, &C);
		fp_div_2(&D, &D);
	}

	/* 6.  if u >= v then */
	if (fp_cmp(&u, &v) != FP_LT) {
		/* u = u - v, A = A - C, B = B - D */
		fp_sub(&u, &v, &u);
		fp_sub(&A, &C, &A);
		fp_sub(&B, &D, &B);
	} else {
		/* v - v - u, C = C - A, D = D - B */
		fp_sub(&v, &u, &v);
		fp_sub(&C, &A, &C);
		fp_sub(&D, &B, &D);
	}

	/* if not zero goto step 4 */
	if (fp_iszero (&u) == 0)
		goto top;

	/* now a = C, b = D, gcd == g*v */

	/* if v != 1 then there is no inverse */
	if (fp_cmp_d(&v, 1) != FP_EQ) {
		return FP_VAL;
	}

	/* if its too low */
	while (fp_cmp_d(&C, 0) == FP_LT) {
		fp_add(&C, b, &C);
	}

	/* too big */
	while (fp_cmp_mag(&C, b) != FP_LT) {
		fp_sub(&C, b, &C);
	}

	/* C is now the inverse */
	fp_copy(&C, c);
	return FP_OKAY;
}

/* c = 1/a (mod b) for odd b only */
int fp_invmod(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	struct fp_int x, y, u, v, B, D;
	int neg;

	/* 2. [modified] b must be odd   */
	if (fp_iseven (b) == FP_YES) {
		return fp_invmod_slow(a, b, c);
	}

	/* init all our temps */
	fp_init(&x);
	fp_init(&y);
	fp_init(&u);
	fp_init(&v);
	fp_init(&B);
	fp_init(&D);

	/* x == modulus, y == value to invert */
	fp_copy(b, &x);

	/* we need y = |a| */
	fp_abs(a, &y);

	/* 3. u=x, v=y, A=1, B=0, C=0,D=1 */
	fp_copy(&x, &u);
	fp_copy(&y, &v);
	fp_set(&D, 1);

	top:
	/* 4.  while u is even do */
	while (fp_iseven (&u) == FP_YES) {
		/* 4.1 u = u/2 */
		fp_div_2(&u, &u);

		/* 4.2 if B is odd then */
		if (fp_isodd (&B) == FP_YES) {
			fp_sub(&B, &x, &B);
		}
		/* B = B/2 */
		fp_div_2(&B, &B);
	}

	/* 5.  while v is even do */
	while (fp_iseven (&v) == FP_YES) {
		/* 5.1 v = v/2 */
		fp_div_2(&v, &v);

		/* 5.2 if D is odd then */
		if (fp_isodd (&D) == FP_YES) {
			/* D = (D-x)/2 */
			fp_sub(&D, &x, &D);
		}
		/* D = D/2 */
		fp_div_2(&D, &D);
	}

	/* 6.  if u >= v then */
	if (fp_cmp(&u, &v) != FP_LT) {
		/* u = u - v, B = B - D */
		fp_sub(&u, &v, &u);
		fp_sub(&B, &D, &B);
	} else {
		/* v - v - u, D = D - B */
		fp_sub(&v, &u, &v);
		fp_sub(&D, &B, &D);
	}

	/* if not zero goto step 4 */
	if (fp_iszero (&u) == FP_NO) {
		goto top;
	}

	/* now a = C, b = D, gcd == g*v */

	/* if v != 1 then there is no inverse */
	if (fp_cmp_d(&v, 1) != FP_EQ) {
		return FP_VAL;
	}

	/* b is now the inverse */
	neg = a->sign;
	while (D.sign == FP_NEG) {
		fp_add(&D, b, &D);
	}
	fp_copy(&D, c);
	c->sign = neg;
	return FP_OKAY;
}

/* c = [a, b] */
void fp_lcm(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	struct fp_int t1, t2;

	fp_init(&t1);
	fp_init(&t2);
	fp_gcd(a, b, &t1);
	if (fp_cmp_mag(a, b) == FP_GT) {
		fp_div(a, &t1, &t2, NULL);
		fp_mul(b, &t2, c);
	} else {
		fp_div(b, &t1, &t2, NULL);
		fp_mul(a, &t2, c);
	}
}

/* This is possibly the mother of all prime generation functions, muahahahahaha! */
int fp_prime_random_ex(struct fp_int *a, int t, int size, int flags, tfm_prime_callback cb, void *dat) {
	unsigned char *tmp, maskAND, maskOR_msb, maskOR_lsb;
	int res, err, bsize, maskOR_msb_offset;
	/* sanity check the input */
	if (size <= 1 || t <= 0) {
		return FP_VAL;
	}
	/* TFM_PRIME_SAFE implies TFM_PRIME_BBS */
	if (flags & TFM_PRIME_SAFE) {
		flags |= TFM_PRIME_BBS;
	}
	/* calc the byte size */
	bsize = (size >> 3) + (size & 7 ? 1 : 0);

	/* we need a buffer of bsize bytes */
	tmp = malloc(bsize);
	if (tmp == NULL) {
		return FP_MEM;
	}

	/* calc the maskAND value for the MSbyte*/
	maskAND = 0xFF >> (8 - (size & 7));

	/* calc the maskOR_msb */
	maskOR_msb = 0;
	maskOR_msb_offset = (size - 2) >> 3;
	if (flags & TFM_PRIME_2MSB_ON) {
		maskOR_msb |= 1 << ((size - 2) & 7);
	} else if (flags & TFM_PRIME_2MSB_OFF) {
		maskAND &= ~(1 << ((size - 2) & 7));
	}

	/* get the maskOR_lsb */
	maskOR_lsb = 1;
	if (flags & TFM_PRIME_BBS) {
		maskOR_lsb |= 3;
	}

	do {
		/* read the bytes */
		if (cb(tmp, bsize, dat) != bsize) {
			err = FP_VAL;
			goto error;
		}

		/* work over the MSbyte */
		tmp[0] &= maskAND;
		tmp[0] |= 1 << ((size - 1) & 7);

		/* mix in the maskORs */
		tmp[maskOR_msb_offset] |= maskOR_msb;
		tmp[bsize - 1] |= maskOR_lsb;

		/* read it in */
		fp_read_unsigned_bin(a, tmp, bsize);

		/* is it prime? */
		res = fp_isprime(a);
		if (res == FP_NO)
			continue;

		if (flags & TFM_PRIME_SAFE) {
			/* see if (a-1)/2 is prime */
			fp_sub_d(a, 1, a);
			fp_div_2(a, a);

			/* is it prime? */
			res = fp_isprime(a);
		}
	} while (res == FP_NO);

	if (flags & TFM_PRIME_SAFE) {
		/* restore a to the original value */
		fp_mul_2(a, a);
		fp_add_d(a, 1, a);
	}

	err = FP_OKAY;
	error: free(tmp);
	return err;
}

/* c = a * a (mod b) */
int fp_sqrmod(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
	struct fp_int tmp;
	fp_zero(&tmp);
	fp_sqr(a, &tmp);
	return fp_mod(&tmp, b, c);
}

int fp_to_unsigned_bin(struct fp_int *a, unsigned char *b) {
	int x;
	struct fp_int t;

	fp_init_copy(&t, a);

	x = 0;
	while (fp_iszero (&t) == FP_NO) {
		b[x++] = (unsigned char) (t.dp[0] & 255);
		fp_div_2d(&t, 8, &t, NULL);
	}
	fp_reverse(b, x);
	return x;
}

/* reverse an array, used for radix code */
void fp_reverse(unsigned char *s, int len) {
	int ix, iy;
	unsigned char t;
	ix = 0;
	iy = len - 1;
	while (ix < iy) {
		t = s[ix];
		s[ix] = s[iy];
		s[iy] = t;
		++ix;
		--iy;
	}
}

/* b = a/2 */
void fp_div_2(struct fp_int * a, struct fp_int * b) {
	int x, oldused;

	oldused = b->used;
	b->used = a->used;
	{
		register fp_digit r, rr, *tmpa, *tmpb;

		/* source alias */
		tmpa = a->dp + b->used - 1;

		/* dest alias */
		tmpb = b->dp + b->used - 1;

		/* carry */
		r = 0;
		for (x = b->used - 1; x >= 0; x--) {
			/* get the carry for the next iteration */
			rr = *tmpa & 1;

			/* shift the current digit, add in carry and store */
			*tmpb-- = (*tmpa-- >> 1) | (r << (DIGIT_BIT - 1));

			/* forward carry to next iteration */
			r = rr;
		}

		/* zero excess digits */
		tmpb = b->dp + b->used;
		for (x = b->used; x < oldused; x++) {
			*tmpb++ = 0;
		}
	}
	b->sign = a->sign;
	fp_clamp(b);
}

/* b = a*a  */
void fp_sqr(struct fp_int *A, struct fp_int *B) {
	int y;
	if (A->used + A->used > FP_SIZE) {
		fp_sqr_comba(A, B);
		return;
	}
	y = A->used;
	fp_sqr_comba(A, B);
}

/* c = a mod b, 0 <= c < b  */
int fp_mod_d(struct fp_int *a, fp_digit b, fp_digit *c) {
	return fp_div_d(a, b, NULL, c);
}

/* c = a * b */
void fp_mul(struct fp_int *A, struct fp_int *B, struct fp_int *C) {
	int y, yy;

	/* call generic if we're out of range */
	if (A->used + B->used > FP_SIZE) {
		fp_mul_comba(A, B, C);
		return;
	}

	y = MAX(A->used, B->used);
	yy = MIN(A->used, B->used);
	/* pick a comba (unrolled 4/8/16/32 x or rolled) based on the size
	 of the largest input.  We also want to avoid doing excess mults if the
	 inputs are not close to the next power of two.  That is, for example,
	 if say y=17 then we would do (32-17)^2 = 225 unneeded multiplications
	 */

#ifdef TFM_MUL3
	if (y <= 3) {
		fp_mul_comba3(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL4
	if (y == 4) {
		fp_mul_comba4(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL6
	if (y <= 6) {
		fp_mul_comba6(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL7
	if (y == 7) {
		fp_mul_comba7(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL8
	if (y == 8) {
		fp_mul_comba8(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL9
	if (y == 9) {
		fp_mul_comba9(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL12
	if (y <= 12) {
		fp_mul_comba12(A,B,C);
		return;
	}
#endif
#ifdef TFM_MUL17
	if (y <= 17) {
		fp_mul_comba17(A,B,C);
		return;
	}
#endif

#ifdef TFM_SMALL_SET
	if (y <= 16) {
		fp_mul_comba_small(A,B,C);
		return;
	}
#endif
#if defined(TFM_MUL20)
	if (y <= 20) {
		fp_mul_comba20(A,B,C);
		return;
	}
#endif
#if defined(TFM_MUL24)
	if (yy >= 16 && y <= 24) {
		fp_mul_comba24(A,B,C);
		return;
	}
#endif
#if defined(TFM_MUL28)
	if (yy >= 20 && y <= 28) {
		fp_mul_comba28(A,B,C);
		return;
	}
#endif
#if defined(TFM_MUL32)
	if (yy >= 24 && y <= 32) {
		fp_mul_comba32(A,B,C);
		return;
	}
#endif
#if defined(TFM_MUL48)
	if (yy >= 40 && y <= 48) {
		fp_mul_comba48(A,B,C);
		return;
	}
#endif
#if defined(TFM_MUL64)
	if (yy >= 56 && y <= 64) {
		fp_mul_comba64(A,B,C);
		return;
	}
#endif
	fp_mul_comba(A, B, C);
}

static int s_is_power_of_two(fp_digit b, int *p) {
	int x;

	/* fast return if no power of two */
	if ((b == 0) || (b & (b - 1))) {
		return 0;
	}

	for (x = 0; x < DIGIT_BIT; x++) {
		if (b == (((fp_digit) 1) << x)) {
			*p = x;
			return 1;
		}
	}
	return 0;
}

/* a/b => cb + d == a */
int fp_div_d(struct fp_int *a, fp_digit b, struct fp_int *c, fp_digit *d) {
	struct fp_int q;
	fp_word w;
	fp_digit t;
	int ix;

	/* cannot divide by zero */
	if (b == 0) {
		return FP_VAL;
	}

	/* quick outs */
	if (b == 1 || fp_iszero(a) == 1) {
		if (d != NULL) {
			*d = 0;
		}
		if (c != NULL) {
			fp_copy(a, c);
		}
		return FP_OKAY;
	}

	/* power of two ? */
	if (s_is_power_of_two(b, &ix) == 1) {
		if (d != NULL) {
			*d = a->dp[0] & ((((fp_digit) 1) << ix) - 1);
		}
		if (c != NULL) {
			fp_div_2d(a, ix, c, NULL);
		}
		return FP_OKAY;
	}

	/* no easy answer [c'est la vie].  Just division */
	fp_init(&q);

	q.used = a->used;
	q.sign = a->sign;
	w = 0;
	for (ix = a->used - 1; ix >= 0; ix--) {
		w = (w << ((fp_word) DIGIT_BIT)) | ((fp_word) a->dp[ix]);

		if (w >= b) {
			t = (fp_digit) (w / b);
			w -= ((fp_word) t) * ((fp_word) b);
		} else {
			t = 0;
		}
		q.dp[ix] = (fp_digit) t;
	}

	if (d != NULL) {
		*d = (fp_digit) w;
	}

	if (c != NULL) {
		fp_clamp(&q);
		fp_copy(&q, c);
	}

	return FP_OKAY;
}

void fp_mul_2(struct fp_int * a, struct fp_int * b) {
	int x, oldused;

	oldused = b->used;
	b->used = a->used;

	{
		register fp_digit r, rr, *tmpa, *tmpb;

		/* alias for source */
		tmpa = a->dp;

		/* alias for dest */
		tmpb = b->dp;

		/* carry */
		r = 0;
		for (x = 0; x < a->used; x++) {

			/* get what will be the *next* carry bit from the
			 * MSB of the current digit
			 */
			rr = *tmpa >> ((fp_digit) (DIGIT_BIT - 1));

			/* now shift up this digit, add in the carry [from the previous] */
			*tmpb++ = ((*tmpa++ << ((fp_digit) 1)) | r);

			/* copy the carry that would be from the source
			 * digit into the next iteration
			 */
			r = rr;
		}

		/* new leading digit? */
		if (r != 0 && b->used != (FP_SIZE - 1)) {
			/* add a MSB which is always 1 at this point */
			*tmpb = 1;
			++(b->used);
		}

		/* now zero any excess digits on the destination
		 * that we didn't write to
		 */
		tmpb = b->dp + b->used;
		for (x = b->used; x < oldused; x++) {
			*tmpb++ = 0;
		}
	}
	b->sign = a->sign;
}

/* Miller-Rabin test of "a" to the base of "b" as described in
 * HAC pp. 139 Algorithm 4.24
 *
 * Sets result to 0 if definitely composite or 1 if probably prime.
 * Randomly the chance of error is no more than 1/4 and often
 * very much lower.
 */
void fp_prime_miller_rabin(struct fp_int * a, struct fp_int * b, int *result) {
	struct fp_int n1, y, r;
	int s, j;

	/* default */
	*result = FP_NO;

	/* ensure b > 1 */
	if (fp_cmp_d(b, 1) != FP_GT) {
		return;
	}

	/* get n1 = a - 1 */
	fp_init_copy(&n1, a);
	fp_sub_d(&n1, 1, &n1);

	/* set 2**s * r = n1 */
	fp_init_copy(&r, &n1);

	/* count the number of least significant bits
	 * which are zero
	 */
	s = fp_cnt_lsb(&r);

	/* now divide n - 1 by 2**s */
	fp_div_2d(&r, s, &r, NULL);

	/* compute y = b**r mod a */
	fp_init(&y);
	fp_exptmod(b, &r, a, &y);

	/* if y != 1 and y != n1 do */
	if (fp_cmp_d(&y, 1) != FP_EQ && fp_cmp(&y, &n1) != FP_EQ) {
		j = 1;
		/* while j <= s-1 and y != n1 */
		while ((j <= (s - 1)) && fp_cmp(&y, &n1) != FP_EQ) {
			fp_sqrmod(&y, a, &y);

			/* if y == 1 then composite */
			if (fp_cmp_d(&y, 1) == FP_EQ) {
				return;
			}
			++j;
		}

		/* if y != n1 then composite */
		if (fp_cmp(&y, &n1) != FP_EQ) {
			return;
		}
	}

	/* probably prime now */
	*result = FP_YES;
}

void fp_read_unsigned_bin(struct fp_int *a, unsigned char *b, int c) {
	/* zero the int */
	fp_zero(a);
	/* read the bytes in */
	for (; c > 0; c--) {
		fp_mul_2d(a, 8, a);
		a->dp[0] |= *b++;
		a->used += 1;
	}
	fp_clamp(a);
}

void fp_sqr_comba(struct fp_int *A, struct fp_int *B) {
	int pa, ix, iz;
	fp_digit c0, c1, c2;
	struct fp_int tmp, *dst;

	/* get size of output and trim */
	pa = A->used + A->used;
	if (pa >= FP_SIZE) {
		pa = FP_SIZE - 1;
	}

	/* number of output digits to produce */CLEAR_CARRY;

	if (A == B) {
		fp_zero(&tmp);
		dst = &tmp;
	} else {
		fp_zero(B);
		dst = B;
	}

	for (ix = 0; ix < pa; ix++) {
		int tx, ty, iy;
		fp_digit *tmpy, *tmpx;

		/* get offsets into the two bignums */
		ty = MIN(A->used-1, ix);
		tx = ix - ty;

		/* setup temp aliases */
		tmpx = A->dp + tx;
		tmpy = A->dp + ty;

		/* this is the number of times the loop will iterrate,
		 while (tx++ < a->used && ty-- >= 0) { ... }
		 */
		iy = MIN(A->used-tx, ty+1);

		/* now for squaring tx can never equal ty
		 * we halve the distance since they approach
		 * at a rate of 2x and we have to round because
		 * odd cases need to be executed
		 */
		iy = MIN(iy, (ty-tx+1)>>1);

		/* forward carries */CARRY_FORWARD;

		/* execute loop */
		for (iz = 0; iz < iy; iz++) {
			SQRADD2(*tmpx++, *tmpy--);
		}

		/* even columns have the square term in them */
		if ((ix & 1) == 0) {
			SQRADD(A->dp[ix >> 1], A->dp[ix >> 1]);
		}

		/* store it */
		COMBA_STORE(dst->dp[ix]);
	}
	/* setup dest */
	dst->used = pa;
	fp_clamp(dst);
	if (dst != B) {
		fp_copy(dst, B);
	}
}

/* generic PxQ multiplier */
void fp_mul_comba(struct fp_int *A, struct fp_int *B, struct fp_int *C) {
	int ix, iy, iz, tx, ty, pa;
	fp_digit c0, c1, c2, *tmpx, *tmpy;
	struct fp_int tmp, *dst;
	COMBA_CLEAR;
	/* get size of output and trim */
	pa = A->used + B->used;
	if (pa >= FP_SIZE) {
		pa = FP_SIZE - 1;
	}

	if (A == C || B == C) {
		fp_zero(&tmp);
		dst = &tmp;
	} else {
		fp_zero(C);
		dst = C;
	}

	for (ix = 0; ix < pa; ix++) {
		/* get offsets into the two bignums */
		ty = MIN(ix, B->used-1);
		tx = ix - ty;

		/* setup temp aliases */
		tmpx = A->dp + tx;
		tmpy = B->dp + ty;

		/* this is the number of times the loop will iterrate, essentially its
		 while (tx++ < a->used && ty-- >= 0) { ... }
		 */
		iy = MIN(A->used-tx, ty+1);

		/* execute loop */COMBA_FORWARD;
		for (iz = 0; iz < iy; ++iz) {
			MULADD(*tmpx++, *tmpy--);
		}

		/* store term */
		COMBA_STORE(dst->dp[ix]);
	}
	dst->used = pa;
	dst->sign = A->sign ^ B->sign;
	fp_clamp(dst);
	fp_copy(dst, C);
}

/* Counts the number of lsbs which are zero before the first zero bit */
int fp_cnt_lsb(struct fp_int *a) {
	int x;
	fp_digit q, qq;
	/* easy out */
	if (fp_iszero(a) == 1) {
		return 0;
	}
	/* scan lower digits until non-zero */
	for (x = 0; x < a->used && a->dp[x] == 0; x++) {
	}
	q = a->dp[x];
	x *= DIGIT_BIT;
	/* now scan this digit until a 1 is found */
	if ((q & 1) == 0) {
		do {
			qq = q & 15;
			x += lnz[qq];
			q >>= 4;
		} while (qq == 0);
	}
	return x;
}

/* computes x/R == x (mod N) via Montgomery Reduction */
void fp_montgomery_reduce(struct fp_int *a, struct fp_int *m, fp_digit mp) {
	fp_digit c[FP_SIZE], *_c, *tmpm, mu;
	int oldused, x, y, pa;
	/* bail if too large */
	if (m->used > (FP_SIZE / 2)) {
		return;
	}
	pa = m->used;
	memset(c, 0, sizeof c);
	/* copy the input */
	oldused = a->used;
	for (x = 0; x < oldused; x++) {
		c[x] = a->dp[x];
	}
	for (x = 0; x < pa; x++) {
		fp_digit cy = 0;
		mu = c[x] * mp;
		_c = c + x;
		tmpm = m->dp;
		y = 0;
		for (; y < pa; y++) {
			fp_word t;
			_c[0] = t = ((fp_word) _c[0] + (fp_word) cy) + (((fp_word) mu) * ((fp_word) *tmpm++));
			cy = (t >> (int) ((8) * sizeof(fp_digit)));
			++_c;
		}
		while (cy) {
			fp_digit t = _c[0] += cy;
			cy = (t < cy);
			++_c;
		}
	}
	/* now copy out */
	_c = c + pa;
	tmpm = a->dp;
	for (x = 0; x < pa + 1; x++) {
		*tmpm++ = *_c++;
	}
	for (; x < oldused; x++) {
		*tmpm++ = 0;
	}
	a->used = pa + 1;
	fp_clamp(a);
	/* if A >= m then A = A - m */
	if (fp_cmp_mag(a, m) != FP_LT) {
		s_fp_sub(a, m, a);
	}
}

/* y = g**x (mod b)
 * Some restrictions... x must be positive and < b
 */
static int _fp_exptmod(struct fp_int * G, struct fp_int * X, struct fp_int * P, struct fp_int * Y) {
	struct fp_int M[64], res;
	fp_digit buf, mp;
	int err, bitbuf, bitcpy, bitcnt, mode, digidx, x, y, winsize;

	/* find window size */
	x = fp_count_bits(X);
	if (x <= 21) {
		winsize = 1;
	} else if (x <= 36) {
		winsize = 3;
	} else if (x <= 140) {
		winsize = 4;
	} else if (x <= 450) {
		winsize = 5;
	} else {
		winsize = 6;
	}

	/* init M array */
	memset(M, 0, sizeof(M));

	/* now setup montgomery  */
	if ((err = fp_montgomery_setup(P, &mp)) != FP_OKAY) {
		return err;
	}

	/* setup result */
	fp_init(&res);

	/* create M table
	 *
	 * The M table contains powers of the input base, e.g. M[x] = G^x mod P
	 *
	 * The first half of the table is not computed though accept for M[0] and M[1]
	 */

	/* now we need R mod m */
	fp_montgomery_calc_normalization(&res, P);

	/* now set M[1] to G * R mod m */
	if (fp_cmp_mag(P, G) != FP_GT) {
		/* G > P so we reduce it first */
		fp_mod(G, P, &M[1]);
	} else {
		fp_copy(G, &M[1]);
	}
	fp_mulmod(&M[1], &res, P, &M[1]);

	/* compute the value at M[1<<(winsize-1)] by squaring M[1] (winsize-1) times */
	fp_copy(&M[1], &M[1 << (winsize - 1)]);
	for (x = 0; x < (winsize - 1); x++) {
		fp_sqr(&M[1 << (winsize - 1)], &M[1 << (winsize - 1)]);
		fp_montgomery_reduce(&M[1 << (winsize - 1)], P, mp);
	}

	/* create upper table */
	for (x = (1 << (winsize - 1)) + 1; x < (1 << winsize); x++) {
		fp_mul(&M[x - 1], &M[1], &M[x]);
		fp_montgomery_reduce(&M[x], P, mp);
	}

	/* set initial mode and bit cnt */
	mode = 0;
	bitcnt = 1;
	buf = 0;
	digidx = X->used - 1;
	bitcpy = 0;
	bitbuf = 0;

	for (;;) {
		/* grab next digit as required */
		if (--bitcnt == 0) {
			/* if digidx == -1 we are out of digits so break */
			if (digidx == -1) {
				break;
			}
			/* read next digit and reset bitcnt */
			buf = X->dp[digidx--];
			bitcnt = (int) DIGIT_BIT;
		}

		/* grab the next msb from the exponent */
		y = (fp_digit) (buf >> (DIGIT_BIT - 1)) & 1;
		buf <<= (fp_digit) 1;

		/* if the bit is zero and mode == 0 then we ignore it
		 * These represent the leading zero bits before the first 1 bit
		 * in the exponent.  Technically this opt is not required but it
		 * does lower the # of trivial squaring/reductions used
		 */
		if (mode == 0 && y == 0) {
			continue;
		}

		/* if the bit is zero and mode == 1 then we square */
		if (mode == 1 && y == 0) {
			fp_sqr(&res, &res);
			fp_montgomery_reduce(&res, P, mp);
			continue;
		}

		/* else we add it to the window */
		bitbuf |= (y << (winsize - ++bitcpy));
		mode = 2;

		if (bitcpy == winsize) {
			/* ok window is filled so square as required and multiply  */
			/* square first */
			for (x = 0; x < winsize; x++) {
				fp_sqr(&res, &res);
				fp_montgomery_reduce(&res, P, mp);
			}

			/* then multiply */
			fp_mul(&res, &M[bitbuf], &res);
			fp_montgomery_reduce(&res, P, mp);

			/* empty window and reset */
			bitcpy = 0;
			bitbuf = 0;
			mode = 1;
		}
	}

	/* if bits remain then square/multiply */
	if (mode == 2 && bitcpy > 0) {
		/* square then multiply if the bit is set */
		for (x = 0; x < bitcpy; x++) {
			fp_sqr(&res, &res);
			fp_montgomery_reduce(&res, P, mp);

			/* get next bit of the window */
			bitbuf <<= 1;
			if ((bitbuf & (1 << winsize)) != 0) {
				/* then multiply */
				fp_mul(&res, &M[1], &res);
				fp_montgomery_reduce(&res, P, mp);
			}
		}
	}

	/* fixup result if Montgomery reduction is used
	 * recall that any value in a Montgomery system is
	 * actually multiplied by R mod n.  So we have
	 * to reduce one more time to cancel out the factor
	 * of R.
	 */
	fp_montgomery_reduce(&res, P, mp);

	/* swap res with Y */
	fp_copy(&res, Y);
	return FP_OKAY;
}

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
void fp_montgomery_calc_normalization(struct fp_int *a, struct fp_int *b) {
	int x, bits;

	/* how many bits of last digit does b use */
	bits = fp_count_bits(b) % DIGIT_BIT;
	if (!bits)
		bits = DIGIT_BIT;

	/* compute A = B^(n-1) * 2^(bits-1) */
	if (b->used > 1) {
		fp_2expt(a, (b->used - 1) * DIGIT_BIT + bits - 1);
	} else {
		fp_set(a, 1);
		bits = 1;
	}

	/* now compute C = A * B mod b */
	for (x = bits - 1; x < (int) DIGIT_BIT; x++) {
		fp_mul_2(a, a);
		if (fp_cmp_mag(a, b) != FP_LT) {
			s_fp_sub(a, b, a);
		}
	}
}

/* setups the montgomery reduction */
int fp_montgomery_setup(struct fp_int *a, fp_digit *rho) {
	fp_digit x, b;

	/* fast inversion mod 2**k
	 *
	 * Based on the fact that
	 *
	 * XA = 1 (mod 2**n)  =>  (X(2-XA)) A = 1 (mod 2**2n)
	 *                    =>  2*X*A - X*X*A*A = 1
	 *                    =>  2*(1) - (1)     = 1
	 */
	b = a->dp[0];

	if ((b & 1) == 0) {
		return FP_VAL;
	}

	x = (((b + 2) & 4) << 1) + b; /* here x*a==1 mod 2**4 */
	x *= 2 - b * x; /* here x*a==1 mod 2**8 */
	x *= 2 - b * x; /* here x*a==1 mod 2**16 */
	x *= 2 - b * x; /* here x*a==1 mod 2**32 */
#ifdef FP_64BIT
	x *= 2 - b * x; /* here x*a==1 mod 2**64 */
#endif

	/* rho = -1/m mod b */
	*rho = (((fp_word) 1 << ((fp_word) DIGIT_BIT)) - ((fp_word) x));

	return FP_OKAY;
}

int fp_exptmod(struct fp_int * G, struct fp_int * X, struct fp_int * P, struct fp_int * Y) {
	struct fp_int tmp;
	int err;
	/* is X negative?  */
	if (X->sign == FP_NEG) {
		/* yes, copy G and invmod it */
		fp_copy(G, &tmp);
		if ((err = fp_invmod(&tmp, P, &tmp)) != FP_OKAY) {
			return err;
		}
		X->sign = FP_ZPOS;
		err = _fp_exptmod(&tmp, X, P, Y);
		if (X != Y) {
			X->sign = FP_NEG;
		}
		return err;
	} else {
		/* Positive exponent so just exptmod */
		return _fp_exptmod(G, X, P, Y);
	}
}

/* computes a = 2**b */
void fp_2expt(struct fp_int *a, int b) {
	int z;

	/* zero a as per default */
	fp_zero(a);

	if (b < 0) {
		return;
	}

	z = b / DIGIT_BIT;
	if (z >= FP_SIZE) {
		return;
	}

	/* set the used count of where the bit will go */
	a->used = z + 1;

	/* put the single bit in its place */
	a->dp[z] = ((fp_digit) 1) << (b % DIGIT_BIT);
}

/* d = a * b (mod c) */
int fp_mulmod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d) {
	struct fp_int tmp;
	fp_zero(&tmp);
	fp_mul(a, b, &tmp);
	return fp_mod(&tmp, c, d);
}

int fp_unsigned_bin_size(struct fp_int *a) {
	register int size = fp_count_bits(a);
	return (size / 8 + ((size & 7) != 0 ? 1 : 0));
}

int fp_signed_bin_size(struct fp_int *a) {
	return 1 + fp_unsigned_bin_size(a);
}

void fp_read_signed_bin(struct fp_int *a, unsigned char *b, int c) {
	/* read magnitude */
	fp_read_unsigned_bin(a, b + 1, c - 1);

	/* first byte is 0 for positive, non-zero for negative */
	if (b[0] == 0) {
		a->sign = FP_ZPOS;
	} else {
		a->sign = FP_NEG;
	}
}

void fp_to_signed_bin(struct fp_int *a, unsigned char *b) {
	fp_to_unsigned_bin(a, b + 1);
	b[0] = (unsigned char) ((a->sign == FP_ZPOS) ? 0 : 1);
}

int fp_read_radix(struct fp_int *a, char *str, int radix) {
	int y, neg;
	char ch;

	/* make sure the radix is ok */
	if (radix < 2 || radix > 64) {
		return FP_VAL;
	}

	/* if the leading digit is a
	 * minus set the sign to negative.
	 */
	if (*str == '-') {
		++str;
		neg = FP_NEG;
	} else {
		neg = FP_ZPOS;
	}

	/* set the integer to the default of zero */
	fp_zero(a);

	/* process each digit of the string */
	while (*str) {
		/* if the radix < 36 the conversion is case insensitive
		 * this allows numbers like 1AB and 1ab to represent the same  value
		 * [e.g. in hex]
		 */
		ch = (char) ((radix < 36) ? toupper(*str) : *str);
		for (y = 0; y < 64; y++) {
			if (ch == fp_s_rmap[y]) {
				break;
			}
		}

		/* if the char was found in the map
		 * and is less than the given radix add it
		 * to the number, otherwise exit the loop.
		 */
		if (y < radix) {
			fp_mul_d(a, (fp_digit) radix, a);
			fp_add_d(a, (fp_digit) y, a);
		} else {
			break;
		}
		++str;
	}

	/* set the sign only if a != 0 */
	if (fp_iszero(a) != FP_YES) {
		a->sign = neg;
	}
	return FP_OKAY;
}

int fp_to_int(struct fp_int *a) {
	return *(int*) a->dp;
}

int fp_toradix(struct fp_int *a, char *str, int radix) {
	int digs;
	struct fp_int t;
	fp_digit d;
	char *_s = str;

	/* check range of the radix */
	if (radix < 2 || radix > 64) {
		return FP_VAL;
	}

	/* quick out if its zero */
	if (fp_iszero(a) == 1) {
		*str++ = '0';
		*str = '\0';
		return FP_OKAY;
	}

	fp_init_copy(&t, a);

	/* if it is negative output a - */
	if (t.sign == FP_NEG) {
		++_s;
		*str++ = '-';
		t.sign = FP_ZPOS;
	}

	digs = 0;
	while (fp_iszero (&t) == FP_NO) {
		fp_div_d(&t, (fp_digit) radix, &t, &d);
		*str++ = fp_s_rmap[d];
		++digs;
	}

	/* reverse the digits of the string.  In this case _s points
	 * to the first digit [exluding the sign] of the number]
	 */
	fp_reverse((unsigned char *) _s, digs);

	/* append a NULL so the string is properly terminated */
	*str = '\0';
	return FP_OKAY;
}
