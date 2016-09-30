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

#ifndef PRIMITIVEACCUMULATOR_H
#define PRIMITIVEACCUMULATOR_H

#include "BoundingBox.h"
#include "glinclude.h"
#include "RenderParameters.h"
#include <string>
#include <vector>


// Class used to accumulate OpenGL
// primitives and create an optimized order to
// display them
class PrimitiveAccumulator
{
public:

  PrimitiveAccumulator (const bool pCreateSimplified);
  ~PrimitiveAccumulator();

  void  addArrow           (const Vector3D& pP1,
                            const Vector3D& pP2,
                            const float     pTipProportion,
                            const int       pTipNbPolygons);

  void  addArrowColored    (const Vector3D& pP1, const Vector3D& pC1,
                            const Vector3D& pP2, const Vector3D& pC2,
                            const float     pTipProportion,
                            const int       pTipNbPolygons);

  void  addSolidCube       (const Vector3D& pCenter,
                            const float     pHalfSize);

  void  addSolidSphere     (const Vector3D& pCenter,
                            const float     pRadius,
                            const int       pSlices,
                            const int       pStacks);

  void  addWireCube        (const Vector3D& pCenter,
                            const float     pHalfSize);

  void  addLine            (const Vector3D& pP1,
                            const Vector3D& pP2);

  void  addLineColored     (const Vector3D& pP1, const Vector3D& pC1,
                            const Vector3D& pP2, const Vector3D& pC2);

  void  addPoint           (const Vector3D& pP);

  void  addPointColored    (const Vector3D& pP,  const Vector3D& pC);

  void  addQuad            (const Vector3D& pP1,
                            const Vector3D& pP2,
                            const Vector3D& pP3,
                            const Vector3D& pP4);

  void  addQuadColored     (const Vector3D& pP1, const Vector3D& pC1,
                            const Vector3D& pP2, const Vector3D& pC2,
                            const Vector3D& pP3, const Vector3D& pC3,
                            const Vector3D& pP4, const Vector3D& pC4);

  void  addTriangle        (const Vector3D& pP1,
                            const Vector3D& pP2,
                            const Vector3D& pP3);

  void  addTriangleColored (const Vector3D& pP1, const Vector3D& pC1,
                            const Vector3D& pP2, const Vector3D& pC2,
                            const Vector3D& pP3, const Vector3D& pC3);

  void  dumpCharacteristics(std::ostream&      pOstream,
                            const std::string& pIndentation,
                            const Matrix4x4&   pTransformation);


  const  BoundingBox&  getBoundingBox() const;
  void                 render        (const RenderParameters& pParams);

private:

  // Block the use of those
  PrimitiveAccumulator();
  PrimitiveAccumulator(const PrimitiveAccumulator&);
  PrimitiveAccumulator& operator=(const PrimitiveAccumulator&);


  struct Line {
    Vector3D aP1;
    Vector3D aP2;

    Vector3D getBarycenter() const {
      return 0.5*(aP1 + aP2);
    }
  };

  struct LineColored : public Line {
    Vector3D aC1;
    Vector3D aC2;
  };

  struct Point {
    Vector3D aP;

    Vector3D getBarycenter() const {
      return aP;
    }
  };

  struct PointColored : public Point {
    Vector3D aC;
  };

  struct Quad {
    Vector3D aP1;
    Vector3D aP2;
    Vector3D aP3;
    Vector3D aP4;

    Vector3D getBarycenter() const {
      return 0.25*(aP1 + aP2 + aP3 + aP4);
    }

    void renderFacetsFrame() const {
      glBegin(GL_LINE_LOOP);
      glVertex3f(aP1.x(), aP1.y(), aP1.z());
      glVertex3f(aP2.x(), aP2.y(), aP2.z());
      glVertex3f(aP3.x(), aP3.y(), aP3.z());
      glVertex3f(aP4.x(), aP4.y(), aP4.z());
      glEnd();
    }
  };

  struct QuadColored : public Quad {
    Vector3D aC1;
    Vector3D aC2;
    Vector3D aC3;
    Vector3D aC4;
  };

  struct QuadNormals : public Quad {
    Vector3D aN1;
    Vector3D aN2;
    Vector3D aN3;
    Vector3D aN4;
  };

  struct QuadNormalsColored : public QuadNormals {
    Vector3D aC1;
    Vector3D aC2;
    Vector3D aC3;
    Vector3D aC4;
  };

