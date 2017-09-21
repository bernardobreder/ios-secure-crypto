//#include <CoreFoundation/CoreFoundation.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "bigint.h"
//#include "tommath.h"
#include "rsa.h"
#include "aes256.h"

#define DUMP(s, i, buf, sz)  {printf(s);                   \
for (i = 0; i < (sz);i++)    \
printf("%02x ", buf[i]); \
printf("\n");}


int main_tmp() {
	struct fp_int fp, fq, fn, fm, fp1, fq1, fe, fd;
    fp_init(&fe);
	fp_init(&fp1);
    fp_init(&fq1);
    fp_init(&fn);
    fp_init(&fm);
    fp_init(&fd);
	fp_read_radix(&fp,
			"25229625325077476312832432659556027725044335816615551027187743260839847906658551790655873981384551929838648803911303368486339439087285296902444259342160492793117679999622848363651641655022242062552137989288156571408985091618158154492394690386494512756914150033285393276767976572063220707297472506846864800087837717215769325804540384596358175044870731930145662684072548930330370574146364257851180790244361026737798960718504955069665601590638636599446764430550891681570975651533814798740253909597483133129489058289369677543331240173141743910736599991856107650198504684324620362627176073636764351205219271370607440127909",
			10);
	fp_read_radix(&fq,
                  "19071407465413413222846839467120877488325270658957102482541227059481609488545123596530001809319834240801198216586451675582151152277645512456106603175424599277356264323098518000166658057272711326816849503315976843931305320577782428788266492329413411455154061927057267521058154663367359726871164313491518060068170528473795872561595601324443369214461536921966692405226661157538373289168621321308227720539582684880248712944523697170917631687984743323561628230271249152589664322743789786397397403229303527566053699127741794931939912856330891427006534034772976132490656282561919879235435888786545628943647413618796311049977",
                  10);
	fp_read_radix(&fe, "65537", 10);
	fp_sub_d(&fp, 1, &fp1);
	fp_sub_d(&fq, 1, &fq1);
	fp_mul(&fp, &fq, &fn);
	fp_mul(&fp1, &fq1, &fm);
	fp_invmod(&fe, &fm, &fd);
	unsigned char* msg = (unsigned char*) "Hello";
	size_t len = strlen((char*) msg);
	int n;
	for (n = 0;; n++) {
        {
            size_t msg_encode_len = cryto_encode_size(&fn, &fe, (unsigned char*) msg, len);
            unsigned char msg_encode[msg_encode_len];
            cryto_encode(&fn, &fe, (unsigned char*) msg, len, msg_encode);
            size_t msg_plain_len = cryto_decode_size(&fn, &fd, msg_encode, msg_encode_len);
            unsigned char msg_plain[msg_plain_len + 1];
            msg_plain[msg_plain_len] = 0;
            cryto_decode(&fp, &fq, &fd, msg_encode, msg_encode_len, msg_plain, msg_plain_len);
            printf("%d. Encrypted Public Key: %s\n", n, msg_plain);
        }
        {
            size_t msg_encode_len = cryto_encode_size(&fn, &fd, (unsigned char*) msg, len);
            unsigned char msg_encode[msg_encode_len];
            cryto_encode(&fn, &fe, (unsigned char*) msg, len, msg_encode);
            size_t msg_plain_len = cryto_decode_size(&fn, &fd, msg_encode, msg_encode_len);
            unsigned char msg_plain[msg_plain_len + 1];
            msg_plain[msg_plain_len] = 0;
            cryto_decode(&fp, &fq, &fd, msg_encode, msg_encode_len, msg_plain, msg_plain_len);
            printf("%d. Encrypted Private Key: %s\n", n, msg_plain);
        }
        {
            int fn_size = fp_unsigned_bin_size(&fn);
            char* message_plain = "Hello";
            unsigned char other_message_plain[fn_size];
            unsigned long message_plain_len = strlen(message_plain);
            unsigned char message_encoded[fn_size];
            struct fp_int fc, fm;
            {
                fp_init(&fc);
                int max = message_plain_len < fn_size ? (int) message_plain_len : fn_size;
                fp_read_unsigned_bin(&fc, (unsigned char*)message_plain, max);
                fp_exptmod(&fc, &fe, &fn, &fc);
                fp_to_unsigned_bin(&fc, message_encoded);
                
            }
            {
                fp_init(&fm);
                fp_exptmod(&fc, &fd, &fn, &fm);
                fp_to_unsigned_bin(&fm, other_message_plain);
                other_message_plain[message_plain_len] = 0;
            }
            printf("%d. Encrypted Public -> Private Key: %s\n", n, other_message_plain);
        }
        {
            int fn_size = fp_unsigned_bin_size(&fn);
            char* message_plain = "Hello";
            unsigned char other_message_plain[fn_size];
            unsigned long message_plain_len = strlen(message_plain);
            unsigned char message_encoded[fn_size];
            struct fp_int fc, fm;
            {
                fp_init(&fc);
                int max = message_plain_len < fn_size ? (int) message_plain_len : fn_size;
                fp_read_unsigned_bin(&fc, (unsigned char*)message_plain, max);
                fp_exptmod(&fc, &fd, &fn, &fc);
                fp_to_unsigned_bin(&fc, message_encoded);
                
            }
            {
                fp_init(&fm);
                fp_exptmod(&fc, &fe, &fn, &fm);
                fp_to_unsigned_bin(&fm, other_message_plain);
                other_message_plain[message_plain_len] = 0;
            }
            printf("%d. Encrypted Private -> Public Key: %s\n", n, other_message_plain);
        }
        {
            aes256_context ctx;
            uint8_t key[32];
            uint8_t buf[16], i;
            
            /* put a test vector */
            for (i = 0; i < sizeof(buf);i++) buf[i] = i * 16 + i;
            for (i = 0; i < sizeof(key);i++) key[i] = i;
            
            DUMP("txt: ", i, buf, sizeof(buf));
            DUMP("key: ", i, key, sizeof(key));
            printf("---\n");
            
            aes256_init(&ctx, key);
            aes256_encrypt_ecb(&ctx, buf);
            
            DUMP("enc: ", i, buf, sizeof(buf));
            printf("tst: 8e a2 b7 ca 51 67 45 bf ea fc 49 90 4b 49 60 89\n");
            
            aes256_init(&ctx, key);
            aes256_decrypt_ecb(&ctx, buf);
            DUMP("dec: ", i, buf, sizeof(buf));
            
            aes256_done(&ctx);
        }
	}
}

