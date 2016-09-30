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

#ifndef VIEW_H
#define VIEW_H

#include "Vector3D.h"

#include <iostream>


class BoundingBox;
class Tile;
class ViewManager;

class View
{
public:

  View ();

  View (const View&         pView);

  View (const Vector3D&     pCenter);

  View (const Vector3D&     pCenter,
        const Vector3D&     pDirection);

  View (const Vector3D&     pCenter,
        const Vector3D&     pDirection,
        const Vector3D&     pUp,
        const float         pDistance,
        const float         pFOVDegrees);

  View (const BoundingBox& pBoundingBox);

  View (const BoundingBox& pBoundingBox,
        const Vector3D&    pDirection,
        const Vector3D&    pUp);

  ~View();

  void     drawCenter () const;

  void     dump       (ViewManager&  pViewManager) const;

  void     dump       (std::ostream& pOstream) const;

  Vector3D getPosition() const;

  void     initCamera (const Tile&   pTile) const;

  View&    operator=  (const View&   pView);

  void     rotate     (const float   pDeltaX,
                       const float   pDeltaY);

  void     scaleFOV   (const float   pFOVFactor);

  void     translate  (const float   pDeltaX,
                       const float   pDeltaY);

private:

  static const Vector3D aDefaultCenter;
  static const Vector3D aDefaultDirection;
  static const float    aDefaultDistance;
  static const float    aDefaultFOVDegrees;
  static const Vector3D aDefaultUp;
  static const float    aInitialObjectDistanceFactor;
  static const float    aMaxFOVDegrees;
  static const float    aMinFOVDegrees;

  void     ajust(const BoundingBox&  pBoundingBox);

  void     init (const Vector3D&     pCenter     = aDefaultCenter,
                 const Vector3D&     pDirection  = aDefaultDirection,
                 const Vector3D&     pUp         = aDefaultUp,
                 const float         pDistance   = aDefaultDistance,
                 const float         pFOVDegrees = aDefaultFOVDegrees);

#ifdef GLV_BEBUG
  void invariants() const;
#endif // #ifdef GLV_BEBUG

  Vector3D aCenter;
  Vector3D aDirection;
  float    aDistance;
  float    aFOVDegrees;
  Vector3D aUp;

};


#endif // VIEW_H
