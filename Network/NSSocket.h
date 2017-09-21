//
//  NSSocket.h
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

typedef enum NSSocketState
{
    NSSocketStateDisconnected,
    NSSocketStateConnected,
    NSSocketStateConnecting,
    NSSocketStateSending,
    NSSocketStateReceiving,
} NSSocketState;

@protocol NSSocketDelegate <NSObject>

@optional

- (void)stateChanged:(NSSocketState)state;

- (void)receiveMessage:(NSString*)message;

@end

@interface NSSocket : NSObject

@property (nonatomic, weak) id<NSSocketDelegate> delegate;

- (id)initHost:(NSString*)host port:(int)port;

- (void)start;

- (bool)connect;

- (bool)isConnected;

- (void)close;

- (bool)send:(NSString*)message;

- (NSString*)receive;

@end
