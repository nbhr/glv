//  glv - OpenGL viewer command line tool
/*****************************************************************************
 * Copyright (C) 2003 Patrick Lagace <bl4cklight@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *****************************************************************************/

#ifdef GLV_USE_QT
#include "WindowQt.h"
#include "qapplication.h"
#else
#include "WindowGLUT.h"
#endif

#include "GraphicData.h"

#include <iostream>
#include <map>
#include <set>
#include <string>

// The program accepts data as filenames or standard input.

int main(int argc,char** argv)
{
  // Initialize object rendering hierarchy
#ifdef GLV_USE_QT
  QApplication lApp(argc,argv);
  WindowQt     lGraphWidget;

  lApp.setMainWidget(&lGraphWidget);
#else
  glutInit(&argc,argv);
  WindowGLUT lGraphWidget;
#endif

  ViewManager&  lViewManager  = lGraphWidget.getViewManager();
  GraphicData&  lGraphicData  = lViewManager.getGraphicData();
  UserSettings& lUserSettings = lViewManager.getUserSettings();

  // Command line processing
  std::set<std::string>             lSetSwitchs;
  std::map<std::string,std::string> lIndexOptions;
  std::vector<std::string>          lVectFilenames;

  for(int i=1;i<argc;i++) {
    std::string lArg(argv[i]);
    if(lArg.find("=") != std::string::npos) {
      lIndexOptions[lArg.substr(0,lArg.find("="))] = lArg.substr(lArg.find("=")+1);
    }
    else if(lArg[0] == '-') {
      lSetSwitchs.insert(argv[i]);
    }
    else {
      lVectFilenames.push_back(argv[i]);
    }
  }

  if(lSetSwitchs.find("-h") != lSetSwitchs.end() ||
     lSetSwitchs.find("-help") != lSetSwitchs.end() ||
     lSetSwitchs.find("--help") != lSetSwitchs.end()) {
    std::cout << "USAGE:     glv [FILENAMES] [OPTIONS]" << std::endl;
    std::cout << "      ...| glv [OPTIONS]" << std::endl;
    std::cout << " glv can either be used with filenames; or be given all data" << std::endl;
    std::cout << "   via standart input (pipe or stdin)" << std::endl;
    std::cout << "   -h, -help, --help : Print this help text" << std::endl;
    std::cout << "   -V, -version, --version : Display version number and exit" << std::endl;
    std::cout << " FILENAMES: Any number of filenames in the proper format" << std::endl;
    std::cout << "     See format description on http://glv.sourceforge.net" << std::endl;
#ifndef WIN32
    std::cout << "   Filenames with the .gz suffix will be deflated using gzip" << std::endl;
#endif // WIN32
    std::cout << " General options:" << std::endl;
    std::cout << "   -i : Enable standart input command processing. Even if filenames are given as arguments" << std::endl;
    std::cout << "   -nogui : Use only offscreen snapshots" << std::endl;
    std::cout << " View options: (most of these options are accessible in the GUI right-click menu" << std::endl;
    std::cout << "                or in the ~/.glvrc)" << std::endl;
    std::cout << "   -plain: disable grid and axes display; showing only the object" << std::endl;
    std::cout << "   -grid -axes: enable grid or axes display" << std::endl;
    std::cout << "   -black -white: change background color" << std::endl;
    std::cout << "   -fast : Enable graphic simplification. Disables the rendering of object when using mouse" << std::endl;
    std::cout << "   -bbox : Enable bounding box mode. Disables the rendering of object when using mouse" << std::endl;
    std::cout << " Preprocessing:" << std::endl;
    std::cout << "   -smooth : Smooth normals of triangular raw meshes" << std::endl;
    std::cout << "   -optim=# : Optimizer threshold [100]. Higher values may incur slower loading," << std::endl;
    std::cout << "              but faster display onto some video cards. Very large datasets only." << std::endl;
    return 0;
  }

  if(lSetSwitchs.find("-V") != lSetSwitchs.end() ||
     lSetSwitchs.find("-version") != lSetSwitchs.end() ||
     lSetSwitchs.find("--version") != lSetSwitchs.end()) {
    std::cout << "glv v0.2 - OpenGL viewer command line tool (http://glv.sourceforge.net)" << std::endl;
    return 0;
  }

#ifdef WIN32
  if(argc == 1) {
    std::cerr << "Enter the filename to open:" << std::endl;
    std::cerr << "  (or drag-and-drop a file onto this window)" << std::endl;
    char lBuffer[1024]= "-";
    while(lBuffer[0] == '-') {
      fgets(lBuffer,1024,stdin);
      if(lBuffer[strlen(lBuffer)-1] == '\n') {
        lBuffer[strlen(lBuffer)-1] = '\0';
      }
      if(lBuffer[0] == '-') {
        lSetSwitchs.insert(lBuffer);
      }
      else {
        lVectFilenames.push_back(lBuffer);
      }
    }
  }
#endif

  if(lSetSwitchs.find("-smooth") != lSetSwitchs.end()) {
    lGraphicData.enableSmoothingMode();
  }
  if(lIndexOptions.find("-optim") != lIndexOptions.end()) {
    int lVal = atoi(lIndexOptions["-optim"].c_str());
    if(lVal < 2) {
      std::cerr << "Warning = optimizer value too low; set to minimum of 2" << std::endl;
      lVal = 2;
    }
    lGraphicData.setOptimizerValue(lVal);
  }
  if(lSetSwitchs.find("-bbox") != lSetSwitchs.end()) {
    lUserSettings.aSimplicationMode = UserSettings::simplificationMode_bounding_box;
  }
  if(lSetSwitchs.find("-fast") != lSetSwitchs.end()) {
    lUserSettings.aSimplicationMode = UserSettings::simplificationMode_fast;
  }
  if(lSetSwitchs.find("-plain") != lSetSwitchs.end()) {
    lUserSettings.aFlagAxes = false;
    lUserSettings.aFlagGrid = false;
  }
  if(lSetSwitchs.find("-axes") != lSetSwitchs.end()) {
    lUserSettings.aFlagAxes = true;
  }
  if(lSetSwitchs.find("-grid") != lSetSwitchs.end()) {
    lUserSettings.aFlagGrid = true;
  }
  if(lSetSwitchs.find("-black") != lSetSwitchs.end()) {
    lUserSettings.aBackgroundMode = 0;
    lUserSettings.aBackgroundR    = 0.0f;
    lUserSettings.aBackgroundG    = 0.0f;
    lUserSettings.aBackgroundB    = 0.0f;
  }
  if(lSetSwitchs.find("-white") != lSetSwitchs.end()) {
    lUserSettings.aBackgroundMode = 1;
    lUserSettings.aBackgroundR    = 1.0f;
    lUserSettings.aBackgroundG    = 1.0f;
    lUserSettings.aBackgroundB    = 1.0f;
  }

#ifndef WIN32
  // If no files are given; or the "-i" switch used; the program
  //  expects data from the standard input.
  // The -i switch also disable the end-program-on-error behavior;
  // enabling the stdin to be used as a fault-tolerant interactive terminal.
  if(lSetSwitchs.find("-i") != lSetSwitchs.end()) {
    lGraphicData.enableStdinMode();
    lGraphicData.enableIgnoreErrorMode();
  }
  else if( lVectFilenames.empty()) {
    lGraphicData.enableStdinMode();
  }
#endif // WIN32

  // Read all filenames given on the command line
  std::vector<std::string>::const_iterator       lFilenameIter    = lVectFilenames.begin();
  const std::vector<std::string>::const_iterator lFilenameIterEnd = lVectFilenames.end  ();
  std::string                                    lError;

  while (lFilenameIter != lFilenameIterEnd) {
    lGraphicData.readDataFile(*lFilenameIter, lError);
    if (!lError.empty()) {
      std::cerr << lError << std::endl;
      return 1;
    }
    ++lFilenameIter;
  }

  if(lSetSwitchs.find("-nogui") == lSetSwitchs.end()) {

    // Execute the GUI
#ifdef GLV_USE_QT
    lGraphWidget.show();
    lApp.exec();
#else
    glutMainLoop();
#endif

  }

  return 0;
}


