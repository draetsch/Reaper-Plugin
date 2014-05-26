//
//  reaper.h
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 04.05.14.
//
//

#ifndef reaper_Ultraschall_reaper_h
#define reaper_Ultraschall_reaper_h

#include "../reaper_plugin.h"
#include <iostream>
#include <string>
#include <vector>

extern void *(*GetSetMediaTrackInfo)(MediaTrack *tr, const char *parmname, void *setNewValue);
extern void* (*GetSetMediaItemInfo)(MediaItem* item, const char* parmname, void* setNewValue);
double (*GetMediaItemInfo_Value)(MediaItem* item, const char* parmname);
extern double (*parse_timestr)(const char* buf);
extern MediaItem* (*GetSelectedMediaItem)(ReaProject* proj, int selitem);
extern MediaTrack* (*GetTrack)(ReaProject* proj, int trackidx);
extern bool (*GetUserFileNameForRead)(char* filenameNeed4096, const char* title, const char* defext);
extern MediaItem* (*AddMediaItemToTrack)(MediaTrack* tr);
extern bool (*SetMediaItemPosition)(MediaItem* item, double position, bool refreshUI);
extern bool (*SetMediaItemLength)(MediaItem* item, double length, bool refreshUI);
extern bool (*SetProjectMarker3)(ReaProject* proj, int markrgnindexnumber, bool isrgn, double pos, double rgnend, const char* name, int color);
extern bool (*SetProjectMarker)(int markrgnindexnumber, bool isrgn, double pos, double rgnend, const char* name);
extern int (*GetNumTracks)();
extern void (*InsertTrackAtIndex)(int idx, bool wantDefaults);
extern int (*AddProjectMarker2)(ReaProject* proj, bool isrgn, double pos, double rgnend, const char* name, int wantidx, int color);
extern void (*UpdateArrange)();
extern void (*UpdateTimeline)();
extern int (*CountProjectMarkers)(ReaProject* proj, int* num_markersOut, int* num_regionsOut);
extern int (*EnumProjectMarkers)(int idx, bool* isrgnOut, double* posOut, double* rgnendOut, const char** nameOut, int* markrgnindexnumberOut);
extern void (*format_timestr_pos)(double tpos, char* buf, int buf_sz, int modeoverride);
extern ReaProject* (*EnumProjects)(int idx, char* projfn, int projfn_sz);
extern int (*ShowMessageBox)(const char* msg, const char* title, int type);
extern MediaItem* (*GetTrackMediaItem)(MediaTrack* tr, int itemidx);
extern int (*CountTrackMediaItems)(MediaTrack* track);
extern PCM_sink* (*PCM_Sink_Create)(const char* filename, const char* cfg, int cfg_sz, int nch, int srate, bool buildpeaks);

//////////////////////////////////////////////////////////////

MediaTrack* getTrackByName(char* trackName)
{
    
    for (int i = 0; i < GetNumTracks(); ++i) {
        
        MediaTrack* track = GetTrack(0,i);
        
        char* currentTrackName = (char*)GetSetMediaTrackInfo(track, "P_NAME", NULL);
        
        if(strcmp(currentTrackName, trackName) == 0)
        {
            free(currentTrackName);
            return track;
        }
        
        
    }
    
    
    return NULL;
    
}

struct time
{
    int s, m, h = NULL;
    std::string ms = "";
};

std::string _format_time(char* aTime)
{
    char * pch;
    pch = std::strtok(aTime, ":");
    std::string bbb = std::string(aTime);
    std::vector<std::string> items;
    
    while(pch != NULL)
    {
        items.push_back(pch);
        pch = std::strtok(NULL, ":");
    }
    
    
    std::reverse(items.begin(), items.end());
    
    struct time times;
    
    times.s =     times.m =     times.h = 0;
    
    times.ms = items[0].substr(items[0].find("."), std::string::npos).c_str();
    items[0].erase(items[0].find("."), std::string::npos);
    
    
    
    for (int i = 0; i < items.size(); ++i) {
        switch (i) {
            case 0:
                times.s = std::atoi(items[0].c_str());
                break;
            case 1:
                times.m = std::atoi(items[1].c_str());
                break;
            case 2:
                times.h = std::atoi(items[2].c_str());
                break;
                
            default:
                break;
        }
    }
    
    char buffer[13];
    
    sprintf(buffer, "%02d:%02d:%02d%s", times.h, times.m, times.s, times.ms.c_str());
    
    return std::string(buffer);
    
}


#endif
