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

#include "View.h"
#include "BoundingBox.h"
#include "Matrix4x4.h"
#include "Tile.h"
#include "Vector3D.h"
#include "ViewManager.h"

#include <stdio.h>

const Vector3D View::aDefaultCenter              (0.0f, 0.0f, 0.0f);
const Vector3D View::aDefaultDirection           (0.5f, sqrt(0.5), 0.5f);
const float    View::aDefaultDistance            (5.0f);
const float    View::aDefaultFOVDegrees          (40.0f);
const Vector3D View::aDefaultUp                  (-0.5f, sqrt(0.5), -0.5f);
const float    View::aInitialObjectDistanceFactor(3.0f);
const float    View::aMaxFOVDegrees              (60.0f);
const float    View::aMinFOVDegrees              (0.001f);


#ifdef GLV_BEBUG
#define GLV_CHECK_INVARIANTS invariants()
#else
#define GLV_CHECK_INVARIANTS
#endif // #ifdef GLV_BEBUG

View::View()
{
  init();
  GLV_CHECK_INVARIANTS;
}

View::View(const View& pView)
{
  init(pView.aCenter, pView.aDirection, pView.aUp, pView.aDistance, pView.aFOVDegrees);
}

View::View(const Vector3D& pCenter)
{
  init(pCenter);
}

View::View(const Vector3D& pCenter,
           const Vector3D& pDirection)
{
  init(pCenter, pDirection);
}

View::View(const Vector3D& pCenter,
           const Vector3D& pDirection,
           const Vector3D& pUp,
           const float     pDistance,
           const float     pFOVDegrees)
{
  init(pCenter, pDirection, pUp, pDistance, pFOVDegrees);
}

View::View(const BoundingBox& pBoundingBox)
{
  init ();
  ajust(pBoundingBox);
}

View::View(const BoundingBox& pBoundingBox,
           const Vector3D&    pDirection,
           const Vector3D&    pUp)
{
  init (Vector3D(0.0f, 0.0f, 0.0f), pDirection, pUp);
  ajust(pBoundingBox);
}

View::~View()
{}

// Draw a small orange axis at the center of the View
void View::drawCenter() const
{
  GLV_CHECK_INVARIANTS;

  glBegin(GL_LINES);

  glColor3f(0.7,0.5,0.1);

  const float lSize = 0.05f*aDistance;
  const float lX    = aCenter.x();
  const float lY    = aCenter.y();
  const float lZ    = aCenter.z();

  glVertex3f(lX-lSize, lY      , lZ      );
  glVertex3f(lX+lSize, lY      , lZ      );
  glVertex3f(lX      , lY-lSize, lZ      );
  glVertex3f(lX      , lY+lSize, lZ      );
  glVertex3f(lX      , lY      , lZ-lSize);
  glVertex3f(lX      , lY      , lZ+lSize);

  glEnd();

  GLV_CHECK_INVARIANTS;
}

// Dump the view on as a status message in the ViewManager
void View::dump(ViewManager& pViewManager) const
{
  GLV_CHECK_INVARIANTS;

  char lViewString[1024];
  sprintf(lViewString," view center = %+f %+f %+f", aCenter.x(), aCenter.y(), aCenter.z());
  pViewManager.addStatusMessage(lViewString);
  sprintf(lViewString," view direction = %+f %+f %+f", aDirection.x(), aDirection.y(), aDirection.z());
  pViewManager.addStatusMessage(lViewString);
  sprintf(lViewString," view up direction =  %+f %+f %+f", aUp.x(), aUp.y(),aUp.z());
  pViewManager.addStatusMessage(lViewString);
  sprintf(lViewString," distance from center = %+f", aDistance);
  pViewManager.addStatusMessage(lViewString);
  sprintf(lViewString," field of view in degrees = %+f", aFOVDegrees);
  pViewManager.addStatusMessage(lViewString);

  GLV_CHECK_INVARIANTS;
}

// Dump the view on an std::ostream
void View::dump(std::ostream& pOstream) const
{
  GLV_CHECK_INVARIANTS;

  char lViewString[1024];
  sprintf(lViewString,"view %+f %+f %+f %+f %+f %+f %+f %+f %+f %+f %+f",
          aCenter.x(),aCenter.y(),aCenter.z(),
          aDirection.x(),aDirection.y(),aDirection.z(),
          aUp.x(),aUp.y(),aUp.z(),
          aDistance,aFOVDegrees);

  pOstream << lViewString << std::endl;

  GLV_CHECK_INVARIANTS;
}

// Returns the position of the camera
Vector3D View::getPosition() const
{
  GLV_CHECK_INVARIANTS;

  return  aCenter + aDistance*aDirection;
}

void View::initCamera(const Tile& pTile) const
{
  GLV_CHECK_INVARIANTS;

  pTile.initFrustrumMatrix(aFOVDegrees,
                           aDistance/10.0f,
                           aDistance*10.0f);

  glMatrixMode  (GL_MODELVIEW);
  glLoadIdentity();

  const Vector3D lAt = aCenter + aDistance*aDirection;

  gluLookAt(lAt    .x(), lAt    .y(), lAt    .z(),
            aCenter.x(), aCenter.y(), aCenter.z(),
            aUp    .x(), aUp    .y(), aUp    .z());

  GLV_CHECK_INVARIANTS;
}

