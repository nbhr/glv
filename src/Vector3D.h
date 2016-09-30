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

#ifndef VECTOR3D_H
#define VECTOR3D_H

#include "assert_glv.h"
#include <cmath>
#include <iosfwd>

//!  Simple 3D vector class
class Vector3D {

public:

  Vector3D () {aVals[0] = 0.0f; aVals[1] = 0.0f; aVals[2] = 0.0f;}
  ~Vector3D() {}

  Vector3D(float pX,float pY,float pZ) {
    aVals[0] = pX; aVals[1] = pY; aVals[2] = pZ;
  }

  Vector3D(const float *pVal);
  Vector3D(const Vector3D& pObj);
  Vector3D& operator=(const Vector3D& pObj);

  bool operator==(const Vector3D& pObj) const ;
  bool operator!=(const Vector3D& pObj) const ;

  Vector3D operator+(const Vector3D& pObj) const {
    return Vector3D(aVals[0]+pObj.aVals[0],
                    aVals[1]+pObj.aVals[1],
                    aVals[2]+pObj.aVals[2]);
  }

  Vector3D operator-(const Vector3D& pObj) const {
    return Vector3D(aVals[0]-pObj.aVals[0],
                    aVals[1]-pObj.aVals[1],
                    aVals[2]-pObj.aVals[2]);
  }

  Vector3D operator*(float pVal) const {
    return Vector3D(aVals[0]*pVal,
                    aVals[1]*pVal,
                    aVals[2]*pVal);
  }

  Vector3D operator/(float pVal) const;

  Vector3D& operator+=(const Vector3D& pObj);
  Vector3D& operator-=(const Vector3D& pObj);

  Vector3D& operator*=(float pVal) {
    aVals[0] *= pVal;
    aVals[1] *= pVal;
    aVals[2] *= pVal;
    return *this;
  }

  Vector3D& operator/=(float pVal);

  Vector3D crossProduct(const Vector3D& pObj) const;
  float dotProduct(const Vector3D& pObj) const;

  inline float x() const { return aVals[0];}
  inline float y() const { return aVals[1];}
  inline float z() const { return aVals[2];}

  float getLength() const {
    return sqrt(aVals[0]*aVals[0] + aVals[1]*aVals[1] + aVals[2]*aVals[2]);
  }

  float getSquaredLength() const;
  float normalize();
  Vector3D projectOnVector(const Vector3D& pObj);

  float getDistanceTo(const Vector3D& pObj) const {
    return (*this - pObj).getLength();
  }

  float getAngleTo(const Vector3D& pVect) const;

  float operator[](int pVal) const {
    GLV_ASSERT(pVal >= 0 && pVal < 3);
    return aVals[pVal];
  }

private:

  float aVals[3];

};

std::ostream&   operator<<(std::ostream& pStream,   const Vector3D& pVector3D);

inline Vector3D operator* (const float   pVal,      const Vector3D& pVector3D)
{
  return pVector3D*pVal;
}


#endif // VECTOR3D_H

