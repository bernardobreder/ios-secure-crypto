#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "tommath.h"
#include "rand.h"

int cryto_encode_size(mp_int* fn, mp_int* fe, unsigned char* message_plain, size_t len) {
	register size_t result = 4;
	register int mp_size = mp_unsigned_bin_size(fn);
	result += (len / mp_size) * mp_size;
	if (len % mp_size != 0) {
		result += mp_size;
	}
	return result;
}

void cryto_encode(mp_int* fn, mp_int* fe, unsigned char* message_plain, size_t message_plain_len, unsigned char* message_encoded) {
	*message_encoded++ = (message_plain_len >> 24) & 0xFF;
	*message_encoded++ = (message_plain_len >> 16) & 0xFF;
	*message_encoded++ = (message_plain_len >> 8) & 0xFF;
	*message_encoded++ = (message_plain_len >> 0) & 0xFF;
	mp_int fc;
	int fn_size = mp_unsigned_bin_size(fn);
	while (message_plain_len > 0) {
		mp_init(&fc);
		int max = message_plain_len < fn_size ? (int) message_plain_len : fn_size;
		mp_read_unsigned_bin(&fc, message_plain, max);
		mp_exptmod(&fc, fe, fn, &fc);
		mp_to_unsigned_bin(&fc, message_encoded);
		message_encoded += fn_size;
		message_plain_len -= max;
		message_plain += max;
	}
}

int cryto_decode_size(mp_int* fn, mp_int* fd, unsigned char* message_encoded, size_t len) {
	register int size = 0;
	size += *message_encoded++ << 24;
	size += *message_encoded++ << 16;
	size += *message_encoded++ << 8;
	size += *message_encoded++ << 0;
	return size;
}

void cryto_decode(mp_int* fp, mp_int* fq, mp_int* fd, unsigned char* message_encoded, size_t message_encoded_len, unsigned char* message_plain, size_t message_plain_len) {
	mp_int fp1, fq1, fn;
	mp_int fc, fdp, fdq, fqinv, fm1, fm2, fh, faux;
    mp_init(&fn);
    mp_mul(fp, fq, &fn);
	int fn_size = mp_unsigned_bin_size(&fn);
	mp_init(&fp1);
	mp_init(&fq1);
	mp_sub_d(fp, 1, &fp1);
	mp_sub_d(fq, 1, &fq1);
	message_encoded_len -= 4;
	message_encoded += 4;
	while (message_encoded_len > 0) {
		mp_read_unsigned_bin(&fc, message_encoded, fn_size);
        mp_mod(fd, &fp1, &fdp);
        mp_mod(fd, &fq1, &fdq);
        mp_invmod(fq, fp, &fqinv);
        mp_exptmod(&fc, &fdp, fp, &fm1);
        mp_exptmod(&fc, &fdq, fq, &fm2);
        mp_sub(&fm1, &fm2, &faux);
        mp_mul(&fqinv, &faux, &faux);
        mp_mod(&faux, fp, &fh);
        mp_mul(&fh, fq, &faux);
        mp_add(&fm2, &faux, &fc);
		mp_to_unsigned_bin(&fc, message_plain);
		message_encoded_len -= fn_size;
		message_encoded += fn_size;
        message_plain += fn_size;
	}
}


