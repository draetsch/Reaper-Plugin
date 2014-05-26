//
//  test.m
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 25.05.14.
//
//

#import "openPanel_wrapper.h"
#import "openPanel_connector.h"

@implementation openPanel_wrapper

+(void) foo
{

    NSOpenPanel* p = [NSOpenPanel openPanel];
    
    NSArray *types = [[NSArray alloc] initWithObjects:@"*.osf", @"*.txt", nil];
    
    //[p delegate: self];
    [p setCanChooseDirectories:NO];
    [p setCanChooseFiles:YES];
    [p setAllowsMultipleSelection:NO];
    [p setAllowsOtherFileTypes:NO];
    [p setAllowedFileTypes:types];
    
    if([p runModal] == NSOKButton)
    {
        int zzz = 0;
    }
}

-(BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename
{
    NSString* ext = [filename pathExtension];
    if ([ext  isEqual: @""] || [ext  isEqual: @"/"] || ext == nil || ext == NULL || [ext length] < 1) {
        return TRUE;
    }
    
    NSLog(@"Ext: '%@'", ext);
    
    NSEnumerator* tagEnumerator = [[NSArray arrayWithObjects:@"osf", @"txt", nil] objectEnumerator];
    NSString* allowedExt;
    while ((allowedExt = [tagEnumerator nextObject]))
    {
        if ([ext caseInsensitiveCompare:allowedExt] == NSOrderedSame)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

@end

void openPanel()
{
    [openPanel_wrapper foo];
}