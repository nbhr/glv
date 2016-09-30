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

#ifndef GLUT_UTILS_H

#include "glinclude.h"
#include <string>

void drawText(const std::string& pText,
              float pXMin, float pYMin, float pXMax, float pYMax,
              int pFontSize, void* pCurrentFont, bool pAutoLineBreak, bool pFlagCentered);

void drawText3D(const std::string& pText, float pX, float pY, float pZ, void* pCurrentFont);

#endif // GLUT_UTILS_H
