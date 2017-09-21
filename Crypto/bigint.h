#ifndef HAS_BIG_INT
#define HAS_BIG_INT

#include <limits.h>

#if defined(__x86_64__)
#if defined(TFM_X86) || defined(TFM_SSE2) || defined(TFM_ARM)
#error x86-64 detected, x86-32/SSE2/ARM optimizations are not valid!
#endif
#if !defined(TFM_X86_64) && !defined(TFM_NO_ASM)
#define TFM_X86_64
#endif
#endif
#if defined(TFM_X86_64)
#if !defined(FP_64BIT)
#define FP_64BIT
#endif
#endif

/* try to detect x86-32 */
#if defined(__i386__) && !defined(TFM_SSE2)
#if defined(TFM_X86_64) || defined(TFM_ARM)
#error x86-32 detected, x86-64/ARM optimizations are not valid!
#endif
#if !defined(TFM_X86) && !defined(TFM_NO_ASM)
#define TFM_X86
#endif
#endif

#ifdef _MSC_VER
#define CONST64(n) n ## ui64
typedef unsigned __int64 ulong64;
#else
#define CONST64(n) n ## ULL
typedef unsigned long long ulong64;
#endif

#if defined(__x86_64__) || (defined(__sparc__) && defined(__arch64__))
typedef unsigned int ulong32;
#else
typedef unsigned long ulong32;
#endif

#ifndef MAX
#define MAX(x, y) ( ((x)>(y))?(x):(y) )
#endif

#ifndef MIN
#define MIN(x, y) ( ((x)<(y))?(x):(y) )
#endif

typedef ulong32 fp_digit;
typedef ulong64 fp_word;
#define FP_MAX_SIZE           (2*2*4096+(8*DIGIT_BIT))
#define DIGIT_BIT  (int)((CHAR_BIT) * sizeof(fp_digit))
#define FP_MASK    (fp_digit)(-1)
#define FP_SIZE    (FP_MAX_SIZE/DIGIT_BIT)

/* signs */
#define FP_ZPOS     0
#define FP_NEG      1

/* return codes */
#define FP_OKAY     0
#define FP_VAL      1
#define FP_MEM      2

/* equalities */
#define FP_LT        -1   /* less than */
#define FP_EQ         0   /* equal to */
#define FP_GT         1   /* greater than */

/* replies */
#define FP_YES        1   /* yes response */
#define FP_NO         0   /* no response */

/* a FP type */
struct fp_int {
	fp_digit dp[FP_SIZE];
	int used, sign;
};

/* initialize [or zero] an fp int */
#define fp_init(a)  memset(a, 0, sizeof(struct fp_int))
#define fp_zero(a)  fp_init(a)

/* zero/even/odd ? */
#define fp_iszero(a) (((a)->used == 0) ? FP_YES : FP_NO)
#define fp_iseven(a) (((a)->used >= 0 && (((a)->dp[0] & 1) == 0)) ? FP_YES : FP_NO)
#define fp_isodd(a)  (((a)->used > 0  && (((a)->dp[0] & 1) == 1)) ? FP_YES : FP_NO)

/* set to a small digit */
void fp_set(struct fp_int *a, fp_digit b);

/* copy from a to b */
#define fp_copy(a, b)      (void)(((a) != (b)) && memcpy((b), (a), sizeof(struct fp_int)))
#define fp_init_copy(a, b) fp_copy(b, a)

/* clamp digits */
#define fp_clamp(a)   { while ((a)->used && (a)->dp[(a)->used-1] == 0) --((a)->used); (a)->sign = (a)->used ? (a)->sign : FP_ZPOS; }

/* negate and absolute */
#define fp_neg(a, b)  { fp_copy(a, b); (b)->sign ^= 1; fp_clamp(b); }
#define fp_abs(a, b)  { fp_copy(a, b); (b)->sign  = 0; }

/* Primality generation flags */
#define TFM_PRIME_BBS      0x0001 /* BBS style prime */
#define TFM_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define TFM_PRIME_2MSB_OFF 0x0004 /* force 2nd MSB to 0 */
#define TFM_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

#define fp_s_rmap "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/"

#define fp_prime_random(a, t, size, bbs, cb, dat) fp_prime_random_ex(a, t, ((size) * 8) + 1, (bbs==1)?TFM_PRIME_BBS:0, cb, dat)

/* right shift x digits */
void fp_rshd(struct fp_int *a, int x);

/* left shift x digits */
void fp_lshd(struct fp_int *a, int x);

/* signed comparison */
int fp_cmp(struct fp_int *a, struct fp_int *b);

/* unsigned comparison */
int fp_cmp_mag(struct fp_int *a, struct fp_int *b);