//int main() {
//	mp_int fp, fq, fn, fm, fp1, fq1, fe, fd;
//    mp_init(&fp);
//	mp_init(&fp1);
//    mp_init(&fq);
//    mp_init(&fq1);
//    mp_init(&fq);
//    mp_init(&fe);
//    mp_init(&fn);
//    mp_init(&fm);
//	mp_read_radix(&fp,
//			"25229625325077476312832432659556027725044335816615551027187743260839847906658551790655873981384551929838648803911303368486339439087285296902444259342160492793117679999622848363651641655022242062552137989288156571408985091618158154492394690386494512756914150033285393276767976572063220707297472506846864800087837717215769325804540384596358175044870731930145662684072548930330370574146364257851180790244361026737798960718504955069665601590638636599446764430550891681570975651533814798740253909597483133129489058289369677543331240173141743910736599991856107650198504684324620362627176073636764351205219271370607440127909",
//			10);
//	mp_read_radix(&fq,
//			"19071407465413413222846839467120877488325270658957102482541227059481609488545123596530001809319834240801198216586451675582151152277645512456106603175424599277356264323098518000166658057272711326816849503315976843931305320577782428788266492329413411455154061927057267521058154663367359726871164313491518060068170528473795872561595601324443369214461536921966692405226661157538373289168621321308227720539582684880248712944523697170917631687984743323561628230271249152589664322743789786397397403229303527566053699127741794931939912856330891427006534034772976132490656282561919879235435888786545628943647413618796311049977",
//			10);
//	mp_read_radix(&fe, "65537", 10);
//	mp_sub_d(&fp, 1, &fp1);
//	mp_sub_d(&fq, 1, &fq1);
//	mp_mul(&fp, &fq, &fn);
//	mp_mul(&fp1, &fq1, &fm);
//	mp_invmod(&fe, &fm, &fd);
//	unsigned char* msg = (unsigned char*) "Hello";
//	size_t len = strlen((char*) msg);
//	int n;
//    {
//        unsigned char buffer[512];
//        mp_int text_mp_int;
//        mp_init(&text_mp_int);
//        mp_read_unsigned_bin(&text_mp_int, "Heelo", <#int c#>)
//        mp_read_radix(&text_mp_int, "Hello", 10);
//        mp_to_unsigned_bin(&text_mp_int, buffer);
//        
//    }
//	if (0) {
//		size_t msg_encode_len = cryto_encode_size(&fn, &fe, (unsigned char*) msg, len);
//		unsigned char msg_encode[msg_encode_len];
//		cryto_encode(&fn, &fe, (unsigned char*) msg, len, msg_encode);
//		size_t msg_plain_len = cryto_decode_size(&fn, &fd, msg_encode, msg_encode_len);
//		unsigned char msg_plain[msg_plain_len + 1];
//		msg_plain[msg_plain_len] = 0;
//		cryto_decode(&fp, &fq, &fd, msg_encode, msg_encode_len, msg_plain, msg_plain_len);
//		printf("%d. %s\n", n, msg_plain);
//	}
//}
