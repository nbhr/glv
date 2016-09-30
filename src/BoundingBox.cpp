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

#include "BoundingBox.h"
#include "assert_glv.h"
#include "limits_glv.h"
#include "Matrix4x4.h"

#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>

BoundingBox::BoundingBox()
  : aInitialized  (false),
    aMaxX         (std::numeric_limits<float>::min()),
    aMaxY         (std::numeric_limits<float>::min()),
    aMaxZ         (std::numeric_limits<float>::min()),
    aMinX         (std::numeric_limits<float>::max()),
    aMinY         (std::numeric_limits<float>::max()),
    aMinZ         (std::numeric_limits<float>::max())
{}

BoundingBox::BoundingBox(const BoundingBox& pBoundingBox)
  : aInitialized  (pBoundingBox.aInitialized),
    aMaxX         (pBoundingBox.aMaxX),
    aMaxY         (pBoundingBox.aMaxY),
    aMaxZ         (pBoundingBox.aMaxZ),
    aMinX         (pBoundingBox.aMinX),
    aMinY         (pBoundingBox.aMinY),
    aMinZ         (pBoundingBox.aMinZ)
{
  GLV_ASSERT(aMaxX >= aMinX || !aInitialized);
  GLV_ASSERT(aMaxY >= aMinY || !aInitialized);
  GLV_ASSERT(aMaxZ >= aMinZ || !aInitialized);
}

BoundingBox::BoundingBox(const Vector3D& pPoint)
  : aInitialized  (true),
    aMaxX         (pPoint.x()),
    aMaxY         (pPoint.y()),
    aMaxZ         (pPoint.z()),
    aMinX         (pPoint.x()),
    aMinY         (pPoint.y()),
    aMinZ         (pPoint.z())
{
  GLV_ASSERT(aMaxX >= aMinX);
  GLV_ASSERT(aMaxY >= aMinY);
  GLV_ASSERT(aMaxZ >= aMinZ);
}

BoundingBox::~BoundingBox()
{}


// Dump in ASCII the caracteristics of the BoundingBox
void BoundingBox::dumpCharacteristics(std::ostream&       pOstream,
                                      const std::string&  pIndentation,
                                      const Matrix4x4&    pTransformation)
{
  pOstream << pIndentation << "BoundingBox " << std::endl;
  std::string lIndentation = pIndentation + "  ";

#ifdef GLV_DUMP_MEMORY_USAGE

  pOstream << lIndentation << "Memory used by the BoundingBox = " << sizeof(*this) << std::endl;

#endif // #ifdef GLV_DUMP_MEMORY_USAGE

  if (aInitialized) {

    // Compute the transformed BoundingBox
    BoundingBox lTransformed = pTransformation * (*this);

    pOstream << lIndentation << "[MinX, MaxX] = [" << lTransformed.aMinX << ", " << lTransformed.aMaxX << "]" << std::endl;
    pOstream << lIndentation << "[MinY, MaxY] = [" << lTransformed.aMinY << ", " << lTransformed.aMaxY << "]" << std::endl;
    pOstream << lIndentation << "[MinZ, MaxZ] = [" << lTransformed.aMinZ << ", " << lTransformed.aMaxZ << "]" << std::endl;
  }
  else {
    pOstream << lIndentation << "Empty" << std::endl;
  }
}

void BoundingBox::render() const
{

  if (aInitialized) {

    GLV_ASSERT(aMaxX >= aMinX);
    GLV_ASSERT(aMaxY >= aMinY);
    GLV_ASSERT(aMaxZ >= aMinZ);

    // We push the attributes on the openGL attribute stack; so
    // we dont disturb the current values of color (current) or line width (line)
    glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
    // Warning: performance hit; when a lot of push(GL_LIGHTING_BIT) calls are made
    glDisable(GL_LIGHTING);
    glLineWidth(2);

    glBegin(GL_LINES);
    {
      glColor3f(0.7, 0.5, 0.1);

      glVertex3f(aMinX,aMinY,aMinZ); glVertex3f(aMaxX,aMinY,aMinZ);
      glVertex3f(aMinX,aMinY,aMinZ); glVertex3f(aMinX,aMaxY,aMinZ);
      glVertex3f(aMinX,aMinY,aMinZ); glVertex3f(aMinX,aMinY,aMaxZ);
      glVertex3f(aMaxX,aMinY,aMinZ); glVertex3f(aMaxX,aMaxY,aMinZ);
      glVertex3f(aMaxX,aMinY,aMinZ); glVertex3f(aMaxX,aMinY,aMaxZ);
      glVertex3f(aMaxX,aMaxY,aMinZ); glVertex3f(aMinX,aMaxY,aMinZ);
      glVertex3f(aMaxX,aMaxY,aMinZ); glVertex3f(aMaxX,aMaxY,aMaxZ);
      glVertex3f(aMinX,aMaxY,aMinZ); glVertex3f(aMinX,aMaxY,aMaxZ);
      glVertex3f(aMinX,aMinY,aMaxZ); glVertex3f(aMaxX,aMinY,aMaxZ);
      glVertex3f(aMinX,aMinY,aMaxZ); glVertex3f(aMinX,aMaxY,aMaxZ);
      glVertex3f(aMaxX,aMinY,aMaxZ); glVertex3f(aMaxX,aMaxY,aMaxZ);
      glVertex3f(aMinX,aMaxY,aMaxZ); glVertex3f(aMaxX,aMaxY,aMaxZ);
    }
    glEnd();

    // Revert the lighting state and the line state
    glPopAttrib();
  }
}

