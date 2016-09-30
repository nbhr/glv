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

#ifndef VERTEXEDPRIMITIVEACCUMULATOR_H
#define VERTEXEDPRIMITIVEACCUMULATOR_H

#include "assert_glv.h"
#include "BoundingBox.h"
#include "glinclude.h"
#include "limits_glv.h"
#include "RenderParameters.h"
#include "VertexAccumulator.h"
#include <string>
#include <vector>

class PrimitiveAccumulator;

// Class used to accumulate OpenGL based on a mesh
// (vertices) and create an optimized order to display
// them
class VertexedPrimitiveAccumulator
{
public:

  VertexedPrimitiveAccumulator (VertexAccumulator& pVertexes);
  ~VertexedPrimitiveAccumulator();

  bool  addLine            (const int           pVertexIndex1,
                            const int           pVertexIndex2);

  bool  addPoint           (const int           pVertexIndex);

  bool  addQuad            (const int           pVertexIndex1,
                            const int           pVertexIndex2,
                            const int           pVertexIndex3,
                            const int           pVertexIndex4);

  bool  addTriangle        (const int           pVertexIndex1,
                            const int           pVertexIndex2,
                            const int           pVertexIndex3);

  void  dumpCharacteristics(std::ostream&       pOstream,
                            const std::string&  pIndentation,
                            const Matrix4x4&    pTransformation);

  const  BoundingBox&  getBoundingBox() const;

  void                 render        (const RenderParameters& pParams);

private:

  typedef VertexAccumulator::Vertices Vertices;
  typedef VertexAccumulator::Normals Normals;
  // Block the use of those
  VertexedPrimitiveAccumulator(const VertexedPrimitiveAccumulator&);
  VertexedPrimitiveAccumulator& operator=(const VertexedPrimitiveAccumulator&);

  struct Line {
    int aP1;
    int aP2;

    Vector3D getBarycenter(const Vertices& pVertices) const {
      GLV_ASSERT(aP1 >= 0 && aP1 < static_cast<int>(pVertices.size()));
      GLV_ASSERT(aP2 >= 0 && aP2 < static_cast<int>(pVertices.size()));
      return 0.5*(pVertices[aP1] + pVertices[aP2]);
    }
  };

  struct Point {
    int aP;

    Vector3D getBarycenter(const Vertices& pVertices) const {
      GLV_ASSERT(aP >= 0 && aP < static_cast<int>(pVertices.size()));
      return pVertices[aP];
    }
  };

  struct Quad {
    int aP1;
    int aP2;
    int aP3;
    int aP4;

    Vector3D getBarycenter(const Vertices& pVertices) const {
      GLV_ASSERT(aP1 >= 0 && aP1 < static_cast<int>(pVertices.size()));
      GLV_ASSERT(aP2 >= 0 && aP2 < static_cast<int>(pVertices.size()));
      GLV_ASSERT(aP3 >= 0 && aP3 < static_cast<int>(pVertices.size()));
      GLV_ASSERT(aP4 >= 0 && aP4 < static_cast<int>(pVertices.size()));
      return 0.25*(pVertices[aP1] + pVertices[aP2] + pVertices[aP3] + pVertices[aP4]);
    }
  };

  struct Triangle {
    int aP1;
    int aP2;
    int aP3;

    Vector3D getBarycenter(const Vertices& pVertices) const {
      GLV_ASSERT(aP1 >= 0 && aP1 < static_cast<int>(pVertices.size()));
      GLV_ASSERT(aP2 >= 0 && aP2 < static_cast<int>(pVertices.size()));
      GLV_ASSERT(aP3 >= 0 && aP3 < static_cast<int>(pVertices.size()));
      return (1.0/3.0)*(pVertices[aP1] + pVertices[aP2] + pVertices[aP3]);
    }
  };

  typedef  std::vector<Line>      Lines;
  typedef  std::vector<Point>     Points;
  typedef  std::vector<Quad>      Quads;
  typedef  std::vector<Triangle>  Triangles;
  typedef  Lines::size_type       SizeType;


  void  computeNormals        ();
  void  constructSimplified   ();
  void  renderFacetsFrame     (const RenderParameters& pParams);
  void  renderFull            (const RenderParameters& pParams);
  void  renderLines           ();
  void  renderLinesColored    ();
  void  renderPoints          ();
  void  renderPointsColored   ();
  void  renderQuads           ();
  void  renderQuadsColored    ();
  void  renderSimplified      (const RenderParameters& pParams);
  void  renderTriangles       ();
  void  renderTrianglesColored();


  mutable BoundingBox    aBoundingBox;
  const VertexAccumulator::Colors& aColors;
  Lines                  aLines;
  mutable SizeType       aLinesBBoxCounter;
  Points                 aPoints;
  mutable SizeType       aPointsBBoxCounter;
  Quads                  aQuads;
  mutable SizeType       aQuadsBBoxCounter;
  VertexAccumulator::Normals& aNormals;
  PrimitiveAccumulator*  aSimplified;
  bool                   aSimplifiedDirty;
  bool                   aSimplifiedSelf;
  Triangles              aTriangles;
  mutable SizeType       aTrianglesBBoxCounter;
  const VertexAccumulator::Vertices& aVertices;
  int                    aPrimitiveOptimizerValue;

  const VertexAccumulator& aVertexAccumulator;
};

#endif // VERTEXEDPRIMITIVEACCUMULATOR_H
