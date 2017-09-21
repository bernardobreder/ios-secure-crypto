//
//  NSBigInt.m
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import "NSBigInt.h"
#import "CommonCrypto/CommonCrypto.h"

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

typedef ulong32 fp_digit;
typedef ulong64 fp_word;
#define FP_MAX_SIZE           (2*2*4096+(8*DIGIT_BIT))
#define DIGIT_BIT  (int)((CHAR_BIT) * sizeof(fp_digit))
#define FP_MASK    (fp_digit)(-1)
#define FP_SIZE    (FP_MAX_SIZE/DIGIT_BIT)

@implementation NSBigInt

@synthesize user = _user;
@synthesize sign = _sign;
@synthesize array = _array;

- (id)init
{
    if (!(self = [super init])) return nil;
    _user = 0;
    _sign = 0;
    _array = [[NSMutableArray alloc] initWithCapacity:FP_SIZE];
    if (!_array) {
        return nil;
    }
    return self;
}

- (id)initForInteger:(NSInteger)value
{
    if (!(self = [super init])) return nil;
    _user = value == 0 ? 0 : 1;
    _sign = value >= 0 ? 0 : 1;
    _array = [[NSMutableArray alloc] initWithCapacity:2];
    if (!_array) {
        return nil;
    }
    _array[0] = [NSNumber numberWithInteger:value];
    return self;
}

- (id)initWithRadix:(NSString*)str withBase:(NSInteger)radix
{
	if (radix < 2 || radix > 64) {
		return nil;
	}    if (!(self = [self init])) return nil;
    int y, neg, i = 0;
	if ([str characterAtIndex:i] == '-') {
		i++;
		neg = 1;
	} else {
		neg = 0;
	}
    if (radix < 36 && radix != 10) {
        str = [str uppercaseString];
    }
    const NSString* fp_s_rmap = @"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";
    NSUInteger len = str.length;
	while (len) {
		unichar ch = [str characterAtIndex:i];
		for (y = 0; y < 64; y++) {
			if (ch == [fp_s_rmap characterAtIndex:y]) {
				break;
			}
		}
		if (y < radix) {
//			fp_mul_d(a, (fp_digit) radix, a);
//			fp_add_d(a, (fp_digit) y, a);
		} else {
			break;
		}
		i++;
	}
    if (![self isZero]) {
        _sign = neg;
    }
    return self;
}

- (bool)isZero
{
    return true;
    //return _user == 0;
}
//
///* c = a + b */
//void fp_add_d(struct fp_int *a, fp_digit b, struct fp_int *c) {
//	struct fp_int tmp;
//	fp_set(&tmp, b);
//	fp_add(a, &tmp, c);
//}
//
//void fp_set(struct fp_int *a, fp_digit b) {
//	fp_zero(a);
//	a->dp[0] = b;
//	a->used = a->dp[0] ? 1 : 0;
//}
//
///* c = a + b */
//void fp_add(struct fp_int *a, struct fp_int *b, struct fp_int *c) {
//	int sa, sb;
//	sa = a->sign;
//	sb = b->sign;
//	if (sa == sb) {
//		c->sign = sa;
//		s_fp_add(a, b, c);
//	} else {
//		if (fp_cmp_mag(a, b) == FP_LT) {
//			c->sign = sb;
//			s_fp_sub(b, a, c);
//		} else {
//			c->sign = sa;
//			s_fp_sub(a, b, c);
//		}
//	}
//}
//

#import <Foundation/Foundation.h>
#import <Security/Security.h>

static const UInt8 publicKeyIdentifier[] = "com.apple.sample.publickey\0";
static const UInt8 privateKeyIdentifier[] = "com.apple.sample.privatekey\0";
static SecKeyRef publicKey = NULL;
static SecKeyRef privateKey = NULL;
static unsigned char* sigBuffer;
static size_t sigBufferSize;

