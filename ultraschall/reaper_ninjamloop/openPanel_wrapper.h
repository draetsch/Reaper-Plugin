//
//  test.h
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 25.05.14.
//
//

#import <Cocoa/Cocoa.h>
#include "openPanel_connector.h"

@interface openPanel_wrapper : NSObject

+(const char*)foo: (importtype_t) importtype;
+(const char*)bar: (importtype_t) importtype;

@end