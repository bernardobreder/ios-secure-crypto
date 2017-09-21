#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "bigint.h"
#include "rand.h"

size_t cryto_encode_size(struct fp_int* fn, struct fp_int* fe, unsigned char* message_plain, size_t len) {
	register size_t result = 8;
	register int fp_size = fp_unsigned_bin_size(fn);
	result += (len / fp_size) * fp_size;
	if (len % fp_size != 0) {
		result += fp_size;
	}
	return result;
}

void cryto_encode(struct fp_int* fn, struct fp_int* fe, unsigned char* message_plain, size_t message_plain_len, unsigned char* message_encoded) {
	*message_encoded++ = (message_plain_len >> 56) & 0xFF;
	*message_encoded++ = (message_plain_len >> 48) & 0xFF;
	*message_encoded++ = (message_plain_len >> 40) & 0xFF;
	*message_encoded++ = (message_plain_len >> 32) & 0xFF;
	*message_encoded++ = (message_plain_len >> 24) & 0xFF;
	*message_encoded++ = (message_plain_len >> 16) & 0xFF;
	*message_encoded++ = (message_plain_len >> 8) & 0xFF;
	*message_encoded++ = (message_plain_len >> 0) & 0xFF;
	struct fp_int fc;
	int fn_size = fp_unsigned_bin_size(fn);
	while (message_plain_len > 0) {
		fp_init(&fc);
		int max = message_plain_len < fn_size ? (int) message_plain_len : fn_size;
		fp_read_unsigned_bin(&fc, message_plain, max);
		fp_exptmod(&fc, fe, fn, &fc);
		fp_to_unsigned_bin(&fc, message_encoded);
		message_encoded += fn_size;
		message_plain_len -= max;
		message_plain += max;
	}
}

size_t cryto_decode_size(struct fp_int* fn, struct fp_int* fd, unsigned char* message_encoded, size_t len) {
	register size_t size = 0;
	size += (size_t) *message_encoded++ << 56;
	size += (size_t) *message_encoded++ << 48;
	size += (size_t) *message_encoded++ << 40;
	size += (size_t) *message_encoded++ << 32;
	size += *message_encoded++ << 24;
	size += *message_encoded++ << 16;
	size += *message_encoded++ << 8;
	size += *message_encoded++ << 0;
	return size;
}

void cryto_decode(struct fp_int* fp, struct fp_int* fq, struct fp_int* fd, unsigned char* message_encoded, size_t message_encoded_len, unsigned char* message_plain, size_t message_plain_len) {
	struct fp_int fp1, fq1, fn;
	struct fp_int fc, fdp, fdq, fqinv, fm1, fm2, fh, faux;
    fp_init(&fn);
    fp_mul(fp, fq, &fn);
	int fn_size = fp_unsigned_bin_size(&fn);
	fp_init(&fp1);
	fp_init(&fq1);
	fp_sub_d(fp, 1, &fp1);
	fp_sub_d(fq, 1, &fq1);
	message_encoded_len -= 8;
	message_encoded += 8;
	while (message_encoded_len > 0) {
		fp_read_unsigned_bin(&fc, message_encoded, fn_size);
        fp_mod(fd, &fp1, &fdp);
        fp_mod(fd, &fq1, &fdq);
        fp_invmod(fq, fp, &fqinv);
        fp_exptmod(&fc, &fdp, fp, &fm1);
        fp_exptmod(&fc, &fdq, fq, &fm2);
        fp_sub(&fm1, &fm2, &faux);
        fp_mul(&fqinv, &faux, &faux);
        fp_mod(&faux, fp, &fh);
        fp_mul(&fh, fq, &faux);
        fp_add(&fm2, &faux, &fc);
		fp_to_unsigned_bin(&fc, message_plain);
		message_encoded_len -= fn_size;
		message_encoded += fn_size;
        message_plain += fn_size;
	}
}


