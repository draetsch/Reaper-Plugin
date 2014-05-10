//
//  Shownote_functions.h
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 04.05.14.
//
//

#ifndef __reaper_Ultraschall__Shownote_functions__
#define __reaper_Ultraschall__Shownote_functions__

#include "reaper.h"


struct shownoteData
{
    double pos;
    std::string note;
};


void exportShownotes()
{
    
    char *projectName = new char[4096];
    
    EnumProjects(0, projectName, 4096);
    
    if ( strcmp(projectName, "") == 0 ) {
        ShowMessageBox("The Project was not safed", "Project location error", 0);
        return;
    }
    
    MediaTrack* track = getTrackByName("Shownotes");
    char* charStr = new char[4096];
    
    if(!track)
        return;
    
    std::vector<std::string> chapterLines;
    
    for (int i=0; i < CountTrackMediaItems(track); ++i) {
        
        MediaItem* item = GetTrackMediaItem(track, i);
        
        std::string note = (const char*)GetSetMediaItemInfo(item, "P_NOTES", NULL);
        
        double pos = GetMediaItemInfo_Value(item, "D_POSITION");
        
        format_timestr_pos(pos, charStr, 4096, 0);
        auto a = _format_time(charStr);
        
        
        a.append((std::string)" " + note);
        
        chapterLines.push_back(a);
        
        
        
    }
    
    std::string sProjectName = std::string(projectName);
    auto chapterFilename = sProjectName.substr( 0, sProjectName.find('.', 0) );
    std::ofstream chapterFile(chapterFilename + ".shownotes", std::ios::app);
    
    for (std::vector<std::string>::iterator iter = chapterLines.begin(); iter != chapterLines.end(); ++iter)
    {
        chapterFile << iter->data() << "\n";
    }
    
    
    
    chapterFile.close();
    
    delete charStr;
    delete projectName;
    
    ShowMessageBox("Shownotes exported", "Project location error", 0);
    
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
        else
        {
            header.push_back(line);
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
    
    
    if(GetUserFileNameForRead(selectedImportPath, "Load Shownote File", ".txt"))
        readShownoteFile(selectedImportPath, track);
    
    free(selectedImportPath);
}

#endif /* defined(__reaper_Ultraschall__Shownote_functions__) */
