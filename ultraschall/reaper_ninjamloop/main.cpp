#ifdef _WIN32
#include <windows.h>
#else
#include "../../WDL/swell/swell.h"
#endif

#include <stdio.h>
#include <math.h>

#include "resource.h"

#include "../reaper_plugin.h"
#include "reaper.h"

#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>
#include <iterator>
#include <regex>
#include <tuple>
#include <fstream>

#include "../../WDL/queue.h"

#include "tinyxml2.h"
#include "Shownote_functions.h"
#include "Chapter_functions.h"

extern gaccel_register_t mdbla;


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
MediaItem* (*GetTrackMediaItem)(MediaTrack* tr, int itemidx);
int (*CountTrackMediaItems)(MediaTrack* track);
PCM_sink* (*PCM_Sink_Create)(const char* filename, const char* cfg, int cfg_sz, int nch, int srate, bool buildpeaks);


int g_registered_command=0;
int g_registered_command_01=0;
int g_registered_command_02=0;
int g_registered_command_03=0;
int g_registered_command_04=0;


REAPER_PLUGIN_HINSTANCE g_hInst;
int (*InsertMedia)(char *file, int mode); // mode: 0=add to current track, 1=add new track
HWND (*GetMainHwnd)();
void (*GetProjectPath)(char *buf, int bufsz);
gaccel_register_t acreg=
{
  {FALT|FVIRTKEY,'1',0},
  "Ultraschall Load chapter"
};

gaccel_register_t acreg1=
{
    {FALT|FVIRTKEY,'2',0},
    "Ultraschall Load Shownote"
};

gaccel_register_t acreg2=
{
    {FALT|FVIRTKEY,'3',0},
    "Ultraschall Export Simplechapters"
};

gaccel_register_t acreg3=
{
    {FALT|FVIRTKEY,'4',0},
    "Ultraschall Export chapters"
};

gaccel_register_t acreg4=
{
    {FALT|FVIRTKEY,'5',0},
    "Ultraschall Export Shownotes"
};




HWND g_parent;

WDL_DLGRET doInsertProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //static JNL_HTTPGet *m_get;
    //static WDL_Queue m_buf;
    switch (uMsg)
    {
        case WM_INITDIALOG:
            //m_get=new JNL_HTTPGet;
            //m_get->connect("http://autosong.ninjam.com/autoloop/auto.php");
            //m_buf.Clear();
            
            //SetTimer(hwndDlg,1,20,NULL);
            return 0;
        case WM_TIMER:
            /*if (wParam==1)
            {
                int rv=m_get->run();
                if (rv>=0)
                {
                    int av=m_get->bytes_available();
                    if (av>0)
                    {
                        m_get->get_bytes((char*)m_buf.Add(NULL,av),av);
                    }
                }
                
                if (rv==0)  return 0;
                
                KillTimer(hwndDlg,1);
                char *hdr=0;
                if (rv<0||m_buf.GetSize()<1||!(hdr=m_get->getheader("ex-fn"))||!*hdr||strstr(hdr,"/") || strstr(hdr,"\\"))
                    MessageBox(hwndDlg,"Error getting a loop, internet ok?","Err",MB_OK);
                else
                {
                    char buf[2048];
                    GetProjectPath(buf,sizeof(buf)-1024);
                    strcat(buf,"\\");
                    lstrcpyn(buf+strlen(buf),hdr,512);
                    FILE *fp=fopen(buf,"rb");
                    if (fp)
                    {
                        fclose(fp);
                        MessageBox(hwndDlg,"Error writing loop, file already exists!","Err",MB_OK);
                    }
                    else
                    {
                        fp=fopen(buf,"wb");
                        if (!fp)
                        {
                            MessageBox(hwndDlg,"Error writing loop, could not create flie!","Err",MB_OK);
                        }
                        else
                        {
                            fwrite(m_buf.Get(),1,m_buf.GetSize(),fp);
                            fclose(fp);
                            
                            InsertMedia(buf,0);
                        }
                    }
                    // save to disk, insert loop
                }
                EndDialog(hwndDlg,0);
                
            }*/
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam)==IDCANCEL)
            {
                EndDialog(hwndDlg,0);
            }
            return 0;
        case WM_DESTROY:
            //m_buf.Clear();
            //delete m_get;
            return 0;
    }
    return 0;
}


void DoInsertPoo()
{
    
    DialogBox(g_hInst,MAKEINTRESOURCE(IDD_CFG),g_parent,doInsertProc);
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
      //DoInsertPoo();
      return true;
  }
    
    
    //DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CFG), g_parent, doInsertProc);
    

    
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
    IMPORT(CountTrackMediaItems)
    IMPORT(GetTrackMediaItem)
    IMPORT(PCM_Sink_Create)
    IMPORT(GetMediaItemInfo_Value)
      
      
    
      acreg.accel.cmd = g_registered_command = rec->Register("command_id",(void*)"1");
    acreg1.accel.cmd = g_registered_command_01 = rec->Register("command_id",(void*)"2");
    acreg2.accel.cmd = g_registered_command_02 = rec->Register("command_id",(void*)"3");
    acreg3.accel.cmd = g_registered_command_03 = rec->Register("command_id",(void*)"4");
    acreg4.accel.cmd = g_registered_command_04 = rec->Register("command_id",(void*)"5");
      //mdbla.accel.cmd = g_registered_command_06 = rec->Register("command_id", (void*)"bla");
      //rec->Register("gaccel", &mdbla);
      
    if (!g_registered_command) return 0; // failed getting a command id, fail!
    if (!g_registered_command_01) return 0; // failed getting a command id, fail!
    if (!g_registered_command_02) return 0; // failed getting a command id, fail!
    if (!g_registered_command_03) return 0; // failed getting a command id, fail!
    if (!g_registered_command_04) return 0; // failed getting a command id, fail!

    rec->Register("gaccel",&acreg);
    rec->Register("hookcommand",(void*)hookCommandProc);
      
    rec->Register("gaccel",&acreg1);
      
    rec->Register("gaccel",&acreg2);
      
    rec->Register("gaccel",&acreg3);

    rec->Register("gaccel",&acreg4);
    

    g_parent = GetMainHwnd();

      
    HMENU hMenu = GetSubMenu(GetMenu(GetMainHwnd()),
#ifdef _WIN32
							 0
#else // OS X has one extra menu
							 1
#endif
	  );
      
    /*MENUITEMINFO mi={sizeof(MENUITEMINFO),};
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
    
      MENUITEMINFO mi6={sizeof(MENUITEMINFO),};
      mi6.fMask = MIIM_TYPE | MIIM_ID;
      mi6.fType = MFT_STRING;
      mi6.dwTypeData = "Export Ultraschall Shownotes";
      mi6.wID = g_registered_command_05;
      InsertMenuItem(hMenu, 12, TRUE, &mi6);*/

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