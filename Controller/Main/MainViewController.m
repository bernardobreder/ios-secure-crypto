//
//  MainViewController.m
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import "MainViewController.h"

@interface MainViewController ()

@property (nonatomic, strong) NSSocket *socket;
@property (nonatomic, assign) NSInteger counter;
@property (nonatomic, strong) UIButton *startButton;
@property (nonatomic, strong) UILabel *stateLabel;

@end

@implementation MainViewController

@synthesize socket;
@synthesize counter;
@synthesize startButton;
@synthesize stateLabel;

- (void)loadView
{
    self.view = [[UIView alloc] init];
    {
        startButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        [startButton setTitle:@"Start" forState:UIControlStateNormal];
        [startButton setTitle:@"Stop" forState:UIControlStateSelected];
        [startButton setTitleColor:[UIColor blueColor] forState:UIControlStateNormal];
        [startButton setTitleColor:[UIColor blueColor] forState:UIControlStateSelected];
        [startButton setTitleColor:[UIColor blueColor] forState:UIControlStateDisabled];
        [startButton addTarget:self action:@selector(onStartButton:) forControlEvents:UIControlEventTouchUpInside];
        [self.view addSubview:startButton];
        CONSTRAINT4(startButton, self.view, NSLayoutAttributeLeft, 1.0, 10, NSLayoutAttributeRight, 1.0, -20, NSLayoutAttributeTop, 1.0, 40, NSLayoutAttributeBottom, 1.0, -40);
    }
    {
        stateLabel = [[UILabel alloc] init];
        stateLabel.textAlignment = NSTextAlignmentCenter;
        stateLabel.text = @"#";
        [self.view addSubview:stateLabel];
        CONSTRAINT4(stateLabel, self.view, NSLayoutAttributeLeft, 1.0, 0, NSLayoutAttributeRight, 1.0, 0, NSLayoutAttributeTop, 1.0, 20, NSLayoutAttributeHeight, 0.0, 20);
    }
    [self initNetworkCommunication];
}

- (void)initNetworkCommunication {
    socket = [[NSSocket alloc] initHost:@"212.1.213.140" port:9090];
    socket.delegate = self;
}

- (void)onStartButton:(UIButton*)button
{
    if ([button isSelected]) {
        [socket close];
    } else {
        counter = 0;
        [startButton setTitle:@"Stop" forState:UIControlStateSelected];
        [socket start];
    }
    [button setSelected:![button isSelected]];
}

- (void)stateChanged:(NSSocketState)state
{
    if (1) {
        switch (state) {
            case NSSocketStateDisconnected:
                stateLabel.text = @"Disconnected";
                counter = 0;
                [startButton setTitle:@"Stop" forState:UIControlStateSelected];
                startButton.enabled = true;
                break;
            case NSSocketStateConnecting:
                stateLabel.text = @"Connecting";
                startButton.enabled = false;
                break;
            case NSSocketStateConnected:
                stateLabel.text = @"Connected";
                startButton.enabled = true;
                break;
            case NSSocketStateSending:
                stateLabel.text = @"Sending";
                startButton.enabled = true;
                break;
            case NSSocketStateReceiving:
                stateLabel.text = @"Receiving";
                startButton.enabled = true;
                break;
            default:
                stateLabel.text = @"Unknown";
                startButton.enabled = true;
                break;
        }
    }
}

- (void)receiveMessage:(NSString*)message
{
    if ([startButton isSelected]) {
        counter++;
        [startButton setTitle:[NSString stringWithFormat:@"Stop %d", counter] forState:UIControlStateSelected];
    }
    NSLog(@"Receive: %@", message);
}

@end
