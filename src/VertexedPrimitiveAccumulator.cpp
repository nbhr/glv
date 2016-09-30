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

#include "VertexedPrimitiveAccumulator.h"
#include "VertexAccumulator.h"
#include "limits_glv.h"
#include "PrimitiveAccumulator.h"
#include <cmath>
#include <iostream>
#include <stdio.h>

VertexedPrimitiveAccumulator::VertexedPrimitiveAccumulator(VertexAccumulator& pVertexes)
  : aBoundingBox                (),
    aColors                     (pVertexes.getColors()),
    aLines                      (),
    aLinesBBoxCounter           (0),
    aPoints                     (),
    aPointsBBoxCounter          (0),
    aQuads                      (),
    aQuadsBBoxCounter           (0),
    aNormals                    (pVertexes.getNormals()),
    aSimplified                 (),
    aSimplifiedDirty            (true),
    aSimplifiedSelf             (false),
    aTriangles                  (),
    aTrianglesBBoxCounter       (0),
    aVertices                   (pVertexes.getVertices()),
    aPrimitiveOptimizerValue    (100),
    aVertexAccumulator          (pVertexes)
{
  aSimplified = new PrimitiveAccumulator(false);
  GLV_ASSERT(aSimplified != 0);
}

VertexedPrimitiveAccumulator::~VertexedPrimitiveAccumulator()
{
  GLV_ASSERT(aSimplified != 0);
  delete aSimplified;
}

// Returns false if one of the parameters
// is out of range
bool VertexedPrimitiveAccumulator::addLine(const int pP1,
                                           const int pP2)
{
  const int  lSize      = static_cast<int>(aVertices.size());
  const bool lIndicesOk = (pP1 >= 0 && pP1 < lSize &&
                           pP2 >= 0 && pP2 < lSize);

  if (lIndicesOk) {
    GLV_ASSERT(aVertexAccumulator.getVerticesFrozen());

    Line lLine;
    lLine.aP1 = pP1;
    lLine.aP2 = pP2;

    aLines.push_back(lLine);

    aSimplifiedDirty = true;
  }

  return lIndicesOk;
}

// Returns false if the parameters is out of range
bool VertexedPrimitiveAccumulator::addPoint(const int pP)
{

  const int  lSize      = static_cast<int>(aVertices.size());
  const bool lIndicesOk = (pP >= 0 && pP < lSize);

  if (lIndicesOk) {
    GLV_ASSERT(aVertexAccumulator.getVerticesFrozen());

    Point lPoint;
    lPoint.aP = pP;

    aPoints.push_back(lPoint);

    aSimplifiedDirty = true;
  }

  return lIndicesOk;
}

// Returns false if one of the parameters
// is out of range
bool VertexedPrimitiveAccumulator::addQuad(const int pP1,
                                           const int pP2,
                                           const int pP3,
                                           const int pP4)
{
  const int  lSize      = static_cast<int>(aVertices.size());
  const bool lIndicesOk = (pP1 >= 0 && pP1 < lSize &&
                           pP2 >= 0 && pP2 < lSize &&
                           pP3 >= 0 && pP3 < lSize &&
                           pP4 >= 0 && pP4 < lSize);

  if (lIndicesOk) {
    GLV_ASSERT(aVertexAccumulator.getVerticesFrozen());

    Quad lQuad;
    lQuad.aP1 = pP1;
    lQuad.aP2 = pP2;
    lQuad.aP3 = pP3;
    lQuad.aP4 = pP4;

    aQuads.push_back(lQuad);

    aSimplifiedDirty = true;
  }

  return lIndicesOk;
}

