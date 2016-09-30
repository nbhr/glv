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

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "glinclude.h"
#include "Matrix4x4.h"
#include "Vector3D.h"
#include <string>


class BoundingBox;

BoundingBox operator*(const Matrix4x4&   pMatrix4x4,
                      const BoundingBox& pBoundingBox);


// Class to manipulate and display a bounding box
class BoundingBox
{
public:

  BoundingBox         ();
  BoundingBox         (const BoundingBox& pBoundingBox);
  explicit BoundingBox(const Vector3D&    pPoint);
  ~BoundingBox        ();

  void         dumpCharacteristics         (std::ostream&      pOstream,
                                            const std::string& pIndentation,
                                            const Matrix4x4&   pTransformation);

  void         render                      () const;

  Vector3D     getCenter                   () const;
  float        getCircumscribedSphereRadius() const;

  BoundingBox&  operator= (const BoundingBox& pBoundingBox);
  BoundingBox   operator+ (const BoundingBox& pBoundingBox) const;
  BoundingBox   operator+ (const Vector3D&    pPoint      ) const;
  BoundingBox&  operator+=(const BoundingBox& pBoundingBox);
  BoundingBox&  operator+=(const Vector3D&    pPoint      );

private:

  friend BoundingBox operator*(const Matrix4x4&   pMatrix4x4,
                               const BoundingBox& pBoundingBox);

  bool            aInitialized;
  float           aMaxX;
  float           aMaxY;
  float           aMaxZ;
  float           aMinX;
  float           aMinY;
  float           aMinZ;

};


#endif // BOUNDINGBOX_H