Vector3D BoundingBox::getCenter() const
{
  Vector3D lCenter(0.0f, 0.0f, 0.0f);

  if (aInitialized) {
    GLV_ASSERT(aMaxX >= aMinX);
    GLV_ASSERT(aMaxY >= aMinY);
    GLV_ASSERT(aMaxZ >= aMinZ);

    lCenter = Vector3D(0.5*(aMaxX + aMinX),
                       0.5*(aMaxY + aMinY),
                       0.5*(aMaxZ + aMinZ));
  }
  return lCenter;
}

float BoundingBox::getCircumscribedSphereRadius() const
{
  float lRadius = 0.0f;

  if (aInitialized) {
    GLV_ASSERT(aMaxX >= aMinX);
    GLV_ASSERT(aMaxY >= aMinY);
    GLV_ASSERT(aMaxZ >= aMinZ);

    float lDX = 0.5*(aMaxX - aMinX);
    float lDY = 0.5*(aMaxY - aMinY);
    float lDZ = 0.5*(aMaxZ - aMinZ);
    lRadius = sqrt(lDX*lDX + lDY*lDY + lDZ*lDZ);
  }
  return lRadius;
}

BoundingBox& BoundingBox::operator=(const BoundingBox& pBoundingBox)
{
  if (&pBoundingBox != this) {

    aInitialized = pBoundingBox.aInitialized;
    aMaxX        = pBoundingBox.aMaxX;
    aMaxY        = pBoundingBox.aMaxY;
    aMaxZ        = pBoundingBox.aMaxZ;
    aMinX        = pBoundingBox.aMinX;
    aMinY        = pBoundingBox.aMinY;
    aMinZ        = pBoundingBox.aMinZ;

    GLV_ASSERT(aMaxX >= aMinX || !aInitialized);
    GLV_ASSERT(aMaxY >= aMinY || !aInitialized);
    GLV_ASSERT(aMaxZ >= aMinZ || !aInitialized);
  }

  return *this;
}

BoundingBox BoundingBox::operator+(const BoundingBox& pBoundingBox) const
{
  BoundingBox lBoundingBox;

  if (aInitialized && pBoundingBox.aInitialized) {
    lBoundingBox.aMaxX = std::max(aMaxX, pBoundingBox.aMaxX);
    lBoundingBox.aMaxY = std::max(aMaxY, pBoundingBox.aMaxY);
    lBoundingBox.aMaxZ = std::max(aMaxZ, pBoundingBox.aMaxZ);
    lBoundingBox.aMinX = std::min(aMinX, pBoundingBox.aMinX);
    lBoundingBox.aMinY = std::min(aMinY, pBoundingBox.aMinY);
    lBoundingBox.aMinZ = std::min(aMinZ, pBoundingBox.aMinZ);
    lBoundingBox.aInitialized = true;

    GLV_ASSERT(lBoundingBox.aMaxX >= lBoundingBox.aMinX);
    GLV_ASSERT(lBoundingBox.aMaxY >= lBoundingBox.aMinY);
    GLV_ASSERT(lBoundingBox.aMaxZ >= lBoundingBox.aMinZ);
  }
  else if (aInitialized) {
    lBoundingBox.aMaxX = aMaxX;
    lBoundingBox.aMaxY = aMaxY;
    lBoundingBox.aMaxZ = aMaxZ;
    lBoundingBox.aMinX = aMinX;
    lBoundingBox.aMinY = aMinY;
    lBoundingBox.aMinZ = aMinZ;
    lBoundingBox.aInitialized = true;

    GLV_ASSERT(lBoundingBox.aMaxX >= lBoundingBox.aMinX);
    GLV_ASSERT(lBoundingBox.aMaxY >= lBoundingBox.aMinY);
    GLV_ASSERT(lBoundingBox.aMaxZ >= lBoundingBox.aMinZ);
  }
  else if (pBoundingBox.aInitialized) {
    lBoundingBox.aMaxX = pBoundingBox.aMaxX;
    lBoundingBox.aMaxY = pBoundingBox.aMaxY;
    lBoundingBox.aMaxZ = pBoundingBox.aMaxZ;
    lBoundingBox.aMinX = pBoundingBox.aMinX;
    lBoundingBox.aMinY = pBoundingBox.aMinY;
    lBoundingBox.aMinZ = pBoundingBox.aMinZ;
    lBoundingBox.aInitialized = true;

    GLV_ASSERT(lBoundingBox.aMaxX >= lBoundingBox.aMinX);
    GLV_ASSERT(lBoundingBox.aMaxY >= lBoundingBox.aMinY);
    GLV_ASSERT(lBoundingBox.aMaxZ >= lBoundingBox.aMinZ);
  }

  return lBoundingBox;
}