// Returns false if one of the parameters
// is out of range
bool VertexedPrimitiveAccumulator::addTriangle(const int pP1,
                                               const int pP2,
                                               const int pP3)
{

  const int  lSize      = static_cast<int>(aVertices.size());
  const bool lIndicesOk = (pP1 >= 0 && pP1 < lSize &&
                           pP2 >= 0 && pP2 < lSize &&
                           pP3 >= 0 && pP3 < lSize);

  if (lIndicesOk) {
    GLV_ASSERT(aVertexAccumulator.getVerticesFrozen());

    Triangle lTriangle;
    lTriangle.aP1 = pP1;
    lTriangle.aP2 = pP2;
    lTriangle.aP3 = pP3;

    aTriangles.push_back(lTriangle);
    aSimplifiedDirty = true;
  }

  return lIndicesOk;
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

// Dump in ASCII the caracteristics of the VertexedPrimitiveAccumulator
void VertexedPrimitiveAccumulator::dumpCharacteristics(std::ostream&       pOstream,
                                                       const std::string&  pIndentation,
                                                       const Matrix4x4&    pTransformation)
{
  pOstream << pIndentation << "VertexedPrimitiveAccumulator " << std::endl;
  std::string lIndentation = pIndentation + "  ";

  // Dump the data of the shared vertexes
  aVertexAccumulator.dumpCharacteristics(pOstream,lIndentation,pTransformation);

  // Update the bounding box. But we use the attribute
  // directly just after the call because we need it non-const
  getBoundingBox();
  aBoundingBox.dumpCharacteristics(pOstream, lIndentation, pTransformation);

#ifdef GLV_DUMP_SIMPLICATION
  if (!aSimplifiedSelf) {
    pOstream << lIndentation << "Simplified "  << std::endl;
    aSimplified->dumpCharacteristics(pOstream, lIndentation + "  ", pTransformation);
  }
#endif // #ifdef GLV_DUMP_SIMPLICATION

#ifdef GLV_DUMP_MEMORY_USAGE
  {

    pOstream << lIndentation << "Memory used by the VertexedPrimitiveAccumulator = " << sizeof(*this) << std::endl;

    pOstream << lIndentation << "Memory used by aPoints    = " << getStringSizeAndCapacity(aPoints   ) << std::endl;
    pOstream << lIndentation << "Memory used by aLines     = " << getStringSizeAndCapacity(aLines    ) << std::endl;
    pOstream << lIndentation << "Memory used by aTriangles = " << getStringSizeAndCapacity(aTriangles) << std::endl;
    pOstream << lIndentation << "Memory used by aQuads     = " << getStringSizeAndCapacity(aQuads    ) << std::endl;

  }
#endif // #ifdef GLV_DUMP_MEMORY_USAGE

  pOstream << lIndentation << "Number of point_v    = " << aPoints    .size() << std::endl;
  pOstream << lIndentation << "Number of line_v     = " << aLines     .size() << std::endl;
  pOstream << lIndentation << "Number of triangle_v = " << aTriangles .size() << std::endl;
  pOstream << lIndentation << "Number of quad_v     = " << aQuads     .size() << std::endl;
}

const BoundingBox& VertexedPrimitiveAccumulator::getBoundingBox() const
{

  // We could take for granted that all the vertices are
  // used by at least a primitive. But since there is a
  // chance that one vertex could not be used, we the
  // primitives instead.

  GLV_ASSERT(aLinesBBoxCounter     <= aLines.size());
  GLV_ASSERT(aPointsBBoxCounter    <= aPoints.size());
  GLV_ASSERT(aQuadsBBoxCounter     <= aQuads.size());
  GLV_ASSERT(aTrianglesBBoxCounter <= aTriangles.size());

  // Scan only the new primitives for each type
  {
    const SizeType lSize = aLines.size();

    while (aLinesBBoxCounter < lSize) {
      const Line& lLine = aLines[aLinesBBoxCounter];
      GLV_ASSERT(lLine.aP1 >= 0 && lLine.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lLine.aP2 >= 0 && lLine.aP2 < static_cast<int>(aVertices.size()));
      aBoundingBox += aVertices[lLine.aP1];
      aBoundingBox += aVertices[lLine.aP2];
      ++aLinesBBoxCounter;
    }
  }

  {
    const SizeType lSize = aPoints.size();

    while (aPointsBBoxCounter < lSize) {
      const Point& lPoint = aPoints[aPointsBBoxCounter];
      GLV_ASSERT(lPoint.aP >= 0 && lPoint.aP < static_cast<int>(aVertices.size()));
      aBoundingBox += aVertices[lPoint.aP];
      ++aPointsBBoxCounter;
    }
  }

  {
    const SizeType lSize = aQuads.size();

    while (aQuadsBBoxCounter < lSize) {
      const Quad& lQuad = aQuads[aQuadsBBoxCounter];
      GLV_ASSERT(lQuad.aP1 >= 0 && lQuad.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP2 >= 0 && lQuad.aP2 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP3 >= 0 && lQuad.aP3 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP4 >= 0 && lQuad.aP4 < static_cast<int>(aVertices.size()));
      aBoundingBox += aVertices[lQuad.aP1];
      aBoundingBox += aVertices[lQuad.aP2];
      aBoundingBox += aVertices[lQuad.aP3];
      aBoundingBox += aVertices[lQuad.aP4];
      ++aQuadsBBoxCounter;
    }
  }

  {
    const SizeType lSize = aTriangles.size();

    while (aTrianglesBBoxCounter < lSize) {
      const Triangle& lTriangle = aTriangles[aTrianglesBBoxCounter];
      GLV_ASSERT(lTriangle.aP1 >= 0 && lTriangle.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lTriangle.aP2 >= 0 && lTriangle.aP2 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lTriangle.aP3 >= 0 && lTriangle.aP3 < static_cast<int>(aVertices.size()));
      aBoundingBox += aVertices[lTriangle.aP1];
      aBoundingBox += aVertices[lTriangle.aP2];
      aBoundingBox += aVertices[lTriangle.aP3];
      ++aTrianglesBBoxCounter;
    }
  }

  return aBoundingBox;
}

void VertexedPrimitiveAccumulator::computeNormals()
{
  GLV_ASSERT(aNormals.size() != aVertices.size() || aVertices.size() == 0);

  const Vector3D lNullVector     (0.0f, 0.0f, 0.0f);
  const Vector3D lArbitraryNormal(1.0f, 0.0f, 0.0f);

  aNormals.resize(aVertices.size());
  std::fill(aNormals.begin(), aNormals.end(), lNullVector);

  Quads::const_iterator       lIterQuads    = aQuads.begin();
  const Quads::const_iterator lIterQuadsEnd = aQuads.end  ();

  while (lIterQuads != lIterQuadsEnd) {

    const Quad& lQuad = *lIterQuads;
    GLV_ASSERT(lQuad.aP1 >= 0 && lQuad.aP1 < static_cast<int>(aVertices.size()));
    GLV_ASSERT(lQuad.aP2 >= 0 && lQuad.aP2 < static_cast<int>(aVertices.size()));
    GLV_ASSERT(lQuad.aP3 >= 0 && lQuad.aP3 < static_cast<int>(aVertices.size()));
    GLV_ASSERT(lQuad.aP4 >= 0 && lQuad.aP4 < static_cast<int>(aVertices.size()));
    const Vector3D& lP1 = aVertices[lQuad.aP1];
    const Vector3D& lP2 = aVertices[lQuad.aP2];
    const Vector3D& lP3 = aVertices[lQuad.aP3];
    const Vector3D& lP4 = aVertices[lQuad.aP4];

    // Compute normals
    Vector3D lN1 = (lP2-lP1).crossProduct(lP4-lP1);
    Vector3D lN2 = (lP3-lP2).crossProduct(lP1-lP2);
    Vector3D lN3 = (lP4-lP3).crossProduct(lP2-lP3);
    Vector3D lN4 = (lP1-lP4).crossProduct(lP3-lP4);

    // We do not normalize the normals at this points
    // This plays the role of weights giving more
    // importance to bigger polygons
    aNormals[lQuad.aP1] += lN1;
    aNormals[lQuad.aP2] += lN2;
    aNormals[lQuad.aP3] += lN3;
    aNormals[lQuad.aP4] += lN4;

    ++lIterQuads;
  }

  Triangles::const_iterator       lIterTriangles    = aTriangles.begin();
  const Triangles::const_iterator lIterTrianglesEnd = aTriangles.end  ();

  while (lIterTriangles != lIterTrianglesEnd) {

    const Triangle& lTriangle = *lIterTriangles;
    GLV_ASSERT(lTriangle.aP1 >= 0 && lTriangle.aP1 < static_cast<int>(aVertices.size()));
    GLV_ASSERT(lTriangle.aP2 >= 0 && lTriangle.aP2 < static_cast<int>(aVertices.size()));
    GLV_ASSERT(lTriangle.aP3 >= 0 && lTriangle.aP3 < static_cast<int>(aVertices.size()));
    const Vector3D& lP1 = aVertices[lTriangle.aP1];
    const Vector3D& lP2 = aVertices[lTriangle.aP2];
    const Vector3D& lP3 = aVertices[lTriangle.aP3];

    // Compute normals
    Vector3D lN = (lP2-lP1).crossProduct(lP3-lP1);

    // We do not normalize the normals at this points
    // This plays the role of weights giving more
    // importance to bigger polygons
    aNormals[lTriangle.aP1] += lN;
    aNormals[lTriangle.aP2] += lN;
    aNormals[lTriangle.aP3] += lN;

    ++lIterTriangles;
  }

  Normals::iterator       lIterNormals    = aNormals.begin();
  const Normals::iterator lIterNormalsEnd = aNormals.end  ();

  while (lIterNormals != lIterNormalsEnd) {
    // There is the possibility that a vertex doesn't
    // have adjacent Quads or Triangles. So we have to
    // check that
    Vector3D& lNormal = *lIterNormals;
    if (!(lNormal == lNullVector)) {
      lNormal.normalize();
    }
    else {
      // But there is also the possibility that two
      // adjacent Quad and/or Triangle are facing in
      // opposite directions. This would cause problems
      // later on, so we put an arbitrary unity vector
      // for the normal
      lNormal = lArbitraryNormal;
    }
    ++lIterNormals;
  }
}

void VertexedPrimitiveAccumulator::render(const RenderParameters& pParams)
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
    // Should not happen!
    GLV_ASSERT(false);
  }
}

