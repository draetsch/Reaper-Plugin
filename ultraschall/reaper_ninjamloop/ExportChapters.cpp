//
//  ExportChapters.cpp
//  reaper_Ultraschall
//
//  Created by Malte Dreschert on 25.04.14.
//
//

#include "ExportChapters.h"
//#include "../reaper_plugin.h"
//#include "functions.h"

using namespace tinyxml2;

void exportChapters()
{
    
    //auto jj = CountProjectMarkers(0, NULL, NULL);
    
    
    XMLDocument * doc = new XMLDocument();
    XMLElement * root = doc->NewElement("Chapters");
    
    doc->InsertEndChild(root);
    
    //double time = parse_timestr("sdd");
    
    doc->SaveFile("/Users/Malte/___hhh.xml");
    auto h = 0;
}