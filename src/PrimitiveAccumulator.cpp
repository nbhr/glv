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

#include "PrimitiveAccumulator.h"
#include "assert_glv.h"
#include "limits_glv.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdio.h>

#ifdef WIN32
// PI is not defined in visual c headers
#define M_PI 3.14159265358979323846
#endif

PrimitiveAccumulator::PrimitiveAccumulator(const bool pCreateSimplified)
  : aBoundingBox                       (),
    aLines                             (),
    aLinesBBoxCounter                  (0),
    aLinesColored                      (),
    aLinesColoredBBoxCounter           (0),
    aPoints                            (),
    aPointsBBoxCounter                 (0),
    aPointsColored                     (),
    aPointsColoredBBoxCounter          (0),
    aQuads                             (),
    aQuadsBBoxCounter                  (0),
    aQuadsColored                      (),
    aQuadsColoredBBoxCounter           (0),
    aQuadsNormals                      (),
    aQuadsNormalsBBoxCounter           (0),
    aQuadsNormalsColored               (),
    aQuadsNormalsColoredBBoxCounter    (0),
    aSimplified                        (),
    aSimplifiedDirty                   (true),
    aTriangles                         (),
    aTrianglesBBoxCounter              (0),
    aTrianglesColored                  (),
    aTrianglesColoredBBoxCounter       (0),
    aTrianglesNormals                  (),
    aTrianglesNormalsBBoxCounter       (0),
    aTrianglesNormalsColored           (),
    aTrianglesNormalsColoredBBoxCounter(0),
    aPrimitiveOptimizerValue           (100)
{
  if (pCreateSimplified) {
    aSimplified = new PrimitiveAccumulator(false);
  }
  else {
    aSimplified = this;
  }
  GLV_ASSERT(aSimplified != 0);
}

PrimitiveAccumulator::~PrimitiveAccumulator()
{
  GLV_ASSERT(aSimplified != 0);

  if (aSimplified != this) {
    delete aSimplified;
  }
}