// Renders the content of aLines
void VertexedPrimitiveAccumulator::renderFacetsFrame(const RenderParameters& pParams)
{
  const SizeType lNbFacets = aQuads.size() + aTriangles.size();

  // If facet drawing is enabled and we've got something to draw
  if(pParams.aFlagRenderFacetFrame && lNbFacets > 0) {

    // We push the attributes on the openGL attribute stack; so
    // we dont disturb the current values of color (current) or line width (line)
    glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
    // Warning: performance hit; when a lot of push(GL_LIGHTING_BIT) calls are made
    glDisable(GL_LIGHTING);
    glLineWidth(1);
    glColor3f(pParams.aFacetBoundaryR, pParams.aFacetBoundaryG, pParams.aFacetBoundaryB);

    Quads::const_iterator       lIterQuads    = aQuads.begin();
    const Quads::const_iterator lIterQuadsEnd = aQuads.end  ();

    while (lIterQuads != lIterQuadsEnd) {

      const Quad& lQuad = *lIterQuads;
      GLV_ASSERT(lQuad.aP1 >= 0 && lQuad.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP2 >= 0 && lQuad.aP2 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP3 >= 0 && lQuad.aP3 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP4 >= 0 && lQuad.aP4 < static_cast<int>(aVertices.size()));
      const Vector3D& lP1 = aVertices[lQuad.aP1];
      const Vector3D& lP2 = aVertices[lQuad.aP2];
      const Vector3D& lP3 = aVertices[lQuad.aP3];
      const Vector3D& lP4 = aVertices[lQuad.aP4];

      glBegin(GL_LINE_LOOP);
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());
      glVertex3f(lP4.x(), lP4.y(), lP4.z());
      glEnd();

      ++lIterQuads;
    }

    Triangles::const_iterator       lIterTriangles    = aTriangles.begin();
    const Triangles::const_iterator lIterTrianglesEnd = aTriangles.end  ();

    while (lIterTriangles != lIterTrianglesEnd) {

      const Triangle& lTriangle = *lIterTriangles;
      GLV_ASSERT(lTriangle.aP1 >= 0 && lTriangle.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lTriangle.aP2 >= 0 && lTriangle.aP2 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lTriangle.aP3 >= 0 && lTriangle.aP3 < static_cast<int>(aVertices.size()));
      const Vector3D& lP1 = aVertices[lTriangle.aP1];
      const Vector3D& lP2 = aVertices[lTriangle.aP2];
      const Vector3D& lP3 = aVertices[lTriangle.aP3];

      glBegin(GL_LINE_LOOP);
      glVertex3f(lP1.x(), lP1.y(), lP1.z());
      glVertex3f(lP2.x(), lP2.y(), lP2.z());
      glVertex3f(lP3.x(), lP3.y(), lP3.z());
      glEnd();

      ++lIterTriangles;
    }

    // Revert the lighting state and the line state
    glPopAttrib();
  }
}