- (void)generateKeyPairPlease
{
    OSStatus status = noErr;
    NSMutableDictionary *privateKeyAttr = [[NSMutableDictionary alloc] init];
    NSMutableDictionary *publicKeyAttr = [[NSMutableDictionary alloc] init];
    NSMutableDictionary *keyPairAttr = [[NSMutableDictionary alloc] init];
    // 2
    
    NSData * publicTag = [NSData dataWithBytes:publicKeyIdentifier length:strlen((const char *)publicKeyIdentifier)];
    NSData * privateTag = [NSData dataWithBytes:privateKeyIdentifier length:strlen((const char *)privateKeyIdentifier)];
    // 3
    
//    SecKeyRef publicKey = NULL;
//    SecKeyRef privateKey = NULL;                                // 4
    
    [keyPairAttr setObject:(__bridge id)kSecAttrKeyTypeRSA forKey:(__bridge id)kSecAttrKeyType]; // 5
    [keyPairAttr setObject:[NSNumber numberWithInt:2048] forKey:(__bridge id)kSecAttrKeySizeInBits]; // 6
    
    [privateKeyAttr setObject:[NSNumber numberWithBool:YES] forKey:(__bridge id)kSecAttrIsPermanent]; // 7
    [privateKeyAttr setObject:privateTag forKey:(__bridge id)kSecAttrApplicationTag]; // 8
    
    [publicKeyAttr setObject:[NSNumber numberWithBool:YES] forKey:(__bridge id)kSecAttrIsPermanent]; // 9
    [publicKeyAttr setObject:publicTag forKey:(__bridge id)kSecAttrApplicationTag]; // 10
    
    [keyPairAttr setObject:privateKeyAttr forKey:(__bridge id)kSecPrivateKeyAttrs]; // 11
    [keyPairAttr setObject:publicKeyAttr forKey:(__bridge id)kSecPublicKeyAttrs]; // 12
    
    status = SecKeyGeneratePair((__bridge CFDictionaryRef)keyPairAttr, &publicKey, &privateKey); // 13
    //    error handling...
    
    if (status != noErr) {
        NSLog(@"Error Generate KeyPair");
        return;
    }
    
    {
        NSDate *methodStart = [NSDate date];
        /* Example credentials sent to the server */
        unsigned char *clearText = (uint8_t*)"username=USERNAME&password=PASSWORD";
        unsigned char cipherText[1024];
        size_t buflen = 1024;
        /* Encrypt: Done on the device */
        status = SecKeyEncrypt(publicKey, kSecPaddingPKCS1, clearText, strlen((char*)clearText) + 1,cipherText, &buflen);
        /* Decrypt: Done on the server */
        unsigned char decryptedText[buflen];
        status = SecKeyDecrypt(privateKey, kSecPaddingPKCS1, cipherText, buflen, decryptedText, &buflen);
        if (status != noErr) {
            NSLog(@"Error Generate Test");
            return;
        }
        NSLog(@"executionTime = %lf", [[NSDate date] timeIntervalSinceDate:methodStart]);
        NSLog(@"Test: %@", [NSString stringWithUTF8String:(char*)decryptedText]);
    }
    {
        NSDate *methodStart = [NSDate date];
        
        sleep(1);
        
        NSLog(@"executionTime = %lf", [[NSDate date] timeIntervalSinceDate:methodStart]);
    }
//    if(publicKey) CFRelease(publicKey);
//    if(privateKey) CFRelease(privateKey);                       // 14
}

- (NSData *)encryptWithPublicKey
{
    NSDate *methodStart = [NSDate date];
    OSStatus status = noErr;
    
    size_t cipherBufferSize;
    uint8_t *cipherBuffer;                     // 1
    
    // [cipherBufferSize]
    const uint8_t dataToEncrypt[] = "the quick brown fox jumps over the lazy dog\0"; // 2
    size_t dataLength = sizeof(dataToEncrypt)/sizeof(dataToEncrypt[0]);
    
//    SecKeyRef publicKey = NULL;                                 // 3
//    
//    NSData * publicTag = [NSData dataWithBytes:publicKeyIdentifier length:strlen((const char *)publicKeyIdentifier)]; // 4
//    
//    NSMutableDictionary *queryPublicKey =
//    [[NSMutableDictionary alloc] init]; // 5
//    
//    [queryPublicKey setObject:(__bridge id)kSecClassKey forKey:(__bridge id)kSecClass];
//    [queryPublicKey setObject:publicTag forKey:(__bridge id)kSecAttrApplicationTag];
//    [queryPublicKey setObject:(__bridge id)kSecAttrKeyTypeRSA forKey:(__bridge id)kSecAttrKeyType];
//    [queryPublicKey setObject:[NSNumber numberWithBool:YES] forKey:(__bridge id)kSecReturnRef];
//    // 6
//    
//    status = SecItemCopyMatching
//    ((__bridge CFDictionaryRef)queryPublicKey, (CFTypeRef *)&publicKey); // 7
    
    //  Allocate a buffer
    
    cipherBufferSize = SecKeyGetBlockSize(publicKey);
    cipherBuffer = malloc(cipherBufferSize);
    
    //  Error handling
    
    if (cipherBufferSize < sizeof(dataToEncrypt)) {
        // Ordinarily, you would split the data up into blocks
        // equal to cipherBufferSize, with the last block being
        // shorter. For simplicity, this example assumes that
        // the data is short enough to fit.
        printf("Could not decrypt.  Packet too large.\n");
        return NULL;
    }
    
    // Encrypt using the public.
    status = SecKeyEncrypt(publicKey,
                           kSecPaddingPKCS1,
                           dataToEncrypt,
                           (size_t) dataLength,
                           cipherBuffer,
                           &cipherBufferSize
                           );                              // 8
    
    //  Error handling
    //  Store or transmit the encrypted text
    
//    if (publicKey) CFRelease(publicKey);
    
    if (status != 0) {
        NSLog(@"Error Encrypting");
        return 0;
    }
    
    NSData *encryptedData = [NSData dataWithBytes:cipherBuffer length:cipherBufferSize];
    
    free(cipherBuffer);
    NSLog(@"Encrypt[%lf]", [[NSDate date] timeIntervalSinceDate:methodStart]);
    {
        sigBufferSize = SecKeyGetBlockSize(privateKey);
        sigBuffer = malloc(sigBufferSize);
        if (SecKeyRawSign(privateKey, kSecPaddingPKCS1, dataToEncrypt, dataLength, sigBuffer, &sigBufferSize) != noErr) {
            NSLog(@"Error Signing");
            return 0;
        }
        NSLog(@"Signed");
    }
    return encryptedData;
}