  struct Triangle {
    Vector3D aP1;
    Vector3D aP2;
    Vector3D aP3;

    Vector3D getBarycenter() const {
      return (1.0/3.0)*(aP1 + aP2 + aP3);
    }

    void renderFacetsFrame() const {
      glBegin(GL_LINE_LOOP);
      glVertex3f(aP1.x(), aP1.y(), aP1.z());
      glVertex3f(aP2.x(), aP2.y(), aP2.z());
      glVertex3f(aP3.x(), aP3.y(), aP3.z());
      glEnd();
    }
  };

  struct TriangleColored : public Triangle {
    Vector3D aC1;
    Vector3D aC2;
    Vector3D aC3;
  };

  struct TriangleNormals : public Triangle {
    Vector3D aN1;
    Vector3D aN2;
    Vector3D aN3;
  };

  struct TriangleNormalsColored : public TriangleNormals {
    Vector3D aC1;
    Vector3D aC2;
    Vector3D aC3;
  };


  void  addArrowTip                  (const Vector3D&         pP1,
                                      const Vector3D&         pP2,
                                      const Vector3D&         pC2,
                                      const bool              pUseColor,
                                      const float             pTipProportion,
                                      const int               pTipNbPolygons);

  void  constructSimplified          ();
  void  renderFacetsFrame            (const RenderParameters& pParams);
  void  renderFull                   (const RenderParameters& pParams);
  void  renderLines                  ();
  void  renderLinesColored           ();
  void  renderPoints                 ();
  void  renderPointsColored          ();
  void  renderQuads                  ();
  void  renderQuadsColored           ();
  void  renderQuadsNormals           ();
  void  renderQuadsNormalsColored    ();
  void  renderSimplified             (const RenderParameters& pParams);
  void  renderTriangles              ();
  void  renderTrianglesColored       ();
  void  renderTrianglesNormals       ();
  void  renderTrianglesNormalsColored();


  typedef  std::vector<Line>                    Lines;
  typedef  std::vector<LineColored>             LinesColored;
  typedef  std::vector<Point>                   Points;
  typedef  std::vector<PointColored>            PointsColored;
  typedef  std::vector<Quad>                    Quads;
  typedef  std::vector<QuadColored>             QuadsColored;
  typedef  std::vector<QuadNormals>             QuadsNormals;
  typedef  std::vector<QuadNormalsColored>      QuadsNormalsColored;
  typedef  std::vector<Triangle>                Triangles;
  typedef  std::vector<TriangleColored>         TrianglesColored;
  typedef  std::vector<TriangleNormals>         TrianglesNormals;
  typedef  std::vector<TriangleNormalsColored>  TrianglesNormalsColored;
  typedef  Lines::size_type                     SizeType;

  mutable BoundingBox      aBoundingBox;
  Lines                    aLines;
  mutable SizeType         aLinesBBoxCounter;
  LinesColored             aLinesColored;
  mutable SizeType         aLinesColoredBBoxCounter;
  Points                   aPoints;
  mutable SizeType         aPointsBBoxCounter;
  PointsColored            aPointsColored;
  mutable SizeType         aPointsColoredBBoxCounter;
  Quads                    aQuads;
  mutable SizeType         aQuadsBBoxCounter;
  QuadsColored             aQuadsColored;
  mutable SizeType         aQuadsColoredBBoxCounter;
  QuadsNormals             aQuadsNormals;
  mutable SizeType         aQuadsNormalsBBoxCounter;
  QuadsNormalsColored      aQuadsNormalsColored;
  mutable SizeType         aQuadsNormalsColoredBBoxCounter;
  PrimitiveAccumulator*    aSimplified;
  bool                     aSimplifiedDirty;
  Triangles                aTriangles;
  mutable SizeType         aTrianglesBBoxCounter;
  TrianglesColored         aTrianglesColored;
  mutable SizeType         aTrianglesColoredBBoxCounter;
  TrianglesNormals         aTrianglesNormals;
  mutable SizeType         aTrianglesNormalsBBoxCounter;
  TrianglesNormalsColored  aTrianglesNormalsColored;
  mutable SizeType         aTrianglesNormalsColoredBBoxCounter;
  int                      aPrimitiveOptimizerValue;

};

#endif // PRIMITIVEACCUMULATOR_H
