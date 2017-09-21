#ifndef HAS_RAND
#define HAS_RAND

#ifndef STANDARD
# define STANDARD
# ifndef STDIO
#  include <stdio.h>
#  define STDIO
# endif
# ifndef STDDEF
#  include <stddef.h>
#  define STDDEF
# endif
typedef unsigned long long ub8;
#define UB8MAXVAL 0xffffffffffffffffLL
#define UB8BITS 64
typedef signed long long sb8;
#define SB8MAXVAL 0x7fffffffffffffffLL
typedef unsigned long int ub4; /* unsigned 4-byte quantities */
#define UB4MAXVAL 0xffffffff
typedef signed long int sb4;
#define UB4BITS 32
#define SB4MAXVAL 0x7fffffff
typedef unsigned short int ub2;
#define UB2MAXVAL 0xffff
#define UB2BITS 16
typedef signed short int sb2;
#define SB2MAXVAL 0x7fff
typedef unsigned char ub1;
#define UB1MAXVAL 0xff
#define UB1BITS 8
typedef signed char sb1; /* signed 1-byte quantities */
#define SB1MAXVAL 0x7f
typedef int word; /* fastest type available */

#define bis(target,mask)  ((target) |=  (mask))
#define bic(target,mask)  ((target) &= ~(mask))
#define bit(target,mask)  ((target) &   (mask))
#ifndef min
# define min(a,b) (((a)<(b)) ? (a) : (b))
#endif /* min */
#ifndef max
# define max(a,b) (((a)<(b)) ? (b) : (a))
#endif /* max */
#ifndef align
# define align(a) (((ub4)a+(sizeof(void *)-1))&(~(sizeof(void *)-1)))
#endif /* align */
#ifndef abs
# define abs(a)   (((a)>0) ? (a) : -(a))
#endif
#define TRUE  1
#define FALSE 0
#define SUCCESS 0  /* 1 on VAX */

#define RANDSIZL   (8)
#define RANDSIZ    (1<<RANDSIZL)
// Para aumentar, deve alterar o método oaes_get_seed

/* context of random number generator */
struct randctx {
	ub4 count;
	ub4 randrsl[RANDSIZ];
	ub4 randmem[RANDSIZ];
	ub4 a;
	ub4 b;
	ub4 c;
};
typedef struct randctx randctx;

/*
 ------------------------------------------------------------------------------
 If (flag==TRUE), then use the contents of randrsl[0..RANDSIZ-1] as the seed.
 ------------------------------------------------------------------------------
 */
void randinit(/*_ randctx *r, word flag _*/);

void isaac(/*_ randctx *r _*/);

/*
 ------------------------------------------------------------------------------
 Call rand(/o_ randctx *r _o/) to retrieve a single 32-bit random value
 ------------------------------------------------------------------------------
 */
#define _rand(r) (!(r)->count-- ? (isaac(r), (r)->count=RANDSIZ-1, (r)->randrsl[(r)->count]) : (r)->randrsl[(r)->count])

void rand_key(unsigned char* buffer, unsigned long buffer_size);

#endif

#endif