BoundingBox BoundingBox::operator+(const Vector3D& pPoint) const
{
  BoundingBox lBoundingBox;

  if (aInitialized) {
    lBoundingBox.aMaxX = std::max(aMaxX, pPoint.x());
    lBoundingBox.aMaxY = std::max(aMaxY, pPoint.y());
    lBoundingBox.aMaxZ = std::max(aMaxZ, pPoint.z());
    lBoundingBox.aMinX = std::min(aMinX, pPoint.x());
    lBoundingBox.aMinY = std::min(aMinY, pPoint.y());
    lBoundingBox.aMinZ = std::min(aMinZ, pPoint.z());
  }
  else {
    lBoundingBox.aMaxX = pPoint.x();
    lBoundingBox.aMaxY = pPoint.y();
    lBoundingBox.aMaxZ = pPoint.z();
    lBoundingBox.aMinX = pPoint.x();
    lBoundingBox.aMinY = pPoint.y();
    lBoundingBox.aMinZ = pPoint.z();
  }

  lBoundingBox.aInitialized = true;

  GLV_ASSERT(lBoundingBox.aMaxX >= lBoundingBox.aMinX);
  GLV_ASSERT(lBoundingBox.aMaxY >= lBoundingBox.aMinY);
  GLV_ASSERT(lBoundingBox.aMaxZ >= lBoundingBox.aMinZ);

  return lBoundingBox;
}

BoundingBox& BoundingBox::operator+=(const BoundingBox& pBoundingBox)
{
  if (aInitialized && pBoundingBox.aInitialized) {
    aMaxX = std::max(aMaxX, pBoundingBox.aMaxX);
    aMaxY = std::max(aMaxY, pBoundingBox.aMaxY);
    aMaxZ = std::max(aMaxZ, pBoundingBox.aMaxZ);
    aMinX = std::min(aMinX, pBoundingBox.aMinX);
    aMinY = std::min(aMinY, pBoundingBox.aMinY);
    aMinZ = std::min(aMinZ, pBoundingBox.aMinZ);

    GLV_ASSERT(aMaxX >= aMinX);
    GLV_ASSERT(aMaxY >= aMinY);
    GLV_ASSERT(aMaxZ >= aMinZ);
  }
  else if (pBoundingBox.aInitialized) {
    aMaxX = pBoundingBox.aMaxX;
    aMaxY = pBoundingBox.aMaxY;
    aMaxZ = pBoundingBox.aMaxZ;
    aMinX = pBoundingBox.aMinX;
    aMinY = pBoundingBox.aMinY;
    aMinZ = pBoundingBox.aMinZ;
    aInitialized = true;

    GLV_ASSERT(aMaxX >= aMinX);
    GLV_ASSERT(aMaxY >= aMinY);
    GLV_ASSERT(aMaxZ >= aMinZ);
  }

  return *this;
}

BoundingBox& BoundingBox::operator+=(const Vector3D& pPoint)
{
  if (aInitialized) {
    aMaxX = std::max(aMaxX, pPoint.x());
    aMaxY = std::max(aMaxY, pPoint.y());
    aMaxZ = std::max(aMaxZ, pPoint.z());
    aMinX = std::min(aMinX, pPoint.x());
    aMinY = std::min(aMinY, pPoint.y());
    aMinZ = std::min(aMinZ, pPoint.z());
  }
  else {
    aMaxX = pPoint.x();
    aMaxY = pPoint.y();
    aMaxZ = pPoint.z();
    aMinX = pPoint.x();
    aMinY = pPoint.y();
    aMinZ = pPoint.z();
    aInitialized = true;
  }

  GLV_ASSERT(aMaxX >= aMinX);
  GLV_ASSERT(aMaxY >= aMinY);
  GLV_ASSERT(aMaxZ >= aMinZ);

  return *this;
}

// Transform all the corners of the BoundingBox
// and modify the current BoundingBox to include all the
// transformed points. This is overkill if we only use translations
// and scalings, but it will work for any linear transformation
// if we add support to let's say glrotate.
BoundingBox operator*(const Matrix4x4& pMatrix4x4, const BoundingBox& pBoundingBox)
{
  BoundingBox lBoundingBox;

  if (pBoundingBox.aInitialized) {

    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMinX, pBoundingBox.aMinY, pBoundingBox.aMinZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMaxX, pBoundingBox.aMinY, pBoundingBox.aMinZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMaxX, pBoundingBox.aMaxY, pBoundingBox.aMinZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMinX, pBoundingBox.aMaxY, pBoundingBox.aMinZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMinX, pBoundingBox.aMinY, pBoundingBox.aMaxZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMaxX, pBoundingBox.aMinY, pBoundingBox.aMaxZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMaxX, pBoundingBox.aMaxY, pBoundingBox.aMaxZ);
    lBoundingBox += pMatrix4x4 * Vector3D(pBoundingBox.aMinX, pBoundingBox.aMaxY, pBoundingBox.aMaxZ);
  }

  return lBoundingBox;
}