void fp_sqr_comba(struct fp_int *a, struct fp_int *b);

/* power of 2 operations */
void fp_div_2d(struct fp_int *a, int b, struct fp_int *c, struct fp_int *d);
void fp_mod_2d(struct fp_int *a, int b, struct fp_int *c);
void fp_mul_2d(struct fp_int *a, int b, struct fp_int *c);
void fp_2expt(struct fp_int *a, int b);
void fp_mul_2(struct fp_int *a, struct fp_int *c);
void fp_div_2(struct fp_int *a, struct fp_int *c);

/* Counts the number of lsbs which are zero before the first zero bit */
int fp_cnt_lsb(struct fp_int *a);

/* c = a + b */
void fp_add(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* c = a - b */
void fp_sub(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* c = a * b */
void fp_mul(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* b = a*a  */
void fp_sqr(struct fp_int *a, struct fp_int *b);

/* a/b => cb + d == a */
int fp_div(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d);

/* c = a mod b, 0 <= c < b  */
int fp_mod(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* compare against a single digit */
int fp_cmp_d(struct fp_int *a, fp_digit b);

/* c = a + b */
void fp_add_d(struct fp_int *a, fp_digit b, struct fp_int *c);

/* c = a - b */
void fp_sub_d(struct fp_int *a, fp_digit b, struct fp_int *c);

/* c = a * b */
void fp_mul_d(struct fp_int *a, fp_digit b, struct fp_int *c);

/* a/b => cb + d == a */
int fp_div_d(struct fp_int *a, fp_digit b, struct fp_int *c, fp_digit *d);

/* c = a mod b, 0 <= c < b  */
int fp_mod_d(struct fp_int *a, fp_digit b, fp_digit *c);

/* ---> number theory <--- */
/* d = a + b (mod c) */
int fp_addmod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d);

/* d = a - b (mod c) */
int fp_submod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d);

/* d = a * b (mod c) */
int fp_mulmod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d);

/* c = a * a (mod b) */
int fp_sqrmod(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* c = 1/a (mod b) */
int fp_invmod(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* c = (a, b) */
void fp_gcd(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* c = [a, b] */
void fp_lcm(struct fp_int *a, struct fp_int *b, struct fp_int *c);

/* setups the montgomery reduction */
int fp_montgomery_setup(struct fp_int *a, fp_digit *mp);

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
void fp_montgomery_calc_normalization(struct fp_int *a, struct fp_int *b);

/* computes x/R == x (mod N) via Montgomery Reduction */
void fp_montgomery_reduce(struct fp_int *a, struct fp_int *m, fp_digit mp);

/* d = a**b (mod c) */
int fp_exptmod(struct fp_int *a, struct fp_int *b, struct fp_int *c, struct fp_int *d);

/* primality stuff */

/* perform a Miller-Rabin test of a to the base b and store result in "result" */
void fp_prime_miller_rabin(struct fp_int * a, struct fp_int * b, int *result);

/* 256 trial divisions + 8 Miller-Rabins, returns FP_YES if probable prime  */
int fp_isprime(struct fp_int *a);

/* Primality generation flags */
#define TFM_PRIME_BBS      0x0001 /* BBS style prime */
#define TFM_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define TFM_PRIME_2MSB_OFF 0x0004 /* force 2nd MSB to 0 */
#define TFM_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

/* callback for fp_prime_random, should fill dst with random bytes and return how many read [upto len] */
typedef int tfm_prime_callback(unsigned char *dst, int len, void *dat);

int fp_prime_random_ex(struct fp_int *a, int t, int size, int flags, tfm_prime_callback cb, void *dat);

/* radix conersions */
int fp_count_bits(struct fp_int *a);

int fp_unsigned_bin_size(struct fp_int *a);
void fp_read_unsigned_bin(struct fp_int *a, unsigned char *b, int c);
int fp_to_unsigned_bin(struct fp_int *a, unsigned char *b);

int fp_signed_bin_size(struct fp_int *a);
void fp_read_signed_bin(struct fp_int *a, unsigned char *b, int c);
void fp_to_signed_bin(struct fp_int *a, unsigned char *b);

int fp_read_radix(struct fp_int *a, char *str, int radix);
int fp_toradix(struct fp_int *a, char *str, int radix);
int fp_to_int(struct fp_int *a);

/* VARIOUS LOW LEVEL STUFFS */
void s_fp_add(struct fp_int *a, struct fp_int *b, struct fp_int *c);
void s_fp_sub(struct fp_int *a, struct fp_int *b, struct fp_int *c);
void fp_reverse(unsigned char *s, int len);

void fp_mul_comba(struct fp_int *A, struct fp_int *B, struct fp_int *C);

#endif
