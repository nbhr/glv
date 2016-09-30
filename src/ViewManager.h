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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include "GraphicData.h"
#include "Object.h"
#include "UserSettings.h"
#include "Vector3D.h"
#include "View.h"
#include <vector>

class Tile;

// ViewManager
//  Class handling the user interactions and drawing the basic graphical elements
//  Contains GraphicData; which contains the "user data"
//  UserSettings is the "preferences" state.
class ViewManager
{
public:

  ViewManager ();
  ~ViewManager();

  void          addView                (const View&        pView);

  void          addStatusMessage       (const std::string& pMessage);

  void          clearStatusMessages    ();

  void          display                (const Tile&        pTile);

  void          displayCallback        ();

  GraphicData&  getGraphicData         ();

  UserSettings& getUserSettings        ();

  bool          idleCallback           ();

  bool          keyboardCallback       (unsigned char      pKey);

  bool          keyboardSpecialCallback(int                pKey);

  bool          menuCallback           (const char*        pMenuEvent);

  bool          mouseButtonCallback    (int                pButton,
                                        int                pState,
                                        int                pKeyState,
                                        int                pX,
                                        int                pY);

  bool          mouseMotionCallback    (int                pX,
                                        int                pY);

  void          reshapeCallback        (int                pWidth,
                                        int                pHeight);

  void          setTitle               (const std::string& pTitle);

private:

  // Block the use of those
  ViewManager(const ViewManager&);
  ViewManager& operator=(const ViewManager&);

  enum MouseMoveMode {mouseMove_none,
                      mouseMove_rotate,
                      mouseMove_translate,
                      mouseMove_zoom};

  void drawAxes                   ();
  void drawGrid                   ();
  void drawMouseMovementIndicators();
  void drawTextLayer              (const Tile& pTile);
  void render                     (const Tile& pTile);
  void resetCamera                ();
  void setupLighting              ();

  Object                   aAxes;
  View                     aCurrentView;
  GraphicData              aGraphicData;
  Object                   aGrid;
  int                      aLastMouseX;
  int                      aLastMouseY;
  MouseMoveMode            aMoveMode;
  bool                     aNewViewAdded;
  std::vector<std::string> aStatusMessages;
  std::string              aTitle;
  UserSettings             aUserSettings;
  int                      aViewIndex;
  std::vector<View>        aViews;
  int                      aWindowHeight;
  int                      aWindowWidth;
};

#endif // VIEWMANAGER_H
