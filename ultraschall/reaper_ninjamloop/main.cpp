#ifdef _WIN32
#include <windows.h>
#else
#include "../../WDL/swell/swell.h"
#endif

#include <stdio.h>
#include <math.h>

#include "resource.h"

#include "../reaper_plugin.h"

#include "../../WDL/queue.h"
#include "../../WDL/jnetlib/jnetlib.h"
#include "../../WDL/jnetlib/httpget.h"
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
//#include "reaper_plugin_functions.h"


void *(*GetSetMediaTrackInfo)(MediaTrack *tr, const char *parmname, void *setNewValue);
void* (*GetSetMediaItemInfo)(MediaItem* item, const char* parmname, void* setNewValue);
double (*parse_timestr)(const char* buf);
MediaItem* (*GetSelectedMediaItem)(ReaProject* proj, int selitem);
MediaTrack* (*GetTrack)(ReaProject* proj, int trackidx);
bool (*GetUserFileNameForRead)(char* filenameNeed4096, const char* title, const char* defext);
MediaItem* (*AddMediaItemToTrack)(MediaTrack* tr);
bool (*SetMediaItemPosition)(MediaItem* item, double position, bool refreshUI);
bool (*SetMediaItemLength)(MediaItem* item, double length, bool refreshUI);
bool (*SetProjectMarker3)(ReaProject* proj, int markrgnindexnumber, bool isrgn, double pos, double rgnend, const char* name, int color);
int (*GetNumTracks)();
void (*InsertTrackAtIndex)(int idx, bool wantDefaults);

int g_registered_command=0;
int g_registered_command_01=1;


REAPER_PLUGIN_HINSTANCE g_hInst;
int (*InsertMedia)(char *file, int mode); // mode: 0=add to current track, 1=add new track
HWND (*GetMainHwnd)();
void (*GetProjectPath)(char *buf, int bufsz);
gaccel_register_t acreg=
{
  {FALT|FVIRTKEY,'5',0},
  "Insert random NINJAM loop"
};



HWND g_parent;


void readChapterFile(char* fileName, MediaTrack* track )
{
    
    std::ifstream infile(fileName);
    int markerindex = 0;
    std::string line;
    
    while (std::getline(infile, line)) {
        
        
        std::string timeStamp = line.substr(0, line.find(' ', 0));
        std::string chapterText = line.substr(line.find(' ', 0)+1, line.length());
        
        std::string* test = &chapterText;
        
        double time = parse_timestr(timeStamp.c_str());
        
        
        MediaItem* item = AddMediaItemToTrack(track);
        if(item)
        {
            SetMediaItemPosition(item, time, true);
            SetMediaItemLength(item, 10, true);
            GetSetMediaItemInfo(item, "P_NOTES", (void*)test->c_str());
            SetProjectMarker3(0, markerindex, true, time, 10, chapterText.c_str(), 0);
        }
        markerindex++;
        
    }
    
    infile.close();
    
}

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


void ImportChapters()
{
    char* chapterTrackName = "Chapters";
    char* selectedImportPath = new char[4096];
    
    MediaTrack* track = getTrackByName("Chapters");

    if(!track)
    {
        InsertTrackAtIndex(0, true);
        track = GetTrack(0,0);
        GetSetMediaTrackInfo(track, "P_NAME", chapterTrackName);
    }
    
    
    if(GetUserFileNameForRead(selectedImportPath, "Load Chapter File", "mp4chaps"))
        readChapterFile(selectedImportPath, track);
    
    free(chapterTrackName);
    free(selectedImportPath);
    
}

bool hookCommandProc(int command, int flag)
{
  if (g_registered_command && command == g_registered_command)
  {
    ImportChapters();
    return true;
  }
  return false;
}

extern "C"
{

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
  g_hInst=hInstance;
  if (rec)
  {
    if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
      return 0;

      
      
      
    #define IMPORT(x) if (!(*((void **)&x) = rec->GetFunc(#x))) return 0;
    IMPORT(GetSetMediaTrackInfo)
    IMPORT(GetSelectedMediaItem)
    IMPORT(GetTrack)
    IMPORT(GetUserFileNameForRead)
    IMPORT(parse_timestr)
    IMPORT(GetSetMediaItemInfo)
    IMPORT(AddMediaItemToTrack)
    IMPORT(SetMediaItemPosition)
    IMPORT(SetMediaItemLength)
    IMPORT(SetProjectMarker3)
    IMPORT(GetNumTracks)
    IMPORT(InsertTrackAtIndex)
      
    *((void **)&InsertMedia) = rec->GetFunc("InsertMedia");
    *((void **)&GetMainHwnd) = rec->GetFunc("GetMainHwnd");
    *((void **)&GetProjectPath) = rec->GetFunc("GetProjectPath");
    

    
    if (!InsertMedia || !GetMainHwnd||!GetProjectPath) return 0;
    acreg.accel.cmd = g_registered_command = rec->Register("command_id",(void*)"NINJAMLOOP");

    if (!g_registered_command) return 0; // failed getting a command id, fail!

    rec->Register("gaccel",&acreg);
    rec->Register("hookcommand",(void*)hookCommandProc);


    g_parent = GetMainHwnd();

    HMENU hMenu = GetSubMenu(GetMenu(GetMainHwnd()),
#ifdef _WIN32
							 0
#else // OS X has one extra menu
							 1
#endif
	  );
    MENUITEMINFO mi={sizeof(MENUITEMINFO),};
    mi.fMask = MIIM_TYPE | MIIM_ID;
    mi.fType = MFT_STRING;
    mi.dwTypeData = "Import Ultraschall chapter file";
    mi.wID = g_registered_command;
    InsertMenuItem(hMenu, 11, TRUE, &mi);

    // our plugin registered, return success

    return 1;
  }
  else
  {
    return 0;
  }
}

};

#ifndef _WIN32 // MAC resources
#include "../../WDL/swell/swell-dlggen.h"
#include "res.rc_mac_dlg"
#undef BEGIN
#undef END
#include "../../WDL/swell/swell-menugen.h"
#include "res.rc_mac_menu"
#endif