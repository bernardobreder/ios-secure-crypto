#ifndef RSA_H
#define RSA_H

//int cryto_encode_size(mp_int* fn, mp_int* fe, unsigned char* message_plain, size_t len);
//
//void cryto_encode(mp_int* fn, mp_int* fe, unsigned char* message_plain, size_t message_plain_len, unsigned char* message_encoded);
//
//int cryto_decode_size(mp_int* fn, mp_int* fd, unsigned char* message_encoded, size_t len);
//
//void cryto_decode(mp_int* fp, mp_int* fq, mp_int* fd, unsigned char* message_encoded, size_t message_encoded_len, unsigned char* message_plain, size_t message_plain_len);

size_t cryto_encode_size(struct fp_int* fn, struct fp_int* fe, unsigned char* message_plain, size_t len);

void cryto_encode(struct fp_int* fn, struct fp_int* fe, unsigned char* message_plain, size_t message_plain_len, unsigned char* message_encoded);

size_t cryto_decode_size(struct fp_int* fn, struct fp_int* fd, unsigned char* message_encoded, size_t len);

void cryto_decode(struct fp_int* fp, struct fp_int* fq, struct fp_int* fd, unsigned char* message_encoded, size_t message_encoded_len, unsigned char* message_plain, size_t message_plain_len);

#endif