- (void)decryptWithPrivateKey: (NSData *)dataToDecrypt
{
    NSDate *methodStart = [NSDate date];
    OSStatus status = noErr;
    
    size_t cipherBufferSize = [dataToDecrypt length];
    uint8_t *cipherBuffer = (uint8_t *)[dataToDecrypt bytes];
    
    size_t plainBufferSize;
    uint8_t *plainBuffer;
    
//    SecKeyRef privateKey = NULL;
    
//    NSData * privateTag = [NSData dataWithBytes:privateKeyIdentifier
//                                         length:strlen((const char *)privateKeyIdentifier)];
//    
//    NSMutableDictionary *queryPrivateKey = [[NSMutableDictionary alloc] init];
//    
//    // Set the private key query dictionary.
//    [queryPrivateKey setObject:(__bridge id)kSecClassKey forKey:(__bridge id)kSecClass];
//    [queryPrivateKey setObject:privateTag forKey:(__bridge id)kSecAttrApplicationTag];
//    [queryPrivateKey setObject:(__bridge id)kSecAttrKeyTypeRSA forKey:(__bridge id)kSecAttrKeyType];
//    [queryPrivateKey setObject:[NSNumber numberWithBool:YES] forKey:(__bridge id)kSecReturnRef];
//    // 1
//    
//    status = SecItemCopyMatching((__bridge CFDictionaryRef)queryPrivateKey, (CFTypeRef *)&privateKey); // 2
    plainBufferSize = SecKeyGetBlockSize(privateKey);
    plainBuffer = malloc(plainBufferSize);
    if (plainBufferSize < cipherBufferSize) {
        printf("Could not decrypt.  Packet too large.\n");
        return;
    }
    status = SecKeyDecrypt(privateKey, kSecPaddingPKCS1, cipherBuffer, cipherBufferSize, plainBuffer, &plainBufferSize);
    if (status != 0) {
        NSLog(@"Error Decrypting");
        return;
    }
    NSLog(@"Decrypt[%lf]: %s", [[NSDate date] timeIntervalSinceDate:methodStart], plainBuffer);
    {
        if (SecKeyRawVerify(publicKey, kSecPaddingPKCS1, plainBuffer, plainBufferSize, sigBuffer, sigBufferSize) != noErr) {
            NSLog(@"Error with Verify Sign");
            return;
        }
        NSLog(@"Verified Sign");
    }
    if (1) {
        NSString *key = @"my password";
        NSString *secret = @"text to encrypt";
        
        NSData *plain = [secret dataUsingEncoding:NSUTF8StringEncoding];
        NSData *cipher = [plain AES256EncryptedDataUsingKey:key error:nil];
        printf("%s\n", [[cipher description] UTF8String]);
        
        plain = [cipher decryptedAES256DataUsingKey:key error:nil];
        printf("%s\n", [[plain description] UTF8String]);
        printf("%s\n", [[[NSString alloc] initWithData:plain encoding:NSUTF8StringEncoding] UTF8String]);
    }
}


@end

