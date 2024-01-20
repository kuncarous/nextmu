#import <Foundation/Foundation.h>
#include "shared_precompiled.h"
#include "mu_main.h"

void AppleLogToConsole(const mu_char *message)
{
	NSString* nsmessage = [NSString stringWithCString : message
		encoding : [NSString defaultCStringEncoding] ];

	NSLog(@"%@", nsmessage);
}

int main(int argc, char* argv[])
{
	Main();
	return 0;
}
