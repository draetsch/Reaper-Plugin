#ifdef _WIN32
#include <windows.h>
#else
#include "../../WDL/swell/swell.h"
#endif

#include <stdio.h>
#include <math.h>

#include "resource.h"

#include "../reaper_plugin.h"

#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>
#include <iterator>
#include <regex>
#include <tuple>
#include <fstream>

#include "tinyxml2.h"



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
bool (*SetProjectMarker)(int markrgnindexnumber, bool isrgn, double pos, double rgnend, const char* name);
int (*GetNumTracks)();
void (*InsertTrackAtIndex)(int idx, bool wantDefaults);
int (*AddProjectMarker2)(ReaProject* proj, bool isrgn, double pos, double rgnend, const char* name, int wantidx, int color);
void (*UpdateArrange)();
void (*UpdateTimeline)();
int (*CountProjectMarkers)(ReaProject* proj, int* num_markersOut, int* num_regionsOut);
int (*EnumProjectMarkers)(int idx, bool* isrgnOut, double* posOut, double* rgnendOut, const char** nameOut, int* markrgnindexnumberOut);
void (*format_timestr_pos)(double tpos, char* buf, int buf_sz, int modeoverride);
ReaProject* (*EnumProjects)(int idx, char* projfn, int projfn_sz);
int (*ShowMessageBox)(const char* msg, const char* title, int type);


int g_registered_command=0;
int g_registered_command_01=1;
int g_registered_command_02=2;
int g_registered_command_03=3;
int g_registered_command_04=4;


REAPER_PLUGIN_HINSTANCE g_hInst;
int (*InsertMedia)(char *file, int mode); // mode: 0=add to current track, 1=add new track
HWND (*GetMainHwnd)();
void (*GetProjectPath)(char *buf, int bufsz);
gaccel_register_t acreg=
{
  {FALT|FVIRTKEY,'1',0},
  "Load chapter file"
};

gaccel_register_t acreg1=
{
    {FALT|FVIRTKEY,'2',0},
    "Load shownote file"
};

gaccel_register_t acreg2=
{
    {FALT|FVIRTKEY,'3',0},
    "Export chapters xml"
};

gaccel_register_t acreg3=
{
    {FALT|FVIRTKEY,'4',0},
    "Export chapters"
};

gaccel_register_t acreg4=
{
    {FALT|FVIRTKEY,'5',0},
    "Export shownotes"
};

struct shownoteData
{
    double pos;
    std::string note;
};

HWND g_parent;

void exportShownotes()
{
    
    
    
    
    auto i = 0;
}

void exportChapters()
{
    int markerIndex = 0;
    bool isRegion;
    double pos;
    double regionEnd;
    const char *markerName;
    int index;
    char *charStr = new char[4096];
    char *projectName = new char[4096];
    
    
    EnumProjects(0, projectName, 4096);
    
    if ( strcmp(projectName, "") == 0 ) {
        ShowMessageBox("The Project was not safed", "Project location error", 0);
        return;
    }
    
    std::vector<std::string> chapterLines;
    
    while( EnumProjectMarkers( markerIndex++, &isRegion, &pos, &regionEnd, &markerName, &index) > 0 ) {
        format_timestr_pos(pos, charStr, 4096, 0);

        std::string a(charStr);
        a.append((std::string)" " + markerName);
        
        chapterLines.push_back(a);
    }
    
    std::string sProjectName = std::string(projectName);
    auto chapterFilename = sProjectName.substr( 0, sProjectName.find('.', 0) );
    std::ofstream chapterFile(chapterFilename + ".mp4chaps", std::ios::app);
    
    for (std::vector<std::string>::iterator iter = chapterLines.begin(); iter != chapterLines.end(); ++iter)
    {
        chapterFile << iter->data() << "\n";
    }
    
    
    
    chapterFile.close();
    
    free(charStr);
    free(projectName);
    
}

