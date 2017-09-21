//
//  NSBigInt.h
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSBigInt : NSObject

@property (nonatomic, assign) int user;
@property (nonatomic, assign) int sign;
@property (nonatomic, strong) NSMutableArray* array;

- (void)generateKeyPairPlease;

- (NSData *)encryptWithPublicKey;

- (void)decryptWithPrivateKey: (NSData *)dataToDecrypt;

@end
