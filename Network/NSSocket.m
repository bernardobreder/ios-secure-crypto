//
//  NSSocket.m
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import "NSSocket.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

@interface NSSocket ()

@property (nonatomic, strong) NSString *host;
@property (nonatomic, assign) int port;
@property (nonatomic, assign) bool connected;
@property (nonatomic, assign) bool started;
@property (nonatomic, assign) int sock;
@property (nonatomic, assign) NSSocketState state;

@end

@implementation NSSocket

@synthesize host = _host;
@synthesize port = _port;
@synthesize sock = _sock;
@synthesize connected = _connected;
@synthesize started = _started;
@synthesize delegate = _delegate;
@synthesize state = _state;

- (id)initHost:(NSString*)host port:(int)port
{
    
    if (!(self = [super init])) return nil;
    _host = host;
    _port = port;
    _sock = -1;
    _connected = false;
    _state = NSSocketStateDisconnected;
    return self;
}

- (void)start
{
    if (!_started) {
        _started = true;
        [self performSelectorInBackground:@selector(_background:) withObject:nil];
    }
}

- (bool)connect
{
    if (_connected) {
        return false;
    }
    [self _fireStateChanged:NSSocketStateConnecting];
    if ((_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        [self _fireStateChanged:NSSocketStateDisconnected];
        return false;
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    const char* chost = [_host cStringUsingEncoding:NSASCIIStringEncoding];
    server.sin_addr.s_addr = inet_addr(chost);
    server.sin_port = htons(_port);
    if (connect(_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        [self close];
        return false;
    }
    _connected = true;
    [self _fireStateChanged:NSSocketStateConnected];
    return true;
}

- (void)close
{
    _connected = false;
    if (_state != NSSocketStateDisconnected) {
        close(_sock);
        _started = false;
        _sock = -1;
        [self _fireStateChanged:NSSocketStateDisconnected];
    }
}

- (bool)isConnected
{
    return _connected;
}

- (bool)send:(NSString*)message
{
    [self _fireStateChanged:NSSocketStateSending];
    const char *buffer = [message cStringUsingEncoding:NSUTF8StringEncoding];
    char *aux = (char*)buffer;
    NSUInteger length = message.length;
    for (;;) {
        ssize_t state = send(_sock, aux, length, 0);
        if (state <= 0) {
            [self close];
            return false;
        }
        if (state == length) {
            break;
        }
        length -= state;
        aux += state;
    }
    [self _fireStateChanged:NSSocketStateConnected];
    return true;
}

- (NSString*)receive
{
    [self _fireStateChanged:NSSocketStateReceiving];
    NSString *string = nil;
    unsigned char buffer[1024];
    ssize_t bytes;
    if ((bytes = recv(_sock, buffer, 1024, MSG_DONTWAIT)) > 0) {
        buffer[bytes] = 0;
        string = [NSString stringWithCString:(char*)buffer encoding:NSASCIIStringEncoding];
    }
    if (bytes == 0) {
        [self close];
        return nil;
    }
    if (string.length == 0) {
        [self _fireStateChanged:NSSocketStateConnected];
        return nil;
    }
    [self _fireStateChanged:NSSocketStateConnected];
    return string;
}

- (void)_fireStateChangedInEDT:(NSDictionary*)data
{
    NSSocketState state = [(NSNumber*)[data objectForKey:@"state"] intValue];
    [_delegate stateChanged:state];
}

- (void)_fireStateChanged:(NSSocketState)state
{
    if (_state != state) {
        _state = state;
        if (_delegate && [_delegate respondsToSelector:@selector(stateChanged:)]) {
            if ([[NSThread currentThread] isMainThread]) {
                [_delegate stateChanged:state];
            } else {
                NSMutableDictionary *data = [[NSMutableDictionary alloc] init];
                [data setObject:[NSNumber numberWithInt:state] forKey:@"state"];
                [self performSelectorOnMainThread:@selector(_fireStateChangedInEDT:) withObject:data waitUntilDone:NO];
            }
        }
    }
}

- (void)_fireMessageInEDT:(NSDictionary*)data
{
    NSString *message = (NSString*)[data objectForKey:@"message"];
    [_delegate receiveMessage:message];
}

- (void)_fireMessage:(NSString*)message
{
    if (_delegate && [_delegate respondsToSelector:@selector(receiveMessage:)]) {
        if ([[NSThread currentThread] isMainThread]) {
            [_delegate receiveMessage:message];
        } else {
            NSMutableDictionary *data = [[NSMutableDictionary alloc] init];
            [data setObject:message forKey:@"message"];
            [self performSelectorOnMainThread:@selector(_fireMessageInEDT:) withObject:data waitUntilDone:NO];
        }
    }
}

- (void)_background:(NSDictionary*)dataMap
{
    [self connect];
    while ([self isConnected]) {
        [self send:@"Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!Hello My World!"];
        NSString *message = [self receive];
        if (message) {
            [self performSelectorOnMainThread:@selector(_fireMessage:) withObject:message waitUntilDone:true];
        }
        sleep(1);
    }
    [self _fireStateChanged:NSSocketStateDisconnected];
}

@end