void exportChaptersAsSimpleChapters()
{
    int markerIndex = 0;
    bool isRegion;
    double pos;
    double regionEnd;
    const char *markerName;
    int index;
    char *charStr = new char[4096];
    char *projectName = new char[4096];
    
    
    EnumProjects(0, projectName, 4096);
    
    if ( strcmp(projectName, "") == 0 ) {
        ShowMessageBox("The Project was not safed", "Project location error", 0);
        return;
    }
    
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *rootElement = doc.NewElement("psc:chapters");
    rootElement->SetAttribute("xmlns", "psc=\"http://podlove.org/simple-chapters\"");
    rootElement->SetAttribute("version", "1.2");
    
    doc.InsertFirstChild(rootElement);
    
    while( EnumProjectMarkers( markerIndex++, &isRegion, &pos, &regionEnd, &markerName, &index) > 0 ) {
        tinyxml2::XMLElement *chapterElement = doc.NewElement("psc:chapter");
        format_timestr_pos(pos, charStr, 4096, 0);
        chapterElement->SetAttribute("start", charStr);
        chapterElement->SetAttribute("title", markerName);
        rootElement->InsertEndChild(chapterElement);
    }
    
    std::string sProjectName = std::string(projectName);
    auto chapterFilename = sProjectName.substr( 0, sProjectName.find('.', 0) );
    chapterFilename.append(".simplechapters");
    
    doc.SaveFile(chapterFilename.c_str());
    
    free(charStr);
    free(projectName);
}

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
            AddProjectMarker2(0, false, time, 10, chapterText.c_str(), 0, 0xFFFF00);
        }
        markerindex++;
        
    }
    
    infile.close();
    
}