void PrimitiveAccumulator::addArrow(const Vector3D& pP1,
                                    const Vector3D& pP2,
                                    const float     pTipProportion,
                                    const int       pTipNbPolygons)
{
  GLV_ASSERT(pTipProportion  > 0.0f);
  GLV_ASSERT(pTipProportion <= 1.0f);
  GLV_ASSERT(pTipNbPolygons >= 1);

  addLine(pP1, pP2);

  addArrowTip(pP1, pP2, pP2, false, pTipProportion, pTipNbPolygons);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addArrowColored(const Vector3D& pP1,
                                           const Vector3D& pC1,
                                           const Vector3D& pP2,
                                           const Vector3D& pC2,
                                           const float     pTipProportion,
                                           const int       pTipNbPolygons)
{
  GLV_ASSERT(pTipProportion  > 0.0f);
  GLV_ASSERT(pTipProportion <= 1.0f);
  GLV_ASSERT(pTipNbPolygons >= 1);

  addLineColored(pP1, pC1, pP2, pC2);

  addArrowTip(pP1, pP2, pC2, true, pTipProportion, pTipNbPolygons);

  aSimplifiedDirty = true;
}

// We now do the equivalent of the glut commands in
// the PrimitiveAccumulator to take adventage of all
// it has to offer (BoundingBox updated, optimized diplay, etc)
void PrimitiveAccumulator::addSolidCube(const Vector3D& pCenter,
                                        const float     pHalfSize)
{
  const float lMinX = pCenter.x() - pHalfSize;
  const float lMinY = pCenter.y() - pHalfSize;
  const float lMinZ = pCenter.z() - pHalfSize;
  const float lMaxX = pCenter.x() + pHalfSize;
  const float lMaxY = pCenter.y() + pHalfSize;
  const float lMaxZ = pCenter.z() + pHalfSize;

  Quad lQuad;

  lQuad.aP1 = Vector3D(lMinX, lMinY, lMinZ);
  lQuad.aP2 = Vector3D(lMaxX, lMinY, lMinZ);
  lQuad.aP3 = Vector3D(lMaxX, lMaxY, lMinZ);
  lQuad.aP4 = Vector3D(lMinX, lMaxY, lMinZ);
  aQuads.push_back(lQuad);

  lQuad.aP1 = Vector3D(lMinX, lMinY, lMaxZ);
  lQuad.aP2 = Vector3D(lMaxX, lMinY, lMaxZ);
  lQuad.aP3 = Vector3D(lMaxX, lMaxY, lMaxZ);
  lQuad.aP4 = Vector3D(lMinX, lMaxY, lMaxZ);
  aQuads.push_back(lQuad);

  lQuad.aP1 = Vector3D(lMinX, lMinY, lMinZ);
  lQuad.aP2 = Vector3D(lMaxX, lMinY, lMinZ);
  lQuad.aP3 = Vector3D(lMaxX, lMinY, lMaxZ);
  lQuad.aP4 = Vector3D(lMinX, lMinY, lMaxZ);
  aQuads.push_back(lQuad       );

  lQuad.aP1 = Vector3D(lMinX, lMaxY, lMinZ);
  lQuad.aP2 = Vector3D(lMaxX, lMaxY, lMinZ);
  lQuad.aP3 = Vector3D(lMaxX, lMaxY, lMaxZ);
  lQuad.aP4 = Vector3D(lMinX, lMaxY, lMaxZ);
  aQuads.push_back(lQuad);

  lQuad.aP1 = Vector3D(lMinX, lMinY, lMinZ);
  lQuad.aP2 = Vector3D(lMinX, lMaxY, lMinZ);
  lQuad.aP3 = Vector3D(lMinX, lMaxY, lMaxZ);
  lQuad.aP4 = Vector3D(lMinX, lMinY, lMaxZ);
  aQuads.push_back(lQuad);

  lQuad.aP1 = Vector3D(lMaxX, lMinY, lMinZ);
  lQuad.aP2 = Vector3D(lMaxX, lMaxY, lMinZ);
  lQuad.aP3 = Vector3D(lMaxX, lMaxY, lMaxZ);
  lQuad.aP4 = Vector3D(lMaxX, lMinY, lMaxZ);
  aQuads.push_back(lQuad);

  aSimplifiedDirty = true;
}

// We now do the equivalent of the glut commands in
// the PrimitiveAccumulator to take adventage of all
// it has to offer (BoundingBox updated, optimized diplay, etc)
void PrimitiveAccumulator::addSolidSphere(const Vector3D& pCenter,
                                          const float     pRadius,
                                          const int       pSlices,
                                          const int       pStacks)
{

  const double lDeltaTheta = 2.0f*M_PI/static_cast<double>(pSlices);
  const double lDeltaPhi   = M_PI/static_cast<double>(pStacks);

  for (int i=0; i<pSlices; ++i) {
    const double lTheta        = static_cast<double>(i)*lDeltaTheta;
    const double lNextTheta    = lTheta + lDeltaTheta;
    const double lCosTheta     = cos(lTheta);
    const double lCosNextTheta = cos(lNextTheta);
    const double lSinTheta     = sin(lTheta);
    const double lSinNextTheta = sin(lNextTheta);

    for (int j=0; j<pStacks; ++j) {
      const double lPhi        = static_cast<double>(j)*lDeltaPhi;
      const double lNextPhi    = lPhi + lDeltaPhi;
      const double lCosPhi     = cos(lPhi);
      const double lCosNextPhi = cos(lNextPhi);
      const double lSinPhi     = sin(lPhi);
      const double lSinNextPhi = sin(lNextPhi);

      QuadNormals lQuad;
      lQuad.aN1 = Vector3D(lCosNextTheta*lSinPhi    , lSinNextTheta*lSinPhi    , lCosPhi);
      lQuad.aN2 = Vector3D(lCosTheta*lSinPhi        , lSinTheta*lSinPhi        , lCosPhi);
      lQuad.aN3 = Vector3D(lCosTheta*lSinNextPhi    , lSinTheta*lSinNextPhi    , lCosNextPhi);
      lQuad.aN4 = Vector3D(lCosNextTheta*lSinNextPhi, lSinNextTheta*lSinNextPhi, lCosNextPhi);

      lQuad.aP1 = pRadius*lQuad.aN1 + pCenter;
      lQuad.aP2 = pRadius*lQuad.aN2 + pCenter;
      lQuad.aP3 = pRadius*lQuad.aN3 + pCenter;
      lQuad.aP4 = pRadius*lQuad.aN4 + pCenter;

      aQuadsNormals.push_back(lQuad);
    }
  }

  aSimplifiedDirty = true;
}

// We now do the equivalent of the glut commands in
// the PrimitiveAccumulator to take adventage of all
// it has to offer (BoundingBox updated, optimized diplay, etc)
void PrimitiveAccumulator::addWireCube(const Vector3D& pCenter,
                                       const float     pHalfSize)
{
  const float lMinX = pCenter.x() - pHalfSize;
  const float lMinY = pCenter.y() - pHalfSize;
  const float lMinZ = pCenter.z() - pHalfSize;
  const float lMaxX = pCenter.x() + pHalfSize;
  const float lMaxY = pCenter.y() + pHalfSize;
  const float lMaxZ = pCenter.z() + pHalfSize;

  Line lLine;
  lLine.aP1 = Vector3D(lMinX,lMinY,lMinZ); lLine.aP2 = Vector3D(lMaxX,lMinY,lMinZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMinX,lMinY,lMinZ); lLine.aP2 = Vector3D(lMinX,lMaxY,lMinZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMinX,lMinY,lMinZ); lLine.aP2 = Vector3D(lMinX,lMinY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMaxX,lMinY,lMinZ); lLine.aP2 = Vector3D(lMaxX,lMaxY,lMinZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMaxX,lMinY,lMinZ); lLine.aP2 = Vector3D(lMaxX,lMinY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMaxX,lMaxY,lMinZ); lLine.aP2 = Vector3D(lMinX,lMaxY,lMinZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMaxX,lMaxY,lMinZ); lLine.aP2 = Vector3D(lMaxX,lMaxY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMinX,lMaxY,lMinZ); lLine.aP2 = Vector3D(lMinX,lMaxY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMinX,lMinY,lMaxZ); lLine.aP2 = Vector3D(lMaxX,lMinY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMinX,lMinY,lMaxZ); lLine.aP2 = Vector3D(lMinX,lMaxY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMaxX,lMinY,lMaxZ); lLine.aP2 = Vector3D(lMaxX,lMaxY,lMaxZ);
  aLines.push_back(lLine);
  lLine.aP1 = Vector3D(lMinX,lMaxY,lMaxZ); lLine.aP2 = Vector3D(lMaxX,lMaxY,lMaxZ);
  aLines.push_back(lLine);

  aSimplifiedDirty = true;
}


void PrimitiveAccumulator::addLine(const Vector3D& pP1,
                                   const Vector3D& pP2)
{
  Line lLine;
  lLine.aP1 = pP1;
  lLine.aP2 = pP2;

  aLines.push_back(lLine);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addLineColored(const Vector3D& pP1,
                                          const Vector3D& pC1,
                                          const Vector3D& pP2,
                                          const Vector3D& pC2)
{
  LineColored lLineColored;
  lLineColored.aP1 = pP1;
  lLineColored.aP2 = pP2;
  lLineColored.aC1 = pC1;
  lLineColored.aC2 = pC2;

  aLinesColored.push_back(lLineColored);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addPoint(const Vector3D& pP)
{
  Point lPoint;
  lPoint.aP = pP;

  aPoints.push_back(lPoint);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addPointColored(const Vector3D& pP,
                                           const Vector3D& pC)
{
  PointColored lPointColored;
  lPointColored.aP = pP;
  lPointColored.aC = pC;

  aPointsColored.push_back(lPointColored);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addQuad(const Vector3D& pP1,
                                   const Vector3D& pP2,
                                   const Vector3D& pP3,
                                   const Vector3D& pP4)
{
  Quad lQuad;
  lQuad.aP1 = pP1;
  lQuad.aP2 = pP2;
  lQuad.aP3 = pP3;
  lQuad.aP4 = pP4;

  aQuads.push_back(lQuad);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addQuadColored(const Vector3D& pP1,
                                          const Vector3D& pC1,
                                          const Vector3D& pP2,
                                          const Vector3D& pC2,
                                          const Vector3D& pP3,
                                          const Vector3D& pC3,
                                          const Vector3D& pP4,
                                          const Vector3D& pC4)
{
  QuadColored lQuadColored;
  lQuadColored.aP1 = pP1;
  lQuadColored.aP2 = pP2;
  lQuadColored.aP3 = pP3;
  lQuadColored.aP4 = pP4;
  lQuadColored.aC1 = pC1;
  lQuadColored.aC2 = pC2;
  lQuadColored.aC3 = pC3;
  lQuadColored.aC4 = pC4;

  aQuadsColored.push_back(lQuadColored);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addTriangle(const Vector3D& pP1,
                                       const Vector3D& pP2,
                                       const Vector3D& pP3)
{
  Triangle lTriangle;
  lTriangle.aP1 = pP1;
  lTriangle.aP2 = pP2;
  lTriangle.aP3 = pP3;

  aTriangles.push_back(lTriangle);

  aSimplifiedDirty = true;
}

void PrimitiveAccumulator::addTriangleColored(const Vector3D& pP1,
                                              const Vector3D& pC1,
                                              const Vector3D& pP2,
                                              const Vector3D& pC2,
                                              const Vector3D& pP3,
                                              const Vector3D& pC3)
{
  TriangleColored lTriangleColored;
  lTriangleColored.aP1 = pP1;
  lTriangleColored.aP2 = pP2;
  lTriangleColored.aP3 = pP3;
  lTriangleColored.aC1 = pC1;
  lTriangleColored.aC2 = pC2;
  lTriangleColored.aC3 = pC3;

  aTrianglesColored.push_back(lTriangleColored);

  aSimplifiedDirty = true;
}

#ifdef GLV_DUMP_MEMORY_USAGE

template <class Container>
inline
std::string getStringSizeAndCapacity(const Container& pContainer)
{
  char lSizeAndCapacity[128];
  sprintf(lSizeAndCapacity,"%d/%d",
          sizeof(typename Container::value_type)*pContainer.size(),
          sizeof(typename Container::value_type)*pContainer.capacity());
  return std::string(lSizeAndCapacity);
}

#endif // #ifdef  GLV_DUMP_MEMORY_USAGE


// Dump in ASCII the caracteristics of the PrimitiveAccumulator
void PrimitiveAccumulator::dumpCharacteristics(std::ostream&       pOstream,
                                               const std::string&  pIndentation,
                                               const Matrix4x4&    pTransformation)
{
  pOstream << pIndentation << "PrimitiveAccumulator "  << std::endl;
  std::string lIndentation = pIndentation + "  ";

  // Update the bounding box. But we use the attribute
  // directly just after the call because we need it non-const
  getBoundingBox();
  aBoundingBox.dumpCharacteristics(pOstream, lIndentation, pTransformation);

#ifdef GLV_DUMP_SIMPLICATION
  if (aSimplified != this) {
    pOstream << lIndentation << "Simplified "  << std::endl;
    aSimplified->dumpCharacteristics(pOstream, lIndentation + "  ", pTransformation);
  }
#endif // #ifdef GLV_DUMP_SIMPLICATION

#ifdef GLV_DUMP_MEMORY_USAGE
  {
    pOstream << lIndentation << "Memory used by the PrimitiveAccumulator = " << sizeof(*this) << std::endl;

    pOstream << lIndentation << "Memory used by aPoints                  = " << getStringSizeAndCapacity(aPoints                 ) << std::endl;
    pOstream << lIndentation << "Memory used by aPointsColored           = " << getStringSizeAndCapacity(aPointsColored          ) << std::endl;
    pOstream << lIndentation << "Memory used by aLines                   = " << getStringSizeAndCapacity(aLines                  ) << std::endl;
    pOstream << lIndentation << "Memory used by aLinesColored            = " << getStringSizeAndCapacity(aLinesColored           ) << std::endl;
    pOstream << lIndentation << "Memory used by aTriangles               = " << getStringSizeAndCapacity(aTriangles              ) << std::endl;
    pOstream << lIndentation << "Memory used by aTrianglesColored        = " << getStringSizeAndCapacity(aTrianglesColored       ) << std::endl;
    pOstream << lIndentation << "Memory used by aTrianglesNormals        = " << getStringSizeAndCapacity(aTrianglesNormals       ) << std::endl;
    pOstream << lIndentation << "Memory used by aTrianglesNormalsColored = " << getStringSizeAndCapacity(aTrianglesNormalsColored) << std::endl;
    pOstream << lIndentation << "Memory used by aQuads                   = " << getStringSizeAndCapacity(aQuads                  ) << std::endl;
    pOstream << lIndentation << "Memory used by aQuadsColored            = " << getStringSizeAndCapacity(aQuadsColored           ) << std::endl;
    pOstream << lIndentation << "Memory used by aQuadsNormals            = " << getStringSizeAndCapacity(aQuadsNormals           ) << std::endl;
    pOstream << lIndentation << "Memory used by aQuadsNormalsColored     = " << getStringSizeAndCapacity(aQuadsNormalsColored    ) << std::endl;

  }
#endif // #ifdef GLV_DUMP_MEMORY_USAGE

  SizeType lNbQuads            = aQuads           .size() + aQuadsNormals           .size();
  SizeType lNbQuadsColored     = aQuadsColored    .size() + aQuadsNormalsColored    .size();
  SizeType lNbTriangles        = aTriangles       .size() + aTrianglesNormals       .size();
  SizeType lNbTrianglesColored = aTrianglesColored.size() + aTrianglesNormalsColored.size();

  pOstream << lIndentation << "Number of point            = " << aPoints       .size() << std::endl;
  pOstream << lIndentation << "Number of point_colored    = " << aPointsColored.size() << std::endl;
  pOstream << lIndentation << "Number of line             = " << aLines        .size() << std::endl;
  pOstream << lIndentation << "Number of line_colored     = " << aLinesColored .size() << std::endl;
  pOstream << lIndentation << "Number of triangle         = " << lNbTriangles          << std::endl;
  pOstream << lIndentation << "Number of triangle_colored = " << lNbTrianglesColored   << std::endl;
  pOstream << lIndentation << "Number of quad             = " << lNbQuads              << std::endl;
  pOstream << lIndentation << "Number of quad_colored     = " << lNbQuadsColored       << std::endl;
}

const BoundingBox& PrimitiveAccumulator::getBoundingBox() const
{

  // Scan only the new primitives for each type
  {
    const SizeType lSize = aLines.size();

    while (aLinesBBoxCounter < lSize) {
      const Line& lLine = aLines[aLinesBBoxCounter];
      aBoundingBox += lLine.aP1;
      aBoundingBox += lLine.aP2;
      ++aLinesBBoxCounter;
    }
  }

  {
    const SizeType lSize = aLinesColored.size();

    while (aLinesColoredBBoxCounter < lSize) {
      const Line& lLine = aLinesColored[aLinesColoredBBoxCounter];
      aBoundingBox += lLine.aP1;
      aBoundingBox += lLine.aP2;
      ++aLinesColoredBBoxCounter;
    }
  }

  {
    const SizeType lSize = aPoints.size();

    while (aPointsBBoxCounter < lSize) {
      const Point& lPoint = aPoints[aPointsBBoxCounter];
      aBoundingBox += lPoint.aP;
      ++aPointsBBoxCounter;
    }
  }

  {
    const SizeType lSize = aPointsColored.size();

    while (aPointsColoredBBoxCounter < lSize) {
      const Point& lPoint = aPointsColored[aPointsColoredBBoxCounter];
      aBoundingBox += lPoint.aP;
      ++aPointsColoredBBoxCounter;
    }
  }

  {
    const SizeType lSize = aQuads.size();

    while (aQuadsBBoxCounter < lSize) {
      const Quad& lQuad = aQuads[aQuadsBBoxCounter];
      aBoundingBox += lQuad.aP1;
      aBoundingBox += lQuad.aP2;
      aBoundingBox += lQuad.aP3;
      aBoundingBox += lQuad.aP4;
      ++aQuadsBBoxCounter;
    }
  }

  {
    const SizeType lSize = aQuadsColored.size();

    while (aQuadsColoredBBoxCounter < lSize) {
      const Quad& lQuad = aQuadsColored[aQuadsColoredBBoxCounter];
      aBoundingBox += lQuad.aP1;
      aBoundingBox += lQuad.aP2;
      aBoundingBox += lQuad.aP3;
      aBoundingBox += lQuad.aP4;
      ++aQuadsColoredBBoxCounter;
    }
  }

  {
    const SizeType lSize = aQuadsNormals.size();

    while (aQuadsNormalsBBoxCounter < lSize) {
      const Quad& lQuad = aQuadsNormals[aQuadsNormalsBBoxCounter];
      aBoundingBox += lQuad.aP1;
      aBoundingBox += lQuad.aP2;
      aBoundingBox += lQuad.aP3;
      aBoundingBox += lQuad.aP4;
      ++aQuadsNormalsBBoxCounter;
    }
  }

  {
    const SizeType lSize = aQuadsNormalsColored.size();

    while (aQuadsNormalsColoredBBoxCounter < lSize) {
      const Quad& lQuad = aQuadsNormalsColored[aQuadsNormalsColoredBBoxCounter];
      aBoundingBox += lQuad.aP1;
      aBoundingBox += lQuad.aP2;
      aBoundingBox += lQuad.aP3;
      aBoundingBox += lQuad.aP4;
      ++aQuadsNormalsColoredBBoxCounter;
    }
  }

  {
    const SizeType lSize = aTriangles.size();

    while (aTrianglesBBoxCounter < lSize) {
      const Triangle& lTriangle = aTriangles[aTrianglesBBoxCounter];
      aBoundingBox += lTriangle.aP1;
      aBoundingBox += lTriangle.aP2;
      aBoundingBox += lTriangle.aP3;
      ++aTrianglesBBoxCounter;
    }
  }

  {
    const SizeType lSize = aTrianglesColored.size();

    while (aTrianglesColoredBBoxCounter < lSize) {
      const Triangle& lTriangle = aTrianglesColored[aTrianglesColoredBBoxCounter];
      aBoundingBox += lTriangle.aP1;
      aBoundingBox += lTriangle.aP2;
      aBoundingBox += lTriangle.aP3;
      ++aTrianglesColoredBBoxCounter;
    }
  }

  {
    const SizeType lSize = aTrianglesNormals.size();

    while (aTrianglesNormalsBBoxCounter < lSize) {
      const Triangle& lTriangle = aTrianglesNormals[aTrianglesNormalsBBoxCounter];
      aBoundingBox += lTriangle.aP1;
      aBoundingBox += lTriangle.aP2;
      aBoundingBox += lTriangle.aP3;
      ++aTrianglesNormalsBBoxCounter;
    }
  }

  {
    const SizeType lSize = aTrianglesNormalsColored.size();

    while (aTrianglesNormalsColoredBBoxCounter < lSize) {
      const Triangle& lTriangle = aTrianglesNormalsColored[aTrianglesNormalsColoredBBoxCounter];
      aBoundingBox += lTriangle.aP1;
      aBoundingBox += lTriangle.aP2;
      aBoundingBox += lTriangle.aP3;
      ++aTrianglesNormalsColoredBBoxCounter;
    }
  }

  return aBoundingBox;
}

void PrimitiveAccumulator::render(const RenderParameters& pParams)
{
  switch (pParams.aRenderMode)
  {
  case RenderParameters::renderMode_full:
    renderFull(pParams);
    break;
  case RenderParameters::renderMode_bounding_box:
    aBoundingBox.render();
    break;
  case RenderParameters::renderMode_fast:
    renderSimplified(pParams);
    break;
  default:
    GLV_ASSERT(false);
  }
}



// Renders the content of aLines
void PrimitiveAccumulator::renderFacetsFrame(const RenderParameters& pParams)
{
  const SizeType lNbFacets = aQuads.size()               +
                             aQuadsColored.size()        +
                             aQuadsNormals.size()        +
                             aQuadsNormalsColored.size() +
                             aTriangles.size()           +
                             aTrianglesColored.size()    +
                             aTrianglesNormals.size()    +
                             aTrianglesNormalsColored.size();

  // If facet drawing is enabled and we've got something to draw
  if(pParams.aFlagRenderFacetFrame && lNbFacets > 0) {

    // We push the attributes on the openGL attribute stack; so
    // we dont disturb the current values of color (current) or line width (line)
    glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
    // Warning: performance hit; when a lot of push(GL_LIGHTING_BIT) calls are made
    glDisable(GL_LIGHTING);
    glLineWidth(1);
    glColor3f(pParams.aFacetBoundaryR, pParams.aFacetBoundaryG, pParams.aFacetBoundaryB);

    {
      Quads::const_iterator       lIterQuads    = aQuads.begin();
      const Quads::const_iterator lIterQuadsEnd = aQuads.end  ();

      while (lIterQuads != lIterQuadsEnd) {
        lIterQuads->renderFacetsFrame();
        ++lIterQuads;
      }
    }

    {
      QuadsColored::const_iterator       lIterQuads    = aQuadsColored.begin();
      const QuadsColored::const_iterator lIterQuadsEnd = aQuadsColored.end  ();

      while (lIterQuads != lIterQuadsEnd) {
        lIterQuads->renderFacetsFrame();
        ++lIterQuads;
      }
    }

    {
      QuadsNormals::const_iterator       lIterQuads    = aQuadsNormals.begin();
      const QuadsNormals::const_iterator lIterQuadsEnd = aQuadsNormals.end  ();

      while (lIterQuads != lIterQuadsEnd) {
        lIterQuads->renderFacetsFrame();
        ++lIterQuads;
      }
    }

    {
      QuadsNormalsColored::const_iterator       lIterQuads    = aQuadsNormalsColored.begin();
      const QuadsNormalsColored::const_iterator lIterQuadsEnd = aQuadsNormalsColored.end  ();

      while (lIterQuads != lIterQuadsEnd) {
        lIterQuads->renderFacetsFrame();
        ++lIterQuads;
      }
    }

    {
      Triangles::const_iterator       lIterTriangles    = aTriangles.begin();
      const Triangles::const_iterator lIterTrianglesEnd = aTriangles.end  ();

      while (lIterTriangles != lIterTrianglesEnd) {
        lIterTriangles->renderFacetsFrame();
        ++lIterTriangles;
      }
    }

    {
      TrianglesColored::const_iterator       lIterTriangles    = aTrianglesColored.begin();
      const TrianglesColored::const_iterator lIterTrianglesEnd = aTrianglesColored.end  ();

      while (lIterTriangles != lIterTrianglesEnd) {
        lIterTriangles->renderFacetsFrame();
        ++lIterTriangles;
      }
    }

    {
      TrianglesNormals::const_iterator       lIterTriangles    = aTrianglesNormals.begin();
      const TrianglesNormals::const_iterator lIterTrianglesEnd = aTrianglesNormals.end  ();

      while (lIterTriangles != lIterTrianglesEnd) {
        lIterTriangles->renderFacetsFrame();
        ++lIterTriangles;
      }
    }

    {
      TrianglesNormalsColored::const_iterator       lIterTriangles    = aTrianglesNormalsColored.begin();
      const TrianglesNormalsColored::const_iterator lIterTrianglesEnd = aTrianglesNormalsColored.end  ();

      while (lIterTriangles != lIterTrianglesEnd) {
        lIterTriangles->renderFacetsFrame();
        ++lIterTriangles;
      }
    }

    // Revert the lighting state and the line state
    glPopAttrib();
  }
}

void PrimitiveAccumulator::renderFull(const RenderParameters& pParams)
{
  aPrimitiveOptimizerValue = pParams.aPrimitiveOptimizerValue;

  renderFacetsFrame            (pParams);
  renderLines                  ();
  renderLinesColored           ();
  renderPoints                 ();
  renderPointsColored          ();
  renderQuads                  ();
  renderQuadsColored           ();
  renderQuadsNormals           ();
  renderQuadsNormalsColored    ();
  renderTriangles              ();
  renderTrianglesColored       ();
  renderTrianglesNormals       ();
  renderTrianglesNormalsColored();
}

// Renders the content of aLines
void PrimitiveAccumulator::renderLines()
{
  int lPrimitiveCount = 0;
  if (aLines.size() > 0) {

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    Lines::const_iterator       lIterLines    = aLines.begin();
    const Lines::const_iterator lIterLinesEnd = aLines.end  ();

    while (lIterLines != lIterLinesEnd) {

      const Line&     lLine = *lIterLines;
      const Vector3D& lP1   = lLine.aP1;
      const Vector3D& lP2   = lLine.aP2;

      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_LINES);
      }
      ++lIterLines;
    }

    glEnd();

    // Revert the lighting state
    glPopAttrib();

  }
}

// Renders the content of aLinesColored
void PrimitiveAccumulator::renderLinesColored()
{
  int lPrimitiveCount = 0;
  if (aLinesColored.size() > 0) {

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    LinesColored::const_iterator       lIterLinesColored    = aLinesColored.begin();
    const LinesColored::const_iterator lIterLinesColoredEnd = aLinesColored.end  ();

    while (lIterLinesColored != lIterLinesColoredEnd) {

      const LineColored&  lLineColored = *lIterLinesColored;
      const Vector3D&     lP1          = lLineColored.aP1;
      const Vector3D&     lP2          = lLineColored.aP2;
      const Vector3D&     lC1          = lLineColored.aC1;
      const Vector3D&     lC2          = lLineColored.aC2;

      glColor3f (lC1.x(), lC1.y(), lC1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glColor3f (lC2.x(), lC2.y(), lC2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_LINES);
      }
      ++lIterLinesColored;
    }

    glEnd();

    // Revert the lighting state
    glPopAttrib();

  }
}

// Renders the content of aPoints
void PrimitiveAccumulator::renderPoints()
{
  if (aPoints.size() > 0) {

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);

    Points::const_iterator       lIterPoints    = aPoints.begin();
    const Points::const_iterator lIterPointsEnd = aPoints.end  ();

    while (lIterPoints != lIterPointsEnd) {

      const Point&    lPoint = *lIterPoints;
      const Vector3D& lP     = lPoint.aP;

      glVertex3f(lP.x(), lP.y(), lP.z());

      ++lIterPoints;
    }

    glEnd();

    // Revert the lighting state
    glPopAttrib();

  }
}

// Renders the content of aPointsColored
void PrimitiveAccumulator::renderPointsColored()
{
  if (aPointsColored.size() > 0) {

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);

    PointsColored::const_iterator       lIterPointsColored    = aPointsColored.begin();
    const PointsColored::const_iterator lIterPointsColoredEnd = aPointsColored.end  ();

    while (lIterPointsColored != lIterPointsColoredEnd) {

      const PointColored&  lPointColored = *lIterPointsColored;
      const Vector3D&      lP            = lPointColored.aP;
      const Vector3D&      lC            = lPointColored.aC;

      glColor3f (lC.x(), lC.y(), lC.z());
      glVertex3f(lP.x(), lP.y(), lP.z());

      ++lIterPointsColored;
    }

    glEnd();

    // Revert the lighting state
    glPopAttrib();

  }
}

// Renders the content of aQuads
void PrimitiveAccumulator::renderQuads()
{
  int lPrimitiveCount = 0;

  if (aQuads.size() > 0) {

    glBegin(GL_QUADS);

    Quads::const_iterator       lIterQuads    = aQuads.begin();
    const Quads::const_iterator lIterQuadsEnd = aQuads.end  ();

    while (lIterQuads != lIterQuadsEnd) {

      const Quad&     lQuad = *lIterQuads;
      const Vector3D& lP1   = lQuad.aP1;
      const Vector3D& lP2   = lQuad.aP2;
      const Vector3D& lP3   = lQuad.aP3;
      const Vector3D& lP4   = lQuad.aP4;

      // Compute normals
      Vector3D lN1 = (lP2-lP1).crossProduct(lP4-lP1);
      Vector3D lN2 = (lP3-lP2).crossProduct(lP1-lP2);
      Vector3D lN3 = (lP4-lP3).crossProduct(lP2-lP3);
      Vector3D lN4 = (lP1-lP4).crossProduct(lP3-lP4);

      lN1.normalize();
      lN2.normalize();
      lN3.normalize();
      lN4.normalize();

      glNormal3f(lN1.x(), lN1.y(), lN1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f(lN2.x(), lN2.y(), lN2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f(lN3.x(), lN3.y(), lN3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());
      glNormal3f(lN4.x(), lN4.y(), lN4.z());
      glVertex3f(lP4.x(), lP4.y(), lP4.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_QUADS);
      }
      ++lIterQuads;
    }

    glEnd();

  }
}

// Renders the content of aQuadsColored
void PrimitiveAccumulator::renderQuadsColored()
{
  int lPrimitiveCount = 0;
  if (aQuadsColored.size() > 0) {

    glBegin(GL_QUADS);

    QuadsColored::const_iterator       lIterQuadsColored    = aQuadsColored.begin();
    const QuadsColored::const_iterator lIterQuadsColoredEnd = aQuadsColored.end  ();

    while (lIterQuadsColored != lIterQuadsColoredEnd) {

      const QuadColored&  lQuadColored = *lIterQuadsColored;
      const Vector3D&     lP1          = lQuadColored.aP1;
      const Vector3D&     lP2          = lQuadColored.aP2;
      const Vector3D&     lP3          = lQuadColored.aP3;
      const Vector3D&     lP4          = lQuadColored.aP4;
      const Vector3D&     lC1          = lQuadColored.aC1;
      const Vector3D&     lC2          = lQuadColored.aC2;
      const Vector3D&     lC3          = lQuadColored.aC3;
      const Vector3D&     lC4          = lQuadColored.aC4;

      // Compute normals
      Vector3D lN1 = (lP2-lP1).crossProduct(lP4-lP1);
      Vector3D lN2 = (lP3-lP2).crossProduct(lP1-lP2);
      Vector3D lN3 = (lP4-lP3).crossProduct(lP2-lP3);
      Vector3D lN4 = (lP1-lP4).crossProduct(lP3-lP4);

      lN1.normalize();
      lN2.normalize();
      lN3.normalize();
      lN4.normalize();

      glNormal3f(lN1.x(), lN1.y(), lN1.z());
      glColor3f (lC1.x(), lC1.y(), lC1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f(lN2.x(), lN2.y(), lN2.z());
      glColor3f (lC2.x(), lC2.y(), lC2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f(lN3.x(), lN3.y(), lN3.z());
      glColor3f (lC3.x(), lC3.y(), lC3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());
      glNormal3f(lN4.x(), lN4.y(), lN4.z());
      glColor3f (lC4.x(), lC4.y(), lC4.z());
      glVertex3f(lP4.x(), lP4.y(), lP4.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_QUADS);
      }
      ++lIterQuadsColored;
    }

    glEnd();

  }
}

// Renders the content of aQuadsNormals
void PrimitiveAccumulator::renderQuadsNormals()
{
  int lPrimitiveCount = 0;
  if (aQuadsNormals.size() > 0) {

    glBegin(GL_QUADS);

    QuadsNormals::const_iterator       lIterQuadsNormals    = aQuadsNormals.begin();
    const QuadsNormals::const_iterator lIterQuadsNormalsEnd = aQuadsNormals.end  ();

    while (lIterQuadsNormals != lIterQuadsNormalsEnd) {

      const QuadNormals& lQuad = *lIterQuadsNormals;

      const Vector3D& lP1 = lQuad.aP1;
      const Vector3D& lP2 = lQuad.aP2;
      const Vector3D& lP3 = lQuad.aP3;
      const Vector3D& lP4 = lQuad.aP4;
      const Vector3D& lN1 = lQuad.aN1;
      const Vector3D& lN2 = lQuad.aN2;
      const Vector3D& lN3 = lQuad.aN3;
      const Vector3D& lN4 = lQuad.aN4;

      glNormal3f(lN1.x(), lN1.y(), lN1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f(lN2.x(), lN2.y(), lN2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f(lN3.x(), lN3.y(), lN3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());
      glNormal3f(lN4.x(), lN4.y(), lN4.z());
      glVertex3f(lP4.x(), lP4.y(), lP4.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_QUADS);
      }
      ++lIterQuadsNormals;
    }

    glEnd();

  }
}

// Renders the content of aQuadsNormalsColored
void PrimitiveAccumulator::renderQuadsNormalsColored()
{
  int lPrimitiveCount = 0;
  if (aQuadsNormalsColored.size() > 0) {

    glBegin(GL_QUADS);

    QuadsNormalsColored::const_iterator       lIterQuadsNormalsColored    = aQuadsNormalsColored.begin();
    const QuadsNormalsColored::const_iterator lIterQuadsNormalsColoredEnd = aQuadsNormalsColored.end  ();

    while (lIterQuadsNormalsColored != lIterQuadsNormalsColoredEnd) {

      const QuadNormalsColored& lQuadColored = *lIterQuadsNormalsColored;

      const Vector3D& lP1 = lQuadColored.aP1;
      const Vector3D& lP2 = lQuadColored.aP2;
      const Vector3D& lP3 = lQuadColored.aP3;
      const Vector3D& lP4 = lQuadColored.aP4;
      const Vector3D& lC1 = lQuadColored.aC1;
      const Vector3D& lC2 = lQuadColored.aC2;
      const Vector3D& lC3 = lQuadColored.aC3;
      const Vector3D& lC4 = lQuadColored.aC4;
      const Vector3D& lN1 = lQuadColored.aN1;
      const Vector3D& lN2 = lQuadColored.aN2;
      const Vector3D& lN3 = lQuadColored.aN3;
      const Vector3D& lN4 = lQuadColored.aN4;

      glNormal3f(lN1.x(), lN1.y(), lN1.z());
      glColor3f (lC1.x(), lC1.y(), lC1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f(lN2.x(), lN2.y(), lN2.z());
      glColor3f (lC2.x(), lC2.y(), lC2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f(lN3.x(), lN3.y(), lN3.z());
      glColor3f (lC3.x(), lC3.y(), lC3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());
      glNormal3f(lN4.x(), lN4.y(), lN4.z());
      glColor3f (lC4.x(), lC4.y(), lC4.z());
      glVertex3f(lP4.x(), lP4.y(), lP4.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_QUADS);
      }
      ++lIterQuadsNormalsColored;
    }

    glEnd();

  }
}

void PrimitiveAccumulator::renderSimplified(const RenderParameters& pParams)
{
  // Check if we have to update the Simplified model
  if (aSimplifiedDirty) {
    constructSimplified();
  }

  RenderParameters lParams = pParams;
  lParams.aRenderMode      = RenderParameters::renderMode_full;

  // We push the attributes on the openGL attribute stack; so
  // we dont disturb the current values of color (current) or line width (line)
  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
  // Warning: performance hit; when a lot of push(GL_LIGHTING_BIT) calls are made
  glDisable(GL_LIGHTING);
  glPointSize(3);

  glColor3f(0.7, 0.5, 0.1);

  GLV_ASSERT(aSimplified != 0);
  aSimplified->render(lParams);

  // Revert the lighting state and the line state
  glPopAttrib();

  GLV_ASSERT(aSimplified != 0);
  aSimplified->renderFull(lParams);
}

// Renders the content of aTriangles
void PrimitiveAccumulator::renderTriangles()
{
  int lPrimitiveCount = 0;

  if (aTriangles.size() > 0) {

    glBegin(GL_TRIANGLES);

    Triangles::const_iterator       lIterTriangles    = aTriangles.begin();
    const Triangles::const_iterator lIterTrianglesEnd = aTriangles.end  ();

    while (lIterTriangles != lIterTrianglesEnd) {

      const Triangle& lTriangle = *lIterTriangles;
      const Vector3D& lP1       = lTriangle.aP1;
      const Vector3D& lP2       = lTriangle.aP2;
      const Vector3D& lP3       = lTriangle.aP3;

      // Compute normals
      Vector3D lN = (lP2-lP1).crossProduct(lP3-lP1);
      lN.normalize();

      glNormal3f( lN.x(),  lN.y(),  lN.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f( lN.x(),  lN.y(),  lN.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f( lN.x(),  lN.y(),  lN.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_TRIANGLES);
      }
      ++lIterTriangles;
    }

    glEnd();

  }
}

// Renders the content of aTrianglesColored
void PrimitiveAccumulator::renderTrianglesColored()
{
  int lPrimitiveCount = 0;

  if (aTrianglesColored.size() > 0) {

    glBegin(GL_TRIANGLES);

    TrianglesColored::const_iterator       lIterTrianglesColored    = aTrianglesColored.begin();
    const TrianglesColored::const_iterator lIterTrianglesColoredEnd = aTrianglesColored.end  ();

    while (lIterTrianglesColored != lIterTrianglesColoredEnd) {

      const TriangleColored& lTriangleColored = *lIterTrianglesColored;
      const Vector3D&        lP1          = lTriangleColored.aP1;
      const Vector3D&        lP2          = lTriangleColored.aP2;
      const Vector3D&        lP3          = lTriangleColored.aP3;
      const Vector3D&        lC1          = lTriangleColored.aC1;
      const Vector3D&        lC2          = lTriangleColored.aC2;
      const Vector3D&        lC3          = lTriangleColored.aC3;

      // Compute normals
      Vector3D lN = (lP2-lP1).crossProduct(lP3-lP1);
      lN.normalize();

      glNormal3f( lN.x(),  lN.y(),  lN.z());
      glColor3f (lC1.x(), lC1.y(), lC1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glColor3f (lC2.x(), lC2.y(), lC2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glColor3f (lC3.x(), lC3.y(), lC3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_TRIANGLES);
      }
      ++lIterTrianglesColored;
    }

    glEnd();

  }
}

// Renders the content of aTrianglesNormals
void PrimitiveAccumulator::renderTrianglesNormals()
{
  int lPrimitiveCount = 0;

  if (aTrianglesNormals.size() > 0) {

    glBegin(GL_TRIANGLES);

    TrianglesNormals::const_iterator       lIterTrianglesNormals    = aTrianglesNormals.begin();
    const TrianglesNormals::const_iterator lIterTrianglesNormalsEnd = aTrianglesNormals.end  ();

    while (lIterTrianglesNormals != lIterTrianglesNormalsEnd) {

      const TriangleNormals& lTriangle = *lIterTrianglesNormals;

      const Vector3D& lP1 = lTriangle.aP1;
      const Vector3D& lP2 = lTriangle.aP2;
      const Vector3D& lP3 = lTriangle.aP3;
      const Vector3D& lN1 = lTriangle.aN1;
      const Vector3D& lN2 = lTriangle.aN2;
      const Vector3D& lN3 = lTriangle.aN3;

      glNormal3f(lN1.x(), lN1.y(), lN1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f(lN2.x(), lN2.y(), lN2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f(lN3.x(), lN3.y(), lN3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_TRIANGLES);
      }
      ++lIterTrianglesNormals;
    }

    glEnd();

  }
}

// Renders the content of aTrianglesNormalsColored
void PrimitiveAccumulator::renderTrianglesNormalsColored()
{
  int lPrimitiveCount = 0;

  if (aTrianglesNormalsColored.size() > 0) {

    glBegin(GL_TRIANGLES);

    TrianglesNormalsColored::const_iterator       lIterTrianglesNormalsColored    = aTrianglesNormalsColored.begin();
    const TrianglesNormalsColored::const_iterator lIterTrianglesNormalsColoredEnd = aTrianglesNormalsColored.end  ();

    while (lIterTrianglesNormalsColored != lIterTrianglesNormalsColoredEnd) {

      const TriangleNormalsColored& lTriangleColored = *lIterTrianglesNormalsColored;

      const Vector3D& lP1 = lTriangleColored.aP1;
      const Vector3D& lP2 = lTriangleColored.aP2;
      const Vector3D& lP3 = lTriangleColored.aP3;
      const Vector3D& lC1 = lTriangleColored.aC1;
      const Vector3D& lC2 = lTriangleColored.aC2;
      const Vector3D& lC3 = lTriangleColored.aC3;
      const Vector3D& lN1 = lTriangleColored.aN1;
      const Vector3D& lN2 = lTriangleColored.aN2;
      const Vector3D& lN3 = lTriangleColored.aN3;

      glNormal3f(lN1.x(), lN1.y(), lN1.z());
      glColor3f (lC1.x(), lC1.y(), lC1.z());
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glNormal3f(lN2.x(), lN2.y(), lN2.z());
      glColor3f (lC2.x(), lC2.y(), lC2.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glNormal3f(lN3.x(), lN3.y(), lN3.z());
      glColor3f (lC3.x(), lC3.y(), lC3.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());

      lPrimitiveCount++;
      // This value sets the maximum length of the number of primitives
      // between a glBegin and a glEnd.  Some openGL driver optimize too
      // much and are slow to execute onto long primitive lists
      if(lPrimitiveCount % aPrimitiveOptimizerValue == 0) {
        glEnd();
        glBegin(GL_TRIANGLES);
      }
      ++lIterTrianglesNormalsColored;
    }

    glEnd();

  }
}

struct SimplificationPoint
{

  SimplificationPoint(const float pX,
                      const float pY,
                      const float pZ)
    : aBarycenterDistance (std::numeric_limits<float>::max()),
      aPoint              (pX, pY, pZ),
      aPrimiticeBarycenter()
    {}

  float     aBarycenterDistance;
  Vector3D  aPoint;
  Vector3D  aPrimiticeBarycenter;

};


template <class Container>
inline void scanForClosest(Container&           pContainer,
                           SimplificationPoint* pIterSPBegin,
                           SimplificationPoint* pIterSPEnd)
{
  const typename Container::size_type lSize = pContainer.size();

  for (typename Container::size_type i=0; i<lSize; ++i) {

    const typename Container::value_type&  lPrimitive = pContainer[i];
    SimplificationPoint*                   lIterSP    = pIterSPBegin;

    while (lIterSP != pIterSPEnd) {
      SimplificationPoint& lSP         = *lIterSP;
      const Vector3D       lBarycenter = lPrimitive .getBarycenter();
      const float          lDistance   = lBarycenter.getDistanceTo(lSP.aPoint);
      if (lDistance < lSP.aBarycenterDistance) {
        lSP.aPrimiticeBarycenter = lBarycenter;
        lSP.aBarycenterDistance  = lDistance;
      }
      ++lIterSP;
    }
  }
}

void PrimitiveAccumulator::addArrowTip(const Vector3D& pP1,
                                       const Vector3D& pP2,
                                       const Vector3D& pC2,
                                       const bool      pUseColor,
                                       const float     pTipProportion,
                                       const int       pTipNbPolygons)
{
  GLV_ASSERT(pTipProportion  > 0.0f);
  GLV_ASSERT(pTipProportion <= 1.0f);
  GLV_ASSERT(pTipNbPolygons >= 1);

  // Now construct the tip
  const float    lTanTheta  = 0.2f;
  const Vector3D lLine      = pP2-pP1;
  const float    lLength    = lLine.getLength();

  if (lLength > 0.0f) {
    const Vector3D lDirection = lLine/lLength;

    // find a perpendicular vector to lDirection
    // using Gram-Schmidt on one of i,j or k. Pick
    // the one with the dot product with lDirection
    // closest to zero.
    Vector3D    lPerpendicularUnitary;
    const float lAbsDirectionX = fabs(lDirection.x());
    const float lAbsDirectionY = fabs(lDirection.y());
    const float lAbsDirectionZ = fabs(lDirection.z());
    if (lAbsDirectionX < lAbsDirectionY && lAbsDirectionX < lAbsDirectionZ) {
      lPerpendicularUnitary = Vector3D(1.0f, 0.0f, 0.0f) - lDirection.x()*lDirection;
    }
    else if(lAbsDirectionY < lAbsDirectionZ) {
      GLV_ASSERT(lAbsDirectionY <= lAbsDirectionX);
      lPerpendicularUnitary = Vector3D(0.0f, 1.0f, 0.0f) - lDirection.y()*lDirection;
    }
    else {
      GLV_ASSERT(lAbsDirectionZ <= lAbsDirectionX);
      GLV_ASSERT(lAbsDirectionZ <= lAbsDirectionY);
      lPerpendicularUnitary = Vector3D(0.0f, 0.0f, 1.0f) - lDirection.z()*lDirection;
    }
    lPerpendicularUnitary.normalize();

    const float lTipRadius     = lTanTheta*pTipProportion*lLength;
    Vector3D    lPerpendicular = lTipRadius*lPerpendicularUnitary;

    // We rotate that lFirstTipPoint around lDirection
    // as many times as we want polygons
    Matrix4x4 lTransformation;
    lTransformation.rotateAbout(lDirection, 2.0*M_PI/static_cast<float>(pTipNbPolygons));

    const Vector3D lBaseTip    = (1.0f-pTipProportion)*lLine + pP1;
    const Vector3D lBaseNormal = -0.1f*lDirection;

    for (int i=0; i<=pTipNbPolygons; ++i) {

      const Vector3D lNewPerpendicularUnitary = lTransformation*lPerpendicularUnitary;
      const Vector3D lNewPerpendicular        = lTipRadius*lPerpendicularUnitary;

      TriangleNormalsColored lTriangle;

      lTriangle.aP1 = lBaseTip + lPerpendicular;
      lTriangle.aP2 = lBaseTip + lNewPerpendicular;
      lTriangle.aP3 = pP2;
      lTriangle.aC1 = pC2;
      lTriangle.aC2 = pC2;
      lTriangle.aC3 = pC2;
      lTriangle.aN1 = lPerpendicularUnitary;
      lTriangle.aN2 = lNewPerpendicularUnitary;
      lTriangle.aN3 = lDirection;

      if (pUseColor) {
        aTrianglesNormalsColored.push_back(lTriangle);
      }
      else {
        aTrianglesNormals.push_back(lTriangle);
      }

      lTriangle.aP1 = lBaseTip + lPerpendicular;
      lTriangle.aP2 = lBaseTip + lNewPerpendicular;
      lTriangle.aP3 = lBaseTip;
      // Use normals to attenuate the intensity
      // of the base of the tip
      lTriangle.aN1 = lBaseNormal;
      lTriangle.aN2 = lBaseNormal;
      lTriangle.aN3 = lBaseNormal;

      if (pUseColor) {
        aTrianglesNormalsColored.push_back(lTriangle);
      }
      else {
        aTrianglesNormals.push_back(lTriangle);
      }

      lPerpendicularUnitary = lNewPerpendicularUnitary;
      lPerpendicular        = lNewPerpendicular;
    }
  }
}

void PrimitiveAccumulator::constructSimplified()
{

  // The simplified model is a set of points located
  // at the barycenter of a subset of the primitives.
  // These primitives are chosen so that the
  // object shape can still be recognise by the user
  // with a modest number of points.
  // More precisely, they are the primitives "closest"
  // to a set of points on a the circumscribed sphere to
  // the BoundingBox.

  if (!aSimplifiedDirty) {
    // Just to be safe, should not pass here
    // if nobody calls it when aSimplified is
    // not used
  }
  else {

    // Start from scratch
    GLV_ASSERT(aSimplified != 0);
    if (aSimplified != this) {
      delete aSimplified;
    }
    aSimplified = new PrimitiveAccumulator(false);

    // We call getBoundingBox so that it gets updated
    // if it needs to
    const BoundingBox& lBoundingBox = getBoundingBox();
    const Vector3D     lCenter      = lBoundingBox.getCenter();
    const float        lRadius      = lBoundingBox.getCircumscribedSphereRadius();

    SimplificationPoint unitSphereSampling[] = {
      SimplificationPoint(-1.0,0.0,0.0),
      SimplificationPoint(-0.894427191,-0.4472135955,0.0),
      SimplificationPoint(-0.894427191,0.0,-0.4472135955),
      SimplificationPoint(-0.894427191,0.0,0.4472135955),
      SimplificationPoint(-0.894427191,0.4472135955,0.0),
      SimplificationPoint(-0.816496580928,-0.408248290464,-0.408248290464),
      SimplificationPoint(-0.816496580928,-0.408248290464,0.408248290464),
      SimplificationPoint(-0.816496580928,0.408248290464,-0.408248290464),
      SimplificationPoint(-0.816496580928,0.408248290464,0.408248290464),
      SimplificationPoint(-0.707106781187,-0.707106781187,0.0),
      SimplificationPoint(-0.707106781187,0.0,-0.707106781187),
      SimplificationPoint(-0.707106781187,0.0,0.707106781187),
      SimplificationPoint(-0.707106781187,0.707106781187,0.0),
      SimplificationPoint(-0.666666666667,-0.333333333333,-0.666666666667),
      SimplificationPoint(-0.666666666667,-0.333333333333,0.666666666667),
      SimplificationPoint(-0.666666666667,-0.666666666667,-0.333333333333),
      SimplificationPoint(-0.666666666667,-0.666666666667,0.333333333333),
      SimplificationPoint(-0.666666666667,0.333333333333,-0.666666666667),
      SimplificationPoint(-0.666666666667,0.333333333333,0.666666666667),
      SimplificationPoint(-0.666666666667,0.666666666667,-0.333333333333),
      SimplificationPoint(-0.666666666667,0.666666666667,0.333333333333),
      SimplificationPoint(-0.57735026919,-0.57735026919,-0.57735026919),
      SimplificationPoint(-0.57735026919,-0.57735026919,0.57735026919),
      SimplificationPoint(-0.57735026919,0.57735026919,-0.57735026919),
      SimplificationPoint(-0.57735026919,0.57735026919,0.57735026919),
      SimplificationPoint(-0.4472135955,-0.894427191,0.0),
      SimplificationPoint(-0.4472135955,0.0,-0.894427191),
      SimplificationPoint(-0.4472135955,0.0,0.894427191),
      SimplificationPoint(-0.4472135955,0.894427191,0.0),
      SimplificationPoint(-0.408248290464,-0.408248290464,-0.816496580928),
      SimplificationPoint(-0.408248290464,-0.408248290464,0.816496580928),
      SimplificationPoint(-0.408248290464,-0.816496580928,-0.408248290464),
      SimplificationPoint(-0.408248290464,-0.816496580928,0.408248290464),
      SimplificationPoint(-0.408248290464,0.408248290464,-0.816496580928),
      SimplificationPoint(-0.408248290464,0.408248290464,0.816496580928),
      SimplificationPoint(-0.408248290464,0.816496580928,-0.408248290464),
      SimplificationPoint(-0.408248290464,0.816496580928,0.408248290464),
      SimplificationPoint(-0.333333333333,-0.666666666667,-0.666666666667),
      SimplificationPoint(-0.333333333333,-0.666666666667,0.666666666667),
      SimplificationPoint(-0.333333333333,0.666666666667,-0.666666666667),
      SimplificationPoint(-0.333333333333,0.666666666667,0.666666666667),
      SimplificationPoint(0.0,-0.4472135955,-0.894427191),
      SimplificationPoint(0.0,-0.4472135955,0.894427191),
      SimplificationPoint(0.0,-0.707106781187,-0.707106781187),
      SimplificationPoint(0.0,-0.707106781187,0.707106781187),
      SimplificationPoint(0.0,-0.894427191,-0.4472135955),
      SimplificationPoint(0.0,-0.894427191,0.4472135955),
      SimplificationPoint(0.0,-1.0,0.0),
      SimplificationPoint(0.0,0.0,-1.0),
      SimplificationPoint(0.0,0.0,1.0),
      SimplificationPoint(0.0,0.4472135955,-0.894427191),
      SimplificationPoint(0.0,0.4472135955,0.894427191),
      SimplificationPoint(0.0,0.707106781187,-0.707106781187),
      SimplificationPoint(0.0,0.707106781187,0.707106781187),
      SimplificationPoint(0.0,0.894427191,-0.4472135955),
      SimplificationPoint(0.0,0.894427191,0.4472135955),
      SimplificationPoint(0.0,1.0,0.0),
      SimplificationPoint(0.333333333333,-0.666666666667,-0.666666666667),
      SimplificationPoint(0.333333333333,-0.666666666667,0.666666666667),
      SimplificationPoint(0.333333333333,0.666666666667,-0.666666666667),
      SimplificationPoint(0.333333333333,0.666666666667,0.666666666667),
      SimplificationPoint(0.408248290464,-0.408248290464,-0.816496580928),
      SimplificationPoint(0.408248290464,-0.408248290464,0.816496580928),
      SimplificationPoint(0.408248290464,-0.816496580928,-0.408248290464),
      SimplificationPoint(0.408248290464,-0.816496580928,0.408248290464),
      SimplificationPoint(0.408248290464,0.408248290464,-0.816496580928),
      SimplificationPoint(0.408248290464,0.408248290464,0.816496580928),
      SimplificationPoint(0.408248290464,0.816496580928,-0.408248290464),
      SimplificationPoint(0.408248290464,0.816496580928,0.408248290464),
      SimplificationPoint(0.4472135955,-0.894427191,0.0),
      SimplificationPoint(0.4472135955,0.0,-0.894427191),
      SimplificationPoint(0.4472135955,0.0,0.894427191),
      SimplificationPoint(0.4472135955,0.894427191,0.0),
      SimplificationPoint(0.57735026919,-0.57735026919,-0.57735026919),
      SimplificationPoint(0.57735026919,-0.57735026919,0.57735026919),
      SimplificationPoint(0.57735026919,0.57735026919,-0.57735026919),
      SimplificationPoint(0.57735026919,0.57735026919,0.57735026919),
      SimplificationPoint(0.666666666667,-0.333333333333,-0.666666666667),
      SimplificationPoint(0.666666666667,-0.333333333333,0.666666666667),
      SimplificationPoint(0.666666666667,-0.666666666667,-0.333333333333),
      SimplificationPoint(0.666666666667,-0.666666666667,0.333333333333),
      SimplificationPoint(0.666666666667,0.333333333333,-0.666666666667),
      SimplificationPoint(0.666666666667,0.333333333333,0.666666666667),
      SimplificationPoint(0.666666666667,0.666666666667,-0.333333333333),
      SimplificationPoint(0.666666666667,0.666666666667,0.333333333333),
      SimplificationPoint(0.707106781187,-0.707106781187,0.0),
      SimplificationPoint(0.707106781187,0.0,-0.707106781187),
      SimplificationPoint(0.707106781187,0.0,0.707106781187),
      SimplificationPoint(0.707106781187,0.707106781187,0.0),
      SimplificationPoint(0.816496580928,-0.408248290464,-0.408248290464),
      SimplificationPoint(0.816496580928,-0.408248290464,0.408248290464),
      SimplificationPoint(0.816496580928,0.408248290464,-0.408248290464),
      SimplificationPoint(0.816496580928,0.408248290464,0.408248290464),
      SimplificationPoint(0.894427191,-0.4472135955,0.0),
      SimplificationPoint(0.894427191,0.0,-0.4472135955),
      SimplificationPoint(0.894427191,0.0,0.4472135955),
      SimplificationPoint(0.894427191,0.4472135955,0.0),
      SimplificationPoint(1.0,0.0,0.0)
    };

//     SimplificationPoint unitSphereSampling[] = {
//       SimplificationPoint(-1.0,0.0,0.0),
//       SimplificationPoint(-0.707106781187,-0.707106781187,0.0),
//       SimplificationPoint(-0.707106781187,0.0,-0.707106781187),
//       SimplificationPoint(-0.707106781187,0.0,0.707106781187),
//       SimplificationPoint(-0.707106781187,0.707106781187,0.0),
//       SimplificationPoint(-0.57735026919,-0.57735026919,-0.57735026919),
//       SimplificationPoint(-0.57735026919,-0.57735026919,0.57735026919),
//       SimplificationPoint(-0.57735026919,0.57735026919,-0.57735026919),
//       SimplificationPoint(-0.57735026919,0.57735026919,0.57735026919),
//       SimplificationPoint(0.0,-0.707106781187,-0.707106781187),
//       SimplificationPoint(0.0,-0.707106781187,0.707106781187),
//       SimplificationPoint(0.0,-1.0,0.0),
//       SimplificationPoint(0.0,0.0,-1.0),
//       SimplificationPoint(0.0,0.0,1.0),
//       SimplificationPoint(0.0,0.707106781187,-0.707106781187),
//       SimplificationPoint(0.0,0.707106781187,0.707106781187),
//       SimplificationPoint(0.0,1.0,0.0),
//       SimplificationPoint(0.57735026919,-0.57735026919,-0.57735026919),
//       SimplificationPoint(0.57735026919,-0.57735026919,0.57735026919),
//       SimplificationPoint(0.57735026919,0.57735026919,-0.57735026919),
//       SimplificationPoint(0.57735026919,0.57735026919,0.57735026919),
//       SimplificationPoint(0.707106781187,-0.707106781187,0.0),
//       SimplificationPoint(0.707106781187,0.0,-0.707106781187),
//       SimplificationPoint(0.707106781187,0.0,0.707106781187),
//       SimplificationPoint(0.707106781187,0.707106781187,0.0),
//       SimplificationPoint(1.0,0.0,0.0)
//     };

    SimplificationPoint* lBegin = unitSphereSampling;
    SimplificationPoint* lIter  = lBegin;
    SimplificationPoint* lEnd   = unitSphereSampling + (sizeof(unitSphereSampling)/sizeof(SimplificationPoint));

    // Scale the point from the unit sphere to the circumscribed sphere
    GLV_ASSERT(lBegin != 0);
    GLV_ASSERT(lEnd   != 0);

    while (lIter != lEnd) {
      GLV_ASSERT(lIter != 0);
      lIter->aPoint *= lRadius;
      lIter->aPoint += lCenter;
      ++lIter;
    }

    // Scan all the primitives to find the closest ones
    // For now, we choose only one primitive in all the types
    // for each point on the shere. Again, we'll have to see
    // if if wouldn't be better to have a separate simplication
    // for each type of primitive.  We'll see.
    // Scan only the new primitives for each type
    scanForClosest(aLines,                   lBegin, lEnd);
    scanForClosest(aLinesColored,            lBegin, lEnd);
    scanForClosest(aPoints,                  lBegin, lEnd);
    scanForClosest(aPointsColored,           lBegin, lEnd);
    scanForClosest(aQuads,                   lBegin, lEnd);
    scanForClosest(aQuadsColored,            lBegin, lEnd);
    scanForClosest(aQuadsNormals,            lBegin, lEnd);
    scanForClosest(aQuadsNormalsColored,     lBegin, lEnd);
    scanForClosest(aTriangles,               lBegin, lEnd);
    scanForClosest(aTrianglesColored,        lBegin, lEnd);
    scanForClosest(aTrianglesNormals,        lBegin, lEnd);
    scanForClosest(aTrianglesNormalsColored, lBegin, lEnd);

    // Now add Points to aSimplified for each primitive barycenter chosen
    lIter = lBegin;

    while (lIter != lEnd) {
      GLV_ASSERT(lIter != 0);
      GLV_ASSERT(aSimplified != 0);
      aSimplified->addPoint(lIter->aPrimiticeBarycenter);
      ++lIter;
    }
  }

  // aSimplified is not dirty anymore
  aSimplifiedDirty = false;
}

