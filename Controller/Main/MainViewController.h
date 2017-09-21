//
//  MainViewController.h
//  iCrypto
//
//  Created by Bernardo Breder on 04/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MainViewController : UIViewController <NSSocketDelegate>

@property (strong) NSInputStream *inputStream;
@property (strong) NSOutputStream *outputStream;

@end
