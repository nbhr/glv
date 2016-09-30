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

#include "WindowGLV.h"

WindowGLV* WindowGLV::aSingleton = 0;

WindowGLV::~WindowGLV()
{}

// Static method
// Get the only instance of type WindowGLV
// One and only one object must be created
// before calling this static member function
WindowGLV& WindowGLV::getInstance()
{
  GLV_ASSERT(aSingleton != 0);
  return *aSingleton;
}

ViewManager& WindowGLV::getViewManager()
{
  return aViewManager;
}

WindowGLV::WindowGLV()
  : aViewManager()
{
  // Only one WindowGLV can be used
  GLV_ASSERT(aSingleton == 0);

  aSingleton = this;
}
