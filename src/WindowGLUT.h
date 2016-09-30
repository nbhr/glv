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

#ifndef WINDOWGLUT_H
#define WINDOWGLUT_H

#include "WindowGLV.h"
#include "glut.h"

// GLUT toolkit implementation of a opengl render window
// Only one instance is permitted (see WindowGLV)
class WindowGLUT : public WindowGLV
{
public:

  static WindowGLUT& getInstance();

  WindowGLUT();
  virtual ~WindowGLUT();

  void idle    ();

  void key     (unsigned char pKey);

  void menu    (const char*   pMenuEvent);

  void mouse   (int           pButton,
                int           pState,
                int           pX,
                int           pY);

  void paintGL ();

  void resizeGL(int           pWidth,
                int           pHeight);

  void skey    (int           pKey);

private:

  // Block the use of those
  WindowGLUT(const WindowGLUT&);
  WindowGLUT& operator=(const WindowGLUT&);

};

#endif // WINDOWGLUT_H
