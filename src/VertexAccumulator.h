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

#ifndef VERTEXACCUMULATOR_H
#define VERTEXACCUMULATOR_H

#include "assert_glv.h"
#include "BoundingBox.h"
#include "glinclude.h"
#include "limits_glv.h"
#include "RenderParameters.h"
#include <string>
#include <vector>

class PrimitiveAccumulator;

// Class used to accumulate OpenGL based on a mesh
// (vertices) .  Contains the base data for VertexedPrimitiveAccumulator
//  : vertexes, colors and normals.
// This data may be shared between multiple VertexedPrimitiveAccumulator
class VertexAccumulator
{
public:

  VertexAccumulator ();
  ~VertexAccumulator();

  void  addColor           (const Vector3D&     pColor);

  void  addVertex          (const Vector3D&     pVertex);

  void  dumpCharacteristics(std::ostream&       pOstream,
                            const std::string&  pIndentation,
                            const Matrix4x4&    pTransformation) const;

  bool  getVerticesFrozen  () const;
  bool  getColorsFrozen    () const;

  bool  freezeColors       ();

  void  freezeVertices     ();

  typedef  std::vector<Vector3D>  Colors;
  typedef  std::vector<Vector3D>  Normals;
  typedef  std::vector<Vector3D>  Vertices;
  
private:

  // Block the use of those
  VertexAccumulator(const VertexAccumulator&);
  VertexAccumulator& operator=(const VertexAccumulator&);

  Colors                 aColors;
  bool                   aColorsFrozen;
  Normals                aNormals;
  bool                   aSimplifiedDirty;
  Vertices               aVertices;
  bool                   aVerticesFrozen;

public:

  const Colors& getColors() const;
  const Vertices& getVertices() const;
  Normals& getNormals();

};

#endif // VERTEXACCUMULATOR_H