void readShownoteFile(char* fileName, MediaTrack* track)
{
    
    std::ifstream infile(fileName);
    std::string line;
    std::vector<shownoteData> lines;
    int starttime = NULL;
    int lastStartTime = NULL;
    int maxlength = 30;
    int length;
    
    std::vector<std::string> header;
    
    
    // read until the Header ends
    while (std::getline(infile, line))
    {
        if (strcmp(line.c_str(), "/HEADER") == 0) {
            break;
        }
        
        if (strcmp(line.c_str(), "HEADER") == 0) {
            continue;
        }
        
    }
    
    // check for type of timestamp
    //auto jk = lines[0].substr(0, lines[0].find(' ', 0));
    
    
    
    // Read all shownote lines into a vector to reverse
    while (std::getline(infile, line)) {
        
        if(line.empty())
            continue;
        
        std::string firstElement = line.substr(0, line.find(' ', 0));
        
        std::regex r("[0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}");
        
        shownoteData data;
        
        
        // timestring is in format 00:00:00.000
        if(std::regex_match(firstElement, r))
        {
            data.pos = parse_timestr(firstElement.c_str());
            
            
        }
        else
        {
            // timestamp is unix timestamp
            try {
                
                if (!starttime) {
                    starttime = std::stod(firstElement);
                    data.pos = 0;
                }
                else
                {
                    data.pos = std::stod(firstElement) - starttime;
                }
                
            } catch (std::exception& e) {
                if (lines.size() > 0) {
                    lines.back().note.append("\n " + line);
                    continue;
                }
                else
                {
                    // Error
                }
            }
        }
        
        data.note = line.substr(line.find(' ', 0)+1, line.length());
        lines.push_back(data);
        

    }

    
    
    // Iterate over all shownotes and create the mediaitems
    std::reverse(lines.begin(), lines.end());
    for (std::vector<shownoteData>::iterator iter = lines.begin(); iter != lines.end(); ++iter) {
        
        
        
        //std::string timeStamp = iter->substr(0, iter->find(' ', 0));
        //std::string shownoteText = iter->substr(iter->find(' ', 0)+1, iter->length());
        //int time = std::stoi(timeStamp) - starttime;
        
        //double time = parse_timestr(timeStamp.c_str());
         
        if(!lastStartTime)
            length = maxlength;
        else
        {
        
            if (lastStartTime-iter->pos > maxlength) {
                length = maxlength;
            }
            else
            {
                length = lastStartTime - iter->pos;
            }
        }
        
        MediaItem* item = AddMediaItemToTrack(track);
        if(item)
        {
            SetMediaItemPosition(item, iter->pos, true);
            SetMediaItemLength(item, length, true);
            GetSetMediaItemInfo(item, "P_NOTES", (void*)iter->note.c_str());
        }
        
        lastStartTime = iter->pos;
        
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
    
    free(selectedImportPath);
    
}

void ImportShowNotes()
{
    char* shownoteTrackName = "Shownotes";
    char* selectedImportPath = new char[4096];
    
    MediaTrack* track = getTrackByName("Shownotes");
    
    if(!track)
    {
        InsertTrackAtIndex(0, true);
        track = GetTrack(0,0);
        GetSetMediaTrackInfo(track, "P_NAME", shownoteTrackName);
    }
    
    
    if(GetUserFileNameForRead(selectedImportPath, "Load Shownote File", "osf"))
        readShownoteFile(selectedImportPath, track);
    
    free(selectedImportPath);
}

bool hookCommandProc(int command, int flag)
{
  if (g_registered_command && command == g_registered_command)
  {
    ImportChapters();
    return true;
  }
  if (g_registered_command_01 && command == g_registered_command_01)
  {
      ImportShowNotes();
      return true;
  }
  if (g_registered_command_02 && command == g_registered_command_02)
  {
      exportChaptersAsSimpleChapters();
      return true;
  }
  if (g_registered_command_03 && command == g_registered_command_03)
  {
      exportChapters();
      return true;
  }
  if (g_registered_command_04 && command == g_registered_command_04)
  {
      exportShownotes();
      return true;
  }
    
    UpdateTimeline();
    UpdateArrange();
    
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
    IMPORT(SetProjectMarker)
    IMPORT(GetNumTracks)
    IMPORT(InsertTrackAtIndex)
    IMPORT(InsertMedia)
    IMPORT(GetMainHwnd)
    IMPORT(GetProjectPath)
    IMPORT(AddProjectMarker2)
    IMPORT(UpdateArrange)
    IMPORT(UpdateTimeline)
    IMPORT(CountProjectMarkers)
    IMPORT(EnumProjectMarkers)
    IMPORT(format_timestr_pos)
    IMPORT(EnumProjects)
    IMPORT(ShowMessageBox)

    acreg.accel.cmd = g_registered_command = rec->Register("command_id",(void*)"Load chapter file");
    acreg1.accel.cmd = g_registered_command_01 = rec->Register("command_id",(void*)"Load shownote file");
    acreg2.accel.cmd = g_registered_command_02 = rec->Register("command_id",(void*)"Export Chapters");
    acreg3.accel.cmd = g_registered_command_03 = rec->Register("command_id",(void*)"Export Chapters2");
    acreg4.accel.cmd = g_registered_command_04 = rec->Register("command_id",(void*)"Export Shownotes");

    if (!g_registered_command) return 0; // failed getting a command id, fail!
    if (!g_registered_command_01) return 0; // failed getting a command id, fail!
    if (!g_registered_command_02) return 0; // failed getting a command id, fail!
    if (!g_registered_command_03) return 0; // failed getting a command id, fail!
    if (!g_registered_command_04) return 0; // failed getting a command id, fail!

    rec->Register("gaccel",&acreg);
    rec->Register("hookcommand",(void*)hookCommandProc);
      
    rec->Register("gaccel",&acreg1);
    rec->Register("hookcommand",(void*)hookCommandProc);
      
    rec->Register("gaccel",&acreg2);
    rec->Register("hookcommand",(void*)hookCommandProc);
      
    rec->Register("gaccel",&acreg3);
    rec->Register("hookcommand",(void*)hookCommandProc);

    rec->Register("gaccel",&acreg4);
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
    InsertMenuItem(hMenu, 7, TRUE, &mi);
      
    MENUITEMINFO mi2={sizeof(MENUITEMINFO),};
    mi2.fMask = MIIM_TYPE | MIIM_ID;
    mi2.fType = MFT_STRING;
    mi2.dwTypeData = "Import Ultraschall shownote file";
    mi2.wID = g_registered_command_01;
    InsertMenuItem(hMenu, 8, TRUE, &mi2);
      
    MENUITEMINFO mi3={sizeof(MENUITEMINFO),};
    mi3.fMask = MIIM_TYPE | MIIM_ID;
    mi3.fType = MFT_STRING;
    mi3.dwTypeData = "Export Ultraschall PODLove simple chapters";
    mi3.wID = g_registered_command_02;
    InsertMenuItem(hMenu, 9, TRUE, &mi3);
      
    MENUITEMINFO mi4={sizeof(MENUITEMINFO),};
    mi4.fMask = MIIM_TYPE | MIIM_ID;
    mi4.fType = MFT_STRING;
    mi4.dwTypeData = "Export Ultraschall mp4 chapters";
    mi4.wID = g_registered_command_03;
    InsertMenuItem(hMenu, 10, TRUE, &mi4);

      MENUITEMINFO mi5={sizeof(MENUITEMINFO),};
      mi5.fMask = MIIM_TYPE | MIIM_ID;
      mi5.fType = MFT_STRING;
      mi5.dwTypeData = "Export Ultraschall Shownotes";
      mi5.wID = g_registered_command_04;
      InsertMenuItem(hMenu, 11, TRUE, &mi5);

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