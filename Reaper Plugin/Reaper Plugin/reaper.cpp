//
//  reaper.cpp
//  Reaper Plugin
//
//  Created by Daniel Lindenfelser on 21.06.14.
//

#include <iostream>
#include <vector>

#define REAPERAPI_DECL
#include "reaper.h"
#undef REAPERAPI_DECL

#ifdef _WIN32
#include <windows.h>
#else
#include "WDL/swell/swell.h"
#endif

#include "Chapter.h"
#include "Shownote.h"

static WDL_IntKeyedArray<COMMAND_T*> g_commands; // no valdispose (cmds can be allocated in different ways)

int g_iFirstCommand = 0;
int g_iLastCommand = 0;

// 1) Get command ID from Reaper
// 2) Add keyboard accelerator (with localized action name) and add to the "action" list
int RegisterCmd(COMMAND_T* pCommand, const char* cFile, int cmdId)
{
    if (!pCommand || !pCommand->id || !pCommand->accel.desc || (!pCommand->doCommand && !pCommand->onAction)) return 0;
    
	// localized action name, if needed
	const char* defaultName = pCommand->accel.desc;
    
	if (!pCommand->uniqueSectionId && pCommand->doCommand)
	{
		if (!cmdId) cmdId = plugin_register("command_id", (void*)pCommand->id);
		if (cmdId)
		{
			pCommand->accel.accel.cmd = cmdId; // need to this before registering "gaccel"
			cmdId = (plugin_register("gaccel", &pCommand->accel) ? cmdId : 0);
		}
	}
	else if (pCommand->onAction)
	{
		static custom_action_register_t s;
		memset(&s, 0, sizeof(custom_action_register_t));
		s.idStr = pCommand->id;
		s.name = pCommand->accel.desc;
		s.uniqueSectionId = pCommand->uniqueSectionId;
		cmdId = plugin_register("custom_action", (void*)&s); // will re-use the known cmd ID, if any
	}
	else
		cmdId = 0;
	pCommand->accel.accel.cmd = cmdId;
    
	// now that it is registered, restore the default action name
	if (pCommand->accel.desc != defaultName) pCommand->accel.desc = defaultName;
    
	if (!cmdId) return 0;
    
	if (!g_iFirstCommand || g_iFirstCommand > cmdId) g_iFirstCommand = cmdId;
	if (cmdId > g_iLastCommand) g_iLastCommand = cmdId;
    
	g_commands.Insert(cmdId, pCommand);
    
	return pCommand->accel.accel.cmd;
}

// For each item in table call RegisterCommand
int RegisterCmds(COMMAND_T* pCommands, const char* cFile)
{
	int i = 0;
	while(pCommands[i].id != LAST_COMMAND)
		RegisterCmd(&pCommands[i++], cFile, 0);
	return 1;
}

COMMAND_T* GetCommandByID(int cmdId) {
	if (cmdId >= g_iFirstCommand && cmdId <= g_iLastCommand) // not enough to ensure it is a SWS action
		return g_commands.Get(cmdId, NULL);
	return NULL;
}

bool hookCommandProc(int iCmd, int flag)
{
    static WDL_PtrList<const char> sReentrantCmds;
    
    // Ignore commands that don't have anything to do with us from this point forward
	if (COMMAND_T* cmd = GetCommandByID(iCmd))
	{
		if (!cmd->uniqueSectionId && cmd->accel.accel.cmd==iCmd && cmd->doCommand)
		{
			if (sReentrantCmds.Find(cmd->id)<0)
			{
				sReentrantCmds.Add(cmd->id);
				cmd->fakeToggle = !cmd->fakeToggle;
				cmd->doCommand(cmd);
				sReentrantCmds.Delete(sReentrantCmds.Find(cmd->id));
				return true;
			}
		}
	}
    
    return false;
}

extern "C"
{
    
    REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
    {
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
            IMPORT(plugin_register)
            if (!rec->Register("hookcommand", (void*)hookCommandProc))
                return 0;
            
            // Call plugin specific init
            if (!UltraschallChaptersInit())
                return 0;
            if (!UltraschallShownotesInit())
                return 0;
            
            // our plugin registered, return success
            
            return 1;
        }
        else
        {
            return 0;
        }
    }
};

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
