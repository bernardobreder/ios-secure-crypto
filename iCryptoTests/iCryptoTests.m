//
//  iCryptoTests.m
//  iCryptoTests
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "NSBigInt.h"

@interface iCryptoTests : XCTestCase

@end

@implementation iCryptoTests

- (void)setUp
{
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)test
{
    NSBigInt* m = [[NSBigInt alloc] init];
    [m generateKeyPairPlease];
    NSData *data = [m encryptWithPublicKey];
    [m decryptWithPrivateKey:data];
}

@end
