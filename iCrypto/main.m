//
//  main.m
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFSIZE 32

int main(int argc, char * argv[])
{
    @autoreleasepool {
//        int sock;
//        struct sockaddr_in echoserver;
//        char buffer[BUFFSIZE];
//        unsigned int echolen;
//        int received = 0;
//        int bytes = 0;
//        //        struct hostent *hostent = gethostbyname("breder.org");
//        /* Create the TCP socket */
//        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
//            return 1;
//        }
//        struct in_addr inaddr;
//        inaddr.s_addr = inet_addr("breder.org");
//        struct hostent *he = gethostbyname("breder.org");
//        char *ip = he->h_addr_list[0];
//        /* Construct the server sockaddr_in structure */
//        memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
//        echoserver.sin_family = AF_INET;                  /* Internet/IP */
//        echoserver.sin_addr.s_addr = inet_addr("212.1.213.140");
//        echoserver.sin_port = htons(9090);       /* server port */
//        for (;;) {
//            /* Establish connection */
//            if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
//                return 1;
//            }
//            /* Send the word to the server */
//            char *msg = "Hello";
//            echolen = strlen(msg);
//            for (;;) {
//                if (send(sock, msg, echolen, 0) != echolen) {
//                    return 1;
//                }
//                if((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) > 0) {
//                    buffer[bytes] = '\0';
//                    printf("%s", buffer);
//                }
//                printf("\n");
//            }
//            close(sock);
//        }
//        NSSocket *socket = [[NSSocket alloc] initHost:@"212.1.213.140" port:9090];
//        if([socket connect]) {
//            while ([socket isConnected]) {
//                [socket send:@"Hello!"];
//                NSString *msg = [socket receive];
//                if (msg) {
//                    NSLog(@"Message: %@", msg);
//                }
//            }
//        }
//        return 0;
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
