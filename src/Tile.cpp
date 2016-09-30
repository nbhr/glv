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

#define _USE_MATH_DEFINES

#include "Tile.h"
#include "assert_glv.h"
#include "glinclude.h"

#include <cmath>

#ifdef GLV_BEBUG
#define GLV_CHECK_INVARIANTS invariants()
#else
#define GLV_CHECK_INVARIANTS
#endif // #ifdef GLV_BEBUG


Tile::Tile(const int pImageWidth,
           const int pImageHeight)
  : aImageHeight(pImageHeight),
    aImageWidth (pImageWidth),
    aXMax       (pImageWidth - 1),
    aXMin       (0),
    aYMax       (pImageHeight - 1),
    aYMin       (0)
{
  GLV_ASSERT(pImageWidth  > 0);
  GLV_ASSERT(pImageHeight > 0);
  GLV_CHECK_INVARIANTS;
}

// A tile is not limited to the Image region, meaning
// that we can have:
//   pXMin < 0.0
//   pXMax > pImageWidth
//   pYMin < 0.0
//   pYMax > pImageHeight
Tile::Tile(const int pImageWidth,
           const int pImageHeight,
           const int pXMin,
           const int pXMax,
           const int pYMin,
           const int pYMax)
  : aImageHeight(pImageHeight),
    aImageWidth (pImageWidth),
    aXMax       (pXMax),
    aXMin       (pXMin),
    aYMax       (pYMax),
    aYMin       (pYMin)
{
  GLV_ASSERT(pImageWidth  > 0);
  GLV_ASSERT(pImageHeight > 0);
  GLV_ASSERT(pXMax > pXMin);
  GLV_ASSERT(pYMax > pYMin);
  GLV_CHECK_INVARIANTS;
}

Tile::~Tile()
{
  GLV_CHECK_INVARIANTS;
}

void Tile::initFrustrumMatrix(const float pFOVDegrees,
                              const float pZNear,
                              const float pZFar) const
{
  GLV_CHECK_INVARIANTS;
  GLV_ASSERT(pFOVDegrees > 0.0);
  GLV_ASSERT(pZNear      > 0.0);
  GLV_ASSERT(pZFar       > pZNear);

  glMatrixMode  (GL_PROJECTION);
  glLoadIdentity();

  // Replace the call to gluPerspective to the
  // equivalent call to glFrustum in order to be
  // able to select a section of the original image
  //   gluPerspective(pFOVDegrees,
  //                  aImageWidth/aImageHeight,
  //                  pZNear,
  //                  pZFar);
  // The equivalent parameters to glFrustum are the following
  //   glFrustum(lXMin, lXMax, lYMin, lYMax, lZNear, lZFar);
  // with the following values:
  const float lAspectRatio = aImageWidth/aImageHeight;
  const float lYMax        = pZNear*tan(pFOVDegrees*M_PI/360.0);
  const float lYMin        = -lYMax;
  const float lXMax        = lYMax * lAspectRatio;
  const float lXMin        = lYMin * lAspectRatio;

  // Compute new min/max to only display the tile
  // We have to keep in mind that OpenGL has the origin
  // at the lower left corner
  const float lOpenGLTileXMin = aXMin;
  const float lOpenGLTileXMax = aXMax + 1;
  const float lOpenGLTileYMax = aImageHeight - aYMin;
  const float lOpenGLTileYMin = aImageHeight - aYMax - 1;

  const float lXMinTile = (lOpenGLTileXMin/aImageWidth )*(lXMax-lXMin) + lXMin;
  const float lXMaxTile = (lOpenGLTileXMax/aImageWidth )*(lXMax-lXMin) + lXMin;
  const float lYMinTile = (lOpenGLTileYMin/aImageHeight)*(lYMax-lYMin) + lYMin;
  const float lYMaxTile = (lOpenGLTileYMax/aImageHeight)*(lYMax-lYMin) + lYMin;

  glFrustum(lXMinTile, lXMaxTile, lYMinTile, lYMaxTile, pZNear, pZFar);

  GLV_CHECK_INVARIANTS;
}

void Tile::initOrtho2DMatrix() const
{
  GLV_CHECK_INVARIANTS;

  // We have to keep in mind that OpenGL has the origin
  // at the lower left corner
  const float lOpenGLTileXMin = aXMin;
  const float lOpenGLTileXMax = aXMax + 1;
  const float lOpenGLTileYMax = aImageHeight - aYMin;
  const float lOpenGLTileYMin = aImageHeight - aYMax - 1;

  const float lXMinTile = (lOpenGLTileXMin/aImageWidth);
  const float lXMaxTile = (lOpenGLTileXMax/aImageWidth);
  const float lYMinTile = (lOpenGLTileYMin/aImageHeight);
  const float lYMaxTile = (lOpenGLTileYMax/aImageHeight);

  glMatrixMode  (GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D    (lXMinTile, lXMaxTile, lYMinTile, lYMaxTile);

  GLV_CHECK_INVARIANTS;
}

void Tile::initViewport() const
{
  GLV_CHECK_INVARIANTS;
  glViewport(0, 0, static_cast<int>(aXMax-aXMin+1), static_cast<int>(aYMax-aYMin+1));
  GLV_CHECK_INVARIANTS;
}

#ifdef GLV_BEBUG
void Tile::invariants() const
{
  GLV_ASSERT(aImageWidth  >= 1.0);
  GLV_ASSERT(aImageHeight >= 1.0);
  GLV_ASSERT(aXMax        >  aXMin);
  GLV_ASSERT(aYMax        >  aYMin);
}
#endif // #ifdef GLV_BEBUG
