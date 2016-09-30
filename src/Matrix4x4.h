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

#ifndef MATRIX4X4_H
#define MATRIX4X4_H

class Vector3D;
class Matrix4x4;

Vector3D operator*(const Matrix4x4& pMatrix4x4,
                   const Vector3D&  pVector3D);

// Simple 4x4 matrix to manipulate GL transformations
// in homogenous coordinates
class Matrix4x4 {

public:

  Matrix4x4 ();
  Matrix4x4 (const Matrix4x4& pMatrix4x4);

  ~Matrix4x4() {}

  Matrix4x4& operator=(const Matrix4x4& pMatrix4x4);

  void rotateAbout(const Vector3D& pVector3D,
                   const float     pAngleRadians);
  void translate  (const Vector3D& pTranslation);
  void scale      (const Vector3D& pScaling);

private:

  friend Vector3D operator*(const Matrix4x4& pMatrix4x4,
                            const Vector3D&  pVector3D);

  float aVals[3][4];

};

#endif // MATRIX4X4_H
