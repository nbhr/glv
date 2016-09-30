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

#ifndef WINDOWGLV_H
#define WINDOWGLV_H

#include "ViewManager.h"


// Base class for a GLUT, Qt, WINDOWS, ... subclass
// We didn't implement this as a Singleton because of
// the memory still allocated at the end of the
// execution. Instead, we permit only one instance via
// GLV_ASSERT checks.
class WindowGLV
{
public:

  virtual ~WindowGLV();

  static WindowGLV&  getInstance();
  ViewManager&       getViewManager();

protected:

  // Used only by the class to create aSingleton
  WindowGLV();

  static WindowGLV* aSingleton;

private:

  // Block the use of those
  WindowGLV(const WindowGLV&);
  WindowGLV& operator=(const WindowGLV&);

  ViewManager aViewManager;

};

#endif // WINDOW_H
