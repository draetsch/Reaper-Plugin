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

+(void) bar: (importtype_t) importtype
{
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    NSArray* types = [[NSArray alloc] initWithObjects:@"osf", @"mp4chaps", nil];
    [savePanel allowedFileTypes:types];
    
    NSInteger fgfgf = [savePanel runModal];
    id hhhhh = 0;
}

+(const char *) foo: (importtype_t) importtype
{

    NSOpenPanel* p = [NSOpenPanel openPanel];
    NSArray *types;
    
    if(importtype == SHOWNOTES)
        types = [[NSArray alloc] initWithObjects:@"osf", @"txt", @"OSF", @"TXT", nil];
    
    if(importtype == CHAPTERS)
        types = [[NSArray alloc] initWithObjects:@"mp4chaps", @"txt", @"MP4CHAPS", @"TXT", nil];
    
    [p setCanChooseDirectories:NO];
    [p setCanChooseFiles:YES];
    [p setAllowsMultipleSelection:NO];
    [p setAllowedFileTypes:types];
    
    if([p runModalForTypes:types] == NSOKButton)
    {
        return [[[p URLs] firstObject] fileSystemRepresentation];
    }
    
    return NULL;
}

@end

void savePanel(importtype_t importtype)
{
    [openPanel_wrapper bar: importtype];
}

char* openPanel(importtype_t importtype)
{
    
    const char* r = [openPanel_wrapper foo: importtype];
    if(!r)
        return NULL;
    
    return (char*)r;
    
}