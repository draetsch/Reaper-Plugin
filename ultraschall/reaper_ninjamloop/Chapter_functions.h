//
//  Chapter_functions.h
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 04.05.14.
//
//

#ifndef reaper_Ultraschall_Chapter_functions_h
#define reaper_Ultraschall_Chapter_functions_h

#include "reaper.h"
#include "openPanel_connector.h"

/*struct time
{
    int ms, s, m, h = NULL;
    
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
    times.ms =     times.s =     times.m =     times.h = 0;
    
    times.ms = std::atoi(items[0].substr(items[0].find("."), std::string::npos).c_str());
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
    
    sprintf(buffer, "%02d:%02d:%02d.%03d", times.h, times.m, times.s, times.ms);
    
    return std::string(buffer);
    
}*/

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
        
        std::string a = _format_time(charStr);
        
        a.append((std::string)" " + markerName);
        
        chapterLines.push_back(a);
    }
    
    std::string sProjectName = std::string(projectName);
    auto chapterFilename = sProjectName.substr( 0, sProjectName.find('.', 0) );
    std::ofstream chapterFile(chapterFilename + ".mp4chaps", std::ios::out);
    
    for (std::vector<std::string>::iterator iter = chapterLines.begin(); iter != chapterLines.end(); ++iter)
    {
        chapterFile << iter->data() << "\n";
    }
    
    
    
    chapterFile.close();
    
    char buffer[4096];
    sprintf(buffer, "The file was saved in:\n%s", (chapterFilename + ".mp4chaps").c_str());
    ShowMessageBox(buffer, "Chapter file saved", 0);
    
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

        chapterElement->SetAttribute("start", _format_time(charStr).c_str());
        chapterElement->SetAttribute("title", markerName);
        rootElement->InsertEndChild(chapterElement);
    }
    
    std::string sProjectName = std::string(projectName);
    auto chapterFilename = sProjectName.substr( 0, sProjectName.find('.', 0) );
    chapterFilename.append(".simplechapters");
    
    doc.SaveFile(chapterFilename.c_str());
    
    char buffer[4096];
    sprintf(buffer, "The file was saved in:\n%s", chapterFilename.c_str());
    ShowMessageBox(buffer, "Chapter file saved", 0);
    
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


void ImportChapters()
{
    char* chapterTrackName = "Chapters";
    
    auto path = openPanel(CHAPTERS);
    if(path) {
        MediaTrack* track = getTrackByName("Chapters");
        if(!track)
        {
            InsertTrackAtIndex(0, true);
            track = GetTrack(0,0);
            GetSetMediaTrackInfo(track, "P_NAME", chapterTrackName);
        }

        readChapterFile(path, track);
    }
    
    
    //free(path);
    
}

#endif
