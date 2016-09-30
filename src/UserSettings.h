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

#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include "Vector3D.h"

#include <string>

class UserSettings
{
public:

  enum SimplificationMode {simplificationMode_none,
                           simplificationMode_bounding_box,
                           simplificationMode_fast};
  UserSettings();
  ~UserSettings();

  int  loadFile       (const std::string& pFilename,
                       std::string&       pError);

  void saveFile       (const std::string& pFilename,
                       std::string&       pError);

  float              aBackgroundB;
  float              aBackgroundG;
  int                aBackgroundMode;
  float              aBackgroundR;
  bool               aFlagGrid;
  bool               aFlagAxes;
  float              aLightAmbient;
  int                aLightModel;
  int                aLightPositionLocal;
  Vector3D           aLightPosition;
  SimplificationMode aSimplicationMode;

};

#endif // USERSETTINGS_H