View& View::operator=(const View& pView)
{
  GLV_CHECK_INVARIANTS;

  if(&pView != this) {
    init(pView.aCenter, pView.aDirection, pView.aUp, pView.aDistance, pView.aFOVDegrees);
  }

  GLV_CHECK_INVARIANTS;
  return *this;
}

void View::translate(const float pDeltaX, const float pDeltaY)
{
  GLV_CHECK_INVARIANTS;

  const Vector3D lOrtho = aUp.crossProduct(aDirection);
  const float    lCoef  = aDistance*aFOVDegrees/45.0f;

  aCenter -= lCoef*pDeltaX*lOrtho;
  aCenter -= lCoef*pDeltaY*aUp;

  GLV_CHECK_INVARIANTS;
}

void View::rotate(const float pDeltaX, const float pDeltaY)
{
  GLV_CHECK_INVARIANTS;

  const Vector3D lOrtho = aUp.crossProduct(aDirection);

  Matrix4x4 lMatrixY;
  lMatrixY.rotateAbout(lOrtho, pDeltaY);

  aUp        = lMatrixY*aUp;
  aDirection = lMatrixY*aDirection;

  Matrix4x4 lMatrixX;
  lMatrixX.rotateAbout(aUp, pDeltaX);

  aDirection = lMatrixX*aDirection;

  aUp       .normalize();
  aDirection.normalize();

  GLV_CHECK_INVARIANTS;
}

void View::scaleFOV(const float pFOVFactor)
{
  GLV_CHECK_INVARIANTS;

  aFOVDegrees *= pFOVFactor;
  aFOVDegrees = std::max(aFOVDegrees, aMinFOVDegrees);
  aFOVDegrees = std::min(aFOVDegrees, aMaxFOVDegrees);

  GLV_CHECK_INVARIANTS;
}

// Used internally by all the constructors using a BoundingBox
// This is called after the View::init function in those constructors
void View::ajust(const BoundingBox& pBoundingBox)
{
  GLV_CHECK_INVARIANTS;

  aCenter       = pBoundingBox.getCenter();
  float lRadius = pBoundingBox.getCircumscribedSphereRadius();

  if(lRadius == 0.0f) {
    lRadius = 1.0f;
  }

  aDistance = lRadius*aInitialObjectDistanceFactor;

  GLV_CHECK_INVARIANTS;
}

// Used internally by all the constructors
void View::init(const Vector3D& pCenter,
                const Vector3D& pDirection,
                const Vector3D& pUp,
                const float     pDistance,
                const float     pFOVDegrees)
{
  aCenter     = pCenter;
  aDirection  = pDirection;
  aUp         = pUp;
  aDistance   = pDistance;
  aFOVDegrees = pFOVDegrees;

  aDirection.normalize();

  // Make sure that the aDirection and aUp are perpendicular
  if((aUp.crossProduct(aDirection)).getLength() < 1.0E-3) {

    // If pUp is close to being parallel to pDirection, then we make an
    // arbitraty choice of aUp direction
    // We take the one of the Vector3D (1,0,0), (0,1,0) and (0,0,1)
    // that has the a dotProduct with aDirection closest to zero
    const float lAbsX = fabs(aDirection.x());
    const float lAbsY = fabs(aDirection.y());
    const float lAbsZ = fabs(aDirection.z());

    if (lAbsX <= lAbsY && lAbsX <= lAbsZ) {
      GLV_ASSERT(lAbsX == std::min(std::min(lAbsX,lAbsY), lAbsZ));
      aUp = Vector3D(1.0, 0.0, 0.0);
    }
    else if (lAbsY <= lAbsZ) {
      GLV_ASSERT(lAbsY == std::min(std::min(lAbsX,lAbsY), lAbsZ));
      aUp = Vector3D(0.0, 1.0, 0.0);
    }
    else {
      GLV_ASSERT(lAbsZ == std::min(std::min(lAbsX,lAbsY), lAbsZ));
      aUp = Vector3D(0.0, 0.0, 1.0);
    }
  }

  // Use Gram-Schmidt to get aUp perpendicular to aDirection
  aUp = aUp - (aUp.dotProduct(aDirection)/aDirection.dotProduct(aDirection))*aDirection;
  aUp.normalize();

  GLV_CHECK_INVARIANTS;
}

#ifdef GLV_BEBUG
void View::invariants() const
{
  GLV_ASSERT(aDistance   >  0.0f);
  GLV_ASSERT(aFOVDegrees >= aMinFOVDegrees);
  GLV_ASSERT(aFOVDegrees <= aMaxFOVDegrees);
  GLV_ASSERT(fabs(aDirection.getLength() - 1.0f) < 1.0e-6);
  GLV_ASSERT(fabs(aUp       .getLength() - 1.0f) < 1.0e-6);
  GLV_ASSERT(fabs((aUp.crossProduct(aDirection)).getLength() - 1.0f) < 1.0e-6);
}
#endif // #ifdef GLV_BEBUG
