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

#ifndef TILE_H
#define TILE_H

#include <string>

class Tile
{
public:

  Tile (const int pImageWidth,
        const int pImageHeight);

  Tile(const int pImageWidth,
       const int pImageHeight,
       const int pXMin,
       const int pXMax,
       const int pYMin,
       const int pYMax);

  ~Tile();

  void initFrustrumMatrix(const float pFOVDegrees,
                          const float pZNear,
                          const float pZFar) const;

  void initOrtho2DMatrix () const;

  void initViewport      () const;

private:

#ifdef GLV_BEBUG
  void invariants() const;
#endif // #ifdef GLV_BEBUG

  float aImageHeight;
  float aImageWidth;
  float aXMax;
  float aXMin;
  float aYMax;
  float aYMin;

};

#endif // TILE_H

