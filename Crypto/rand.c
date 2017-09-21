#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include "rand.h"

void rand_key(unsigned char* buffer, unsigned long buffer_size) {
	randctx rctx;
	char seed[RANDSIZ + 1];
	struct timeb timer;
	ftime(&timer);
	struct tm* gmTimer = gmtime(&timer.time);
	char* data = (char*) malloc(timer.millitm);
	if (!data) {
		data = (char*) &rctx;
	}
	sprintf( seed,
			"%04d%02d%02d%02d%02d%02d%03d%p%d", gmTimer->tm_year + 1900, gmTimer->tm_mon + 1, gmTimer->tm_mday, gmTimer->tm_hour, gmTimer->tm_min, gmTimer->tm_sec, timer.millitm, data + timer.millitm, getpid());
	if (data && data != (char*) &rctx) {
		free(data);
	}
	memcpy(rctx.randrsl, seed, RANDSIZ);
	randinit(&rctx);
	size_t i;
	for (i = 0; i < buffer_size; i++) {
		if (rctx.count-- == 0) {
			isaac(&rctx);
			rctx.count = RANDSIZ - 1;
		}
		*buffer++ = (uint8_t) rctx.randrsl[rctx.count];
	}
}

#define ind(mm,x)  (*(ub4 *)((ub1 *)(mm) + ((x) & ((RANDSIZ-1)<<2))))

#define rngstep(mix,a,b,mm,m,m2,r,x) \
{ \
x = *m;  \
a = (a^(mix)) + *(m2++); \
*(m++) = y = ind(mm,x) + a + b; \
*(r++) = b = ind(mm,y>>RANDSIZL) + x; \
}

void isaac(randctx* ctx) {
	register ub4 a, b, x, y, *m, *mm, *m2, *r, *mend;
	mm = ctx->randmem;
	r = ctx->randrsl;
	a = ctx->a;
	b = ctx->b + (++ctx->c);
	for (m = mm, mend = m2 = m + (RANDSIZ / 2); m < mend;) {
		rngstep( a<<13, a, b, mm, m, m2, r, x);
		rngstep( a>>6, a, b, mm, m, m2, r, x);
		rngstep( a<<2, a, b, mm, m, m2, r, x);
		rngstep( a>>16, a, b, mm, m, m2, r, x);
	}
	for (m2 = mm; m2 < mend;) {
		rngstep( a<<13, a, b, mm, m, m2, r, x);
		rngstep( a>>6, a, b, mm, m, m2, r, x);
		rngstep( a<<2, a, b, mm, m, m2, r, x);
		rngstep( a>>16, a, b, mm, m, m2, r, x);
	}
	ctx->b = b;
	ctx->a = a;
}

#define mix(a,b,c,d,e,f,g,h) \
{ \
a^=b<<11; d+=a; b+=c; \
b^=c>>2;  e+=b; c+=d; \
c^=d<<8;  f+=c; d+=e; \
d^=e>>16; g+=d; e+=f; \
e^=f<<10; h+=e; f+=g; \
f^=g>>4;  a+=f; g+=h; \
g^=h<<8;  b+=g; h+=a; \
h^=a>>9;  c+=h; a+=b; \
}

void randinit(randctx* ctx) {
	word i;
	ub4 a, b, c, d, e, f, g, h;
	ub4 *m, *r;
	ctx->a = ctx->b = ctx->c = 0;
	m = ctx->randmem;
	r = ctx->randrsl;
	a = b = c = d = e = f = g = h = 0x9e3779b9;
	for (i = 0; i < 4; ++i) {
		mix(a, b, c, d, e, f, g, h);
	}
	for (i = 0; i < RANDSIZ; i += 8) {
		a += r[i];
		b += r[i + 1];
		c += r[i + 2];
		d += r[i + 3];
		e += r[i + 4];
		f += r[i + 5];
		g += r[i + 6];
		h += r[i + 7];
		mix(a, b, c, d, e, f, g, h);
		m[i] = a;
		m[i + 1] = b;
		m[i + 2] = c;
		m[i + 3] = d;
		m[i + 4] = e;
		m[i + 5] = f;
		m[i + 6] = g;
		m[i + 7] = h;
	}
	for (i = 0; i < RANDSIZ; i += 8) {
		a += m[i];
		b += m[i + 1];
		c += m[i + 2];
		d += m[i + 3];
		e += m[i + 4];
		f += m[i + 5];
		g += m[i + 6];
		h += m[i + 7];
		mix(a, b, c, d, e, f, g, h);
		m[i] = a;
		m[i + 1] = b;
		m[i + 2] = c;
		m[i + 3] = d;
		m[i + 4] = e;
		m[i + 5] = f;
		m[i + 6] = g;
		m[i + 7] = h;
	}
	isaac(ctx);
	ctx->count = RANDSIZ;
}