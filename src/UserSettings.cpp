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


#include "UserSettings.h"
#include <cstdio>
#include <algorithm>
#include "assert_glv.h"
#include "limits_glv.h"

// By default, distant light
UserSettings::UserSettings()
  : aBackgroundB       (0.0f),
    aBackgroundG       (0.0f),
    aBackgroundMode    (0),
    aBackgroundR       (0.0f),
    aFlagGrid          (true),
    aFlagAxes          (false),
    aLightAmbient      (0.5f),
    aLightModel        (1),
    aLightPositionLocal(0),
    aLightPosition     (1.0f, 1.0f, 1.0f),
    aSimplicationMode  (simplificationMode_none)
{}

UserSettings::~UserSettings()
{}

// Return 0 if no error (see pError).  This does not override settings
// that arent listed in the file.
// return 1 if error, return 2 if no such file
int UserSettings::loadFile(const std::string& pFilename, std::string& pError)
{
  FILE* file = fopen(pFilename.c_str(),"r");
  if(!file) {
    pError = "Cannot open file : ";
    pError += pFilename;
    return 2;
  }

  float lLightPositionX = aLightPosition.x();
  float lLightPositionY = aLightPosition.y();
  float lLightPositionZ = aLightPosition.z();

  char lLine[200];
  while(fgets(lLine,200,file)) {

    if(lLine[0] == '\n' || lLine[0] == '#') {
      //Skip blank/comment lines
      continue;
    }
    std::string lLineStr(lLine);
    unsigned int lPosEqual = lLineStr.find("=");
    if(lPosEqual == std::string::npos) {
      pError = "Syntax error in configuration file: ";
      pError += pFilename;
      fclose(file);
      return 1;
    }

    std::string lVariable = lLineStr.substr(0,lPosEqual);
    std::string lParam    = lLineStr.substr(lPosEqual+1);

    if(lVariable == "background_mode") {
      sscanf(lParam.c_str(),"%d",&aBackgroundMode);
    }
    else if(lVariable == "background_r") {
      int lBackgroundR;
      sscanf(lParam.c_str(),"%d",&lBackgroundR);
      lBackgroundR = std::max(lBackgroundR, 0);
      lBackgroundR = std::min(lBackgroundR, 255);
      aBackgroundR = static_cast<float>(lBackgroundR)/255.0f;
    }
    else if(lVariable == "background_g") {
      int lBackgroundG;
      sscanf(lParam.c_str(),"%d",&lBackgroundG);
      lBackgroundG = std::max(lBackgroundG, 0);
      lBackgroundG = std::min(lBackgroundG, 255);
      aBackgroundG = static_cast<float>(lBackgroundG)/255.0f;
    }
    else if(lVariable == "background_b") {
      int lBackgroundB;
      sscanf(lParam.c_str(),"%d",&lBackgroundB);
      lBackgroundB = std::max(lBackgroundB, 0);
      lBackgroundB = std::min(lBackgroundB, 255);
      aBackgroundB = static_cast<float>(lBackgroundB)/255.0f;
    }
    else if(lVariable == "enable_grid") {
      int lFlagGrid;
      sscanf(lParam.c_str(),"%d",&lFlagGrid);
      aFlagGrid = (lFlagGrid == 1);
    }
    else if(lVariable == "enable_axes") {
      int lFlagAxes;
      sscanf(lParam.c_str(),"%d",&lFlagAxes);
      aFlagAxes = (lFlagAxes == 1);
    }
    else if(lVariable == "graphic_simplication") {
      int lSimplificationMode = -1;
      sscanf(lParam.c_str(),"%d",&lSimplificationMode);
      if (lSimplificationMode == 1) {
        aSimplicationMode = simplificationMode_bounding_box;
      }
      else if (lSimplificationMode == 2) {
        aSimplicationMode = simplificationMode_fast;
      }
      else {
        aSimplicationMode = simplificationMode_none;
      }
    }
    else if(lVariable == "light_model") {
      sscanf(lParam.c_str(),"%d",&aLightModel);
    }
    else if(lVariable == "light_position_x") {
      sscanf(lParam.c_str(),"%f",&lLightPositionX);
    }
    else if(lVariable == "light_position_y") {
      sscanf(lParam.c_str(),"%f",&lLightPositionY);
    }
    else if(lVariable == "light_position_z") {
      sscanf(lParam.c_str(),"%f",&lLightPositionZ);
    }
    else if(lVariable == "light_position_local") {
      sscanf(lParam.c_str(),"%d",&aLightPositionLocal);
    }
    else if(lVariable == "light_ambient") {
      sscanf(lParam.c_str(),"%f",&aLightAmbient);
    }
  }
  fclose(file);

  aLightPosition = Vector3D(lLightPositionX, lLightPositionY, lLightPositionZ);

  return 0;
}

