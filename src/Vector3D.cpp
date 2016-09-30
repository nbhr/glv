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

#include "Vector3D.h"
#include "assert_glv.h"
#include <iostream>

// Copy constructor
Vector3D::Vector3D(const Vector3D& pObj)
{
  aVals[0] = pObj.x();
  aVals[1] = pObj.y();
  aVals[2] = pObj.z();
}

// operator=
Vector3D& Vector3D::operator=(const Vector3D& pObj)
{

  aVals[0] = pObj.x();
  aVals[1] = pObj.y();
  aVals[2] = pObj.z();

  return *this;
}

// Constructor with arguments
//  arguments: coordinates float[3]
Vector3D::Vector3D(const float *pVal)
{

  aVals[0] = pVal[0];
  aVals[1] = pVal[1];
  aVals[2] = pVal[2];

}

// Return the squared length
//  (faster; need no sqrt() in computation)
float Vector3D::getSquaredLength() const
{

  return aVals[0]*aVals[0] + aVals[1]*aVals[1] + aVals[2]*aVals[2];
}

// Normalize the vector to the unit vector
//  return: the length before normalization
float Vector3D::normalize()
{

  float lLength = getLength();
  aVals[0] /= lLength;
  aVals[1] /= lLength;
  aVals[2] /= lLength;

  return lLength;
}

// operator/
//   divide earch coordinate element by the value.
Vector3D Vector3D::operator/(float pVal) const
{
  return Vector3D(aVals[0]/pVal,aVals[1]/pVal,aVals[2]/pVal);
}

// cross product
Vector3D Vector3D::crossProduct(const Vector3D& pObj) const
{
  return Vector3D(aVals[1]*pObj.z() - aVals[2]*pObj.y(),aVals[2]*pObj.x() - aVals[0]*pObj.z(),aVals[0]*pObj.y() - aVals[1]*pObj.x());
}

// dot product
float Vector3D::dotProduct(const Vector3D& pObj) const
{
  return aVals[0]*pObj.x()+aVals[1]*pObj.y()+aVals[2]*pObj.z();
}

// operator+=   Adds the given vector to the current vector
Vector3D& Vector3D::operator+=(const Vector3D& pObj)
{

  aVals[0] += pObj.x();
  aVals[1] += pObj.y();
  aVals[2] += pObj.z();

  return *this;
}

// operator-=   Substracts the given vector to the current vector
Vector3D& Vector3D::operator-=(const Vector3D& pObj)
{

  aVals[0] -= pObj.x();
  aVals[1] -= pObj.y();
  aVals[2] -= pObj.z();
  return *this;
}

// operator/=
Vector3D& Vector3D::operator/=(float pVal)
{
  aVals[0] /= pVal;
  aVals[1] /= pVal;
  aVals[2] /= pVal;
  return *this;
}

// Return the angle between the current vector; and the target vector.
float Vector3D::getAngleTo(const Vector3D& pVect) const
{
  float lNorm = getLength();
  float lVectNorm = pVect.getLength();
  if(lNorm==0 || lVectNorm==0)
    return 0;

  float lDotProduct = dotProduct(pVect);
  float lQuotient = lDotProduct/(lNorm*lVectNorm);
  float lAngle = acos(lQuotient);

  return lAngle;
}

// Orthogonal projection
Vector3D Vector3D::projectOnVector(const Vector3D& pObj)
{
  float lDotProduct = dotProduct(pObj);
  float lDotProductSelf = pObj.dotProduct(pObj);
  Vector3D lVectResult = pObj*(lDotProduct/lDotProductSelf);
  return lVectResult;
}

// Utilitary: ostream operator
std::ostream& operator<<(std::ostream& pStream, const Vector3D& pVector3D)
{
  pStream << pVector3D.x() << " " << pVector3D.y() << " " << pVector3D.z();
  return pStream;
}

bool Vector3D::operator==(const Vector3D& pObj) const
{
  return aVals[0] == pObj.aVals[0] && aVals[1] == pObj.aVals[1] && aVals[2] == pObj.aVals[2];
}

bool Vector3D::operator!=(const Vector3D& pObj) const
{
  return aVals[0] != pObj.aVals[0] || aVals[1] != pObj.aVals[1] || aVals[2] != pObj.aVals[2];
}
