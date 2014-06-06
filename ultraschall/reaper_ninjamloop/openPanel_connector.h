//
//  openPanel_connector.h
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 26.05.14.
//
//

#ifndef reaper_Ultraschall_openPanel_connector_h
#define reaper_Ultraschall_openPanel_connector_h

typedef enum {CHAPTERS, SHOWNOTES} importtype_t;

char* openPanel(importtype_t);
void savePanel(importtype_t);

#endif