void UserSettings::saveFile(const std::string& pFilename, std::string& pError)
{
  FILE* file = fopen(pFilename.c_str(),"w");
  if(!file) {
    pError = "Cannot write to file : ";
    pError += pFilename;
  }
  else {
    fprintf(file,"# Background color mode: integer value [0,4]\n");
    fprintf(file,"#   0 = user settings (see variable background_r,g,b), default=black\n");
    fprintf(file,"#   1 = white\n");
    fprintf(file,"#   2 = gray\n");
    fprintf(file,"#   3 = gray gradient\n");
    fprintf(file,"#   4 = black gradient\n");
    fprintf(file,"background_mode=%d\n\n",aBackgroundMode);
    fprintf(file,"# Background color (if background_mode is 0): R G B value [0,255]\n");
    fprintf(file,"background_r=%d\n",static_cast<int>(aBackgroundR*255.0f));
    fprintf(file,"background_g=%d\n",static_cast<int>(aBackgroundG*255.0f));
    fprintf(file,"background_b=%d\n",static_cast<int>(aBackgroundB*255.0f));
    fprintf(file,"# Show grid [0,1]\n");
    fprintf(file,"enable_grid=%d\n\n",aFlagGrid);
    fprintf(file,"# Show axes [0,1]\n");
    fprintf(file,"enable_axes=%d\n\n",aFlagAxes);
    fprintf(file,"# Graphic simplification [0,1,2]\n");
    fprintf(file,"#   0 = no simplification\n");
    fprintf(file,"#   1 = using bounding Boxes for simplification (-bbox)\n");
    fprintf(file,"#   2 = using a small subset of the primitives for simplification (-fast)\n");
    fprintf(file,"#  If 1 or 2 is selected, the 3D model is hidden while using the mouse to rotate, translate, zoom\n");
    fprintf(file,"#    thus, enabling a faster user interaction.  Use this for large models\n");
    switch (aSimplicationMode) {
    case simplificationMode_none:
      fprintf(file,"graphic_simplication=0\n\n");
      break;
    case simplificationMode_bounding_box:
      fprintf(file,"graphic_simplication=1\n\n");
      break;
    default:
      GLV_ASSERT(aSimplicationMode == simplificationMode_fast);
      fprintf(file,"graphic_simplication=2\n\n");
      break;
    }
    fprintf(file,"# Light model [0,2]\n");
    fprintf(file,"#  0 = Flag shading, no lighting\n");
    fprintf(file,"#  1 = Light is camera  (default)\n");
    fprintf(file,"#  2 = Fixed , see light_x,y,z \n");
    fprintf(file,"light_model=%d\n",aLightModel);
    fprintf(file,"# Light position (if light_model is 2)\n");
    fprintf(file,"#  x,y,z = floating point position/vector  local : 0=infinite light 1=local light\n");
    fprintf(file,"light_position_x=%f\n",aLightPosition.x());
    fprintf(file,"light_position_y=%f\n",aLightPosition.y());
    fprintf(file,"light_position_z=%f\n",aLightPosition.z());
    fprintf(file,"light_position_local=%d\n\n",aLightPositionLocal);
    fprintf(file,"# Light ambient constant. floating point value [0,1]\n");
    fprintf(file,"#   if 0: unlit part of the model are totally black\n");
    fprintf(file,"light_ambient=%f\n\n",aLightAmbient);

    fclose(file);
  }
}
