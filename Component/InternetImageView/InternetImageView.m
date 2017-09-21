//
//  InternetImageView.m
//  iCrypto
//
//  Created by Bernardo Breder on 05/04/14.
//  Copyright (c) 2014 Bernardo Breder. All rights reserved.
//

#import "InternetImageView.h"

@implementation InternetImageView

- (void)drawRect:(CGRect)rect
{
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, [UIColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0].CGColor);
    CGContextFillEllipseInRect(context, rect);
    CGContextSetFillColorWithColor(context, [UIColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0].CGColor);
    CGContextFillEllipseInRect(context, CGRectMake(rect.origin.x + 3, rect.origin.y + 3, rect.size.width - 6, rect.size.height - 6));
}

@end
