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

#include "Matrix4x4.h"
#include "assert_glv.h"
#include "Vector3D.h"


// By default, construct the indentity matrix
Matrix4x4::Matrix4x4()
{
  aVals[0][0] = 1.0f; aVals[0][1] = 0.0f; aVals[0][2] = 0.0f; aVals[0][3] = 0.0f;
  aVals[1][0] = 0.0f; aVals[1][1] = 1.0f; aVals[1][2] = 0.0f; aVals[1][3] = 0.0f;
  aVals[2][0] = 0.0f; aVals[2][1] = 0.0f; aVals[2][2] = 1.0f; aVals[2][3] = 0.0f;
}

Matrix4x4::Matrix4x4(const Matrix4x4& pMatrix4x4)
{
  aVals[0][0] = pMatrix4x4.aVals[0][0];
  aVals[0][1] = pMatrix4x4.aVals[0][1];
  aVals[0][2] = pMatrix4x4.aVals[0][2];
  aVals[0][3] = pMatrix4x4.aVals[0][3];

  aVals[1][0] = pMatrix4x4.aVals[1][0];
  aVals[1][1] = pMatrix4x4.aVals[1][1];
  aVals[1][2] = pMatrix4x4.aVals[1][2];
  aVals[1][3] = pMatrix4x4.aVals[1][3];

  aVals[2][0] = pMatrix4x4.aVals[2][0];
  aVals[2][1] = pMatrix4x4.aVals[2][1];
  aVals[2][2] = pMatrix4x4.aVals[2][2];
  aVals[2][3] = pMatrix4x4.aVals[2][3];
}

Matrix4x4& Matrix4x4::operator=(const Matrix4x4& pMatrix4x4)
{
  if (this != &pMatrix4x4) {
    aVals[0][0] = pMatrix4x4.aVals[0][0];
    aVals[0][1] = pMatrix4x4.aVals[0][1];
    aVals[0][2] = pMatrix4x4.aVals[0][2];
    aVals[0][3] = pMatrix4x4.aVals[0][3];

    aVals[1][0] = pMatrix4x4.aVals[1][0];
    aVals[1][1] = pMatrix4x4.aVals[1][1];
    aVals[1][2] = pMatrix4x4.aVals[1][2];
    aVals[1][3] = pMatrix4x4.aVals[1][3];

    aVals[2][0] = pMatrix4x4.aVals[2][0];
    aVals[2][1] = pMatrix4x4.aVals[2][1];
    aVals[2][2] = pMatrix4x4.aVals[2][2];
    aVals[2][3] = pMatrix4x4.aVals[2][3];
  }

  return *this;
}

// Multiplies to the right the current Matrix4x4
// by the rotation matrix of pAngleRadians about
// pVector3D
void Matrix4x4::rotateAbout(const Vector3D& pVector3D,
                            const float     pAngleRadians)
{
  const float lLength = pVector3D.getLength();

  GLV_ASSERT(lLength > 0.0f);

  // Only multiplies if lLength > 0.0f in order
  // for the code to still work when GLV_DEBUG
  // is not defined and if for some unknown
  // reason pVector3D.getLength() == 0.0f
  if (lLength > 0.0f) {

    const float co  = cos(pAngleRadians);
    const float si  = sin(pAngleRadians);
    const float vx  = pVector3D.x()/lLength;
    const float vy  = pVector3D.y()/lLength;
    const float vz  = pVector3D.z()/lLength;
    const float vxx = vx*vx;
    const float vyy = vy*vy;
    const float vzz = vz*vz;

    const float lRot00  = vxx+co*(1-vxx);
    const float lRot01  = vx*vy*(1-co)-vz*si;
    const float lRot02  = vz*vx*(1-co)+vy*si;
    const float lRot10  = vx*vy*(1-co)+vz*si;
    const float lRot11  = vyy+co*(1-vyy);
    const float lRot12  = vy*vz*(1-co)-vx*si;
    const float lRot20  = vz*vx*(1-co)-vy*si;
    const float lRot21  = vy*vz*(1-co)+vx*si;
    const float lRot22  = vzz+co*(1-vzz);

    const float lVals00 = aVals[0][0];
    const float lVals01 = aVals[0][1];
    const float lVals02 = aVals[0][2];
    const float lVals10 = aVals[1][0];
    const float lVals11 = aVals[1][1];
    const float lVals12 = aVals[1][2];
    const float lVals20 = aVals[2][0];
    const float lVals21 = aVals[2][1];
    const float lVals22 = aVals[2][2];

    aVals[0][0] = lVals00*lRot00 + lVals01*lRot10 + lVals02*lRot20;
    aVals[0][1] = lVals00*lRot01 + lVals01*lRot11 + lVals02*lRot21;
    aVals[0][2] = lVals00*lRot02 + lVals01*lRot12 + lVals02*lRot22;
    aVals[1][0] = lVals10*lRot00 + lVals11*lRot10 + lVals12*lRot20;
    aVals[1][1] = lVals10*lRot01 + lVals11*lRot11 + lVals12*lRot21;
    aVals[1][2] = lVals10*lRot02 + lVals11*lRot12 + lVals12*lRot22;
    aVals[2][0] = lVals20*lRot00 + lVals21*lRot10 + lVals22*lRot20;
    aVals[2][1] = lVals20*lRot01 + lVals21*lRot11 + lVals22*lRot21;
    aVals[2][2] = lVals20*lRot02 + lVals21*lRot12 + lVals22*lRot22;
  }
}

// Multiplies to the right the current Matrix4x4
// by the translation matrix.
void Matrix4x4::translate(const Vector3D& pTranslation)
{
  const float x = pTranslation.x();
  const float y = pTranslation.y();
  const float z = pTranslation.z();

  aVals[0][3] += aVals[0][0]*x + aVals[0][1]*y + aVals[0][2]*z;
  aVals[1][3] += aVals[1][0]*x + aVals[1][1]*y + aVals[1][2]*z;
  aVals[2][3] += aVals[2][0]*x + aVals[2][1]*y + aVals[2][2]*z;
}

// Multiplies to the right the current Matrix4x4
// by the scaling matrix.
void Matrix4x4::scale(const Vector3D& pScaling)
{
  const float x = pScaling.x();
  const float y = pScaling.y();
  const float z = pScaling.z();

  aVals[0][0] *= x; aVals[0][1] *= y; aVals[0][2] *= z;
  aVals[1][0] *= x; aVals[1][1] *= y; aVals[1][2] *= z;
  aVals[2][0] *= x; aVals[2][1] *= y; aVals[2][2] *= z;
}

// aVals is the GL transformation matrix
// Returns: aVals * pVector3D
Vector3D operator*(const Matrix4x4& pMatrix4x4,
                   const Vector3D&  pVector3D)
{
  const float x = pVector3D.x();
  const float y = pVector3D.y();
  const float z = pVector3D.z();

  return Vector3D(pMatrix4x4.aVals[0][0]*x + pMatrix4x4.aVals[0][1]*y + pMatrix4x4.aVals[0][2]*z + pMatrix4x4.aVals[0][3],
                  pMatrix4x4.aVals[1][0]*x + pMatrix4x4.aVals[1][1]*y + pMatrix4x4.aVals[1][2]*z + pMatrix4x4.aVals[1][3],
                  pMatrix4x4.aVals[2][0]*x + pMatrix4x4.aVals[2][1]*y + pMatrix4x4.aVals[2][2]*z + pMatrix4x4.aVals[2][3]);
}