void VertexedPrimitiveAccumulator::renderFull(const RenderParameters& pParams)
{
  aPrimitiveOptimizerValue = pParams.aPrimitiveOptimizerValue;
  if (pParams.aFlagSmoothNormals == true) {
    if (aNormals.size() != aVertices.size()) {
      computeNormals();
    }
  }
  renderFacetsFrame(pParams);
  renderLines      ();
  renderPoints     ();
  renderQuads      ();
  renderTriangles  ();
}

// Renders the content of aLines
void VertexedPrimitiveAccumulator::renderLines()
{
  int lPrimitiveCount = 0;

  if (aLines.size() > 0) {

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    Lines::const_iterator       lIterLines    = aLines.begin();
    const Lines::const_iterator lIterLinesEnd = aLines.end  ();

    while (lIterLines != lIterLinesEnd) {

      const Line& lLine = *lIterLines;
      GLV_ASSERT(lLine.aP1 >= 0 && lLine.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lLine.aP2 >= 0 && lLine.aP2 < static_cast<int>(aVertices.size()));
      const Vector3D& lP1 = aVertices[lLine.aP1];
      const Vector3D& lP2 = aVertices[lLine.aP2];

      if (aColors.size() == aVertices.size()) {
        GLV_ASSERT(aColors.size() != 0);
        const Vector3D& lC1 = aColors[lLine.aP1];
        const Vector3D& lC2 = aColors[lLine.aP2];

        glColor3f (lC1.x(), lC1.y(), lC1.z());
        glVertex3f(lP1.x(), lP1.y(), lP1.z());
        glColor3f (lC2.x(), lC2.y(), lC2.z());
        glVertex3f(lP2.x(), lP2.y(), lP2.z());
      }
      else {
        GLV_ASSERT(aColors.size() == 0);
        glVertex3f(lP1.x(), lP1.y(), lP1.z());
        glVertex3f(lP2.x(), lP2.y(), lP2.z());
      }

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

// Renders the content of aPoints
void VertexedPrimitiveAccumulator::renderPoints()
{
  if (aPoints.size() > 0) {

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);

    Points::const_iterator       lIterPoints    = aPoints.begin();
    const Points::const_iterator lIterPointsEnd = aPoints.end  ();

    while (lIterPoints != lIterPointsEnd) {

      const Point& lPoint = *lIterPoints;
      GLV_ASSERT(lPoint.aP >= 0 && lPoint.aP < static_cast<int>(aVertices.size()));
      const Vector3D& lP = aVertices[lPoint.aP];

      if (aColors.size() == aVertices.size()) {
        GLV_ASSERT(aColors.size() != 0);
        const Vector3D& lC = aColors[lPoint.aP];

        glColor3f (lC.x(), lC.y(), lC.z());
        glVertex3f(lP.x(), lP.y(), lP.z());
      }
      else {
        GLV_ASSERT(aColors.size() == 0);
        glVertex3f(lP.x(), lP.y(), lP.z());
      }

      ++lIterPoints;
    }

    glEnd();

    // Revert the lighting state
    glPopAttrib();

  }
}

// Renders the content of aQuads
void VertexedPrimitiveAccumulator::renderQuads()
{
  int lPrimitiveCount = 0;

  if (aQuads.size() > 0) {

    glBegin(GL_QUADS);

    const Vector3D lNullVector     (0.0f, 0.0f, 0.0f);
    const Vector3D lArbitraryNormal(1.0f, 0.0f, 0.0f);

    Quads::const_iterator       lIterQuads    = aQuads.begin();
    const Quads::const_iterator lIterQuadsEnd = aQuads.end  ();

    while (lIterQuads != lIterQuadsEnd) {

      const Quad& lQuad = *lIterQuads;
      GLV_ASSERT(lQuad.aP1 >= 0 && lQuad.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP2 >= 0 && lQuad.aP2 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP3 >= 0 && lQuad.aP3 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lQuad.aP4 >= 0 && lQuad.aP4 < static_cast<int>(aVertices.size()));
      const Vector3D& lP1 = aVertices[lQuad.aP1];
      const Vector3D& lP2 = aVertices[lQuad.aP2];
      const Vector3D& lP3 = aVertices[lQuad.aP3];
      const Vector3D& lP4 = aVertices[lQuad.aP4];
      Vector3D        lN1;
      Vector3D        lN2;
      Vector3D        lN3;
      Vector3D        lN4;

      if (aNormals.size() == aVertices.size()) {
        GLV_ASSERT(aNormals.size() != 0);
        lN1 = aNormals[lQuad.aP1];
        lN2 = aNormals[lQuad.aP2];
        lN3 = aNormals[lQuad.aP3];
        lN4 = aNormals[lQuad.aP4];
      }
      else {
        GLV_ASSERT(aNormals.size() == 0);
        lN1 = (lP2-lP1).crossProduct(lP4-lP1);
        lN2 = (lP3-lP2).crossProduct(lP1-lP2);
        lN3 = (lP4-lP3).crossProduct(lP2-lP3);
        lN4 = (lP1-lP4).crossProduct(lP3-lP4);

        // For degenerated Quads (unfortunately, we can't
        // prevent this from happening since it depends
        // on the user's input)
        if (lN1 != lNullVector) {
          lN1.normalize();
        }
        else {
          lN1 = lArbitraryNormal;
        }

        if (lN2 != lNullVector) {
          lN2.normalize();
        }
        else {
          lN2 = lArbitraryNormal;
        }

        if (lN3 != lNullVector) {
          lN3.normalize();
        }
        else {
          lN3 = lArbitraryNormal;
        }

        if (lN4 != lNullVector) {
          lN4.normalize();
        }
        else {
          lN4 = lArbitraryNormal;
        }
      }

      GLV_ASSERT(fabs(lN1.getLength() - 1.0f) < 1.0E-6);
      GLV_ASSERT(fabs(lN2.getLength() - 1.0f) < 1.0E-6);
      GLV_ASSERT(fabs(lN3.getLength() - 1.0f) < 1.0E-6);
      GLV_ASSERT(fabs(lN4.getLength() - 1.0f) < 1.0E-6);

      if (aColors.size() == aVertices.size()) {
        GLV_ASSERT(aColors.size() != 0);
        const Vector3D& lC1 = aColors[lQuad.aP1];
        const Vector3D& lC2 = aColors[lQuad.aP2];
        const Vector3D& lC3 = aColors[lQuad.aP3];
        const Vector3D& lC4 = aColors[lQuad.aP4];

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
      }
      else {
        GLV_ASSERT(aColors.size() == 0);
        glNormal3f(lN1.x(), lN1.y(), lN1.z());
        glVertex3f(lP1.x(), lP1.y(), lP1.z());
        glNormal3f(lN2.x(), lN2.y(), lN2.z());
        glVertex3f(lP2.x(), lP2.y(), lP2.z());
        glNormal3f(lN3.x(), lN3.y(), lN3.z());
        glVertex3f(lP3.x(), lP3.y(), lP3.z());
        glNormal3f(lN4.x(), lN4.y(), lN4.z());
        glVertex3f(lP4.x(), lP4.y(), lP4.z());
      }

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

void VertexedPrimitiveAccumulator::renderSimplified(const RenderParameters& pParams)
{
  // Check if we have to update the Simplified model
  if (aSimplifiedDirty) {
    constructSimplified();
  }

  RenderParameters lParams = pParams;
  lParams.aRenderMode      = RenderParameters::renderMode_full;

  // If there is no need for simplication, we
  // just render *this
  if (aSimplifiedSelf) {
    renderFull(lParams);
  }
  else {

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
  }
}

// Renders the content of aTriangles
void VertexedPrimitiveAccumulator::renderTriangles()
{
  int lPrimitiveCount = 0;

  if (aTriangles.size() > 0) {

    glBegin(GL_TRIANGLES);

    const Vector3D lNullVector     (0.0f, 0.0f, 0.0f);
    const Vector3D lArbitraryNormal(1.0f, 0.0f, 0.0f);

    Triangles::const_iterator       lIterTriangles    = aTriangles.begin();
    const Triangles::const_iterator lIterTrianglesEnd = aTriangles.end  ();

    while (lIterTriangles != lIterTrianglesEnd) {

      const Triangle& lTriangle = *lIterTriangles;
      GLV_ASSERT(lTriangle.aP1 >= 0 && lTriangle.aP1 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lTriangle.aP2 >= 0 && lTriangle.aP2 < static_cast<int>(aVertices.size()));
      GLV_ASSERT(lTriangle.aP3 >= 0 && lTriangle.aP3 < static_cast<int>(aVertices.size()));
      const Vector3D& lP1 = aVertices[lTriangle.aP1];
      const Vector3D& lP2 = aVertices[lTriangle.aP2];
      const Vector3D& lP3 = aVertices[lTriangle.aP3];
      Vector3D        lN1;
      Vector3D        lN2;
      Vector3D        lN3;

      if (aNormals.size() == aVertices.size()) {
        GLV_ASSERT(aNormals.size() != 0);
        lN1 = aNormals[lTriangle.aP1];
        lN2 = aNormals[lTriangle.aP2];
        lN3 = aNormals[lTriangle.aP3];
      }
      else {
        GLV_ASSERT(aNormals.size() == 0);
        lN1 = (lP2-lP1).crossProduct(lP3-lP1);

        // For degenerated Quads (unfortunately, we can't
        // prevent this from happening since it depends
        // on the user's input)
        if (lN1 != lNullVector) {
          lN1.normalize();
        }
        else {
          lN1 = lArbitraryNormal;
        }

        lN2 = lN1;
        lN3 = lN1;
      }

      GLV_ASSERT(fabs(lN1.getLength() - 1.0f) < 1.0E-6);
      GLV_ASSERT(fabs(lN2.getLength() - 1.0f) < 1.0E-6);
      GLV_ASSERT(fabs(lN3.getLength() - 1.0f) < 1.0E-6);

      if (aColors.size() == aVertices.size()) {
        GLV_ASSERT(aColors.size() != 0);
        const Vector3D& lC1 = aColors[lTriangle.aP1];
        const Vector3D& lC2 = aColors[lTriangle.aP2];
        const Vector3D& lC3 = aColors[lTriangle.aP3];

        glNormal3f(lN1.x(), lN1.y(), lN1.z());
        glColor3f (lC1.x(), lC1.y(), lC1.z());
        glVertex3f(lP1.x(), lP1.y(), lP1.z());
        glNormal3f(lN2.x(), lN2.y(), lN2.z());
        glColor3f (lC2.x(), lC2.y(), lC2.z());
        glVertex3f(lP2.x(), lP2.y(), lP2.z());
        glNormal3f(lN3.x(), lN3.y(), lN3.z());
        glColor3f (lC3.x(), lC3.y(), lC3.z());
        glVertex3f(lP3.x(), lP3.y(), lP3.z());
      }
      else {
        GLV_ASSERT(aColors.size() == 0);
        glNormal3f(lN1.x(), lN1.y(), lN1.z());
        glVertex3f(lP1.x(), lP1.y(), lP1.z());
        glNormal3f(lN2.x(), lN2.y(), lN2.z());
        glVertex3f(lP2.x(), lP2.y(), lP2.z());
        glNormal3f(lN3.x(), lN3.y(), lN3.z());
        glVertex3f(lP3.x(), lP3.y(), lP3.z());
      }

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

template <class Container, class Vertices>
inline void scanForClosest(Container&           pContainer,
                           SimplificationPoint* pIterSPBegin,
                           SimplificationPoint* pIterSPEnd,
                           const Vertices&      aVertices)
{
  const typename Container::size_type lSize = pContainer.size();

  for (typename Container::size_type i=0; i<lSize; ++i) {

    const typename Container::value_type&  lPrimitive = pContainer[i];
    SimplificationPoint*                   lIterSP    = pIterSPBegin;

    while (lIterSP != pIterSPEnd) {
      SimplificationPoint& lSP         = *lIterSP;
      const Vector3D       lBarycenter = lPrimitive .getBarycenter(aVertices);
      const float          lDistance   = lBarycenter.getDistanceTo(lSP.aPoint);
      if (lDistance < lSP.aBarycenterDistance) {
        lSP.aPrimiticeBarycenter = lBarycenter;
        lSP.aBarycenterDistance  = lDistance;
      }
      ++lIterSP;
    }
  }
}

void VertexedPrimitiveAccumulator::constructSimplified()
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
    delete aSimplified;
    aSimplified     = new PrimitiveAccumulator(false);
    aSimplifiedSelf = false;

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

    scanForClosest(aLines,     lBegin, lEnd, aVertices);
    scanForClosest(aPoints,    lBegin, lEnd, aVertices);
    scanForClosest(aQuads,     lBegin, lEnd, aVertices);
    scanForClosest(aTriangles, lBegin, lEnd, aVertices);

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

