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

#ifndef OBJECT_H
#define OBJECT_H

#include "BoundingBox.h"
#include "glinclude.h"
#include "RenderParameters.h"
#include <map>
#include <string>
#include <vector>

class Parser;
class PrimitiveAccumulator;
class VertexAccumulator;
class VertexedPrimitiveAccumulator;

// Class used to manage the data associated with
// an Object displayed in OpenGL
class Object
{
public:

  Object ();
  ~Object();

  void                addCommand         (const std::string& pCommand,
                                          const std::string& pParameters,
                                          Parser&            pCurrentParser,
                                          std::string&       pError);

  void                deleteDisplayLists ();

  void                dumpCharacteristics(std::ostream&      pOstream,
                                          const std::string& pIndentation,
                                          const Matrix4x4&   pTransformation);

  const BoundingBox&  getBoundingBox     ();

  void                render             (RenderParameters&  pParams);

private:

  // Block the use of those
  Object(const Object&);
  Object& operator=(const Object&);

  typedef  std::vector<std::string>                    Commands;
  typedef  std::map<std::string, Object*>              IndexNamedObjects;
  typedef  std::vector<PrimitiveAccumulator*>          PrimitiveAccumulators;
  typedef  std::vector<Object*>                        SubObjects;
  typedef  std::vector<VertexAccumulator*>             VertexAccumulators;
  typedef  std::vector<VertexedPrimitiveAccumulator*>  VertexedPrimitiveAccumulators;

  enum RawMode {rawMode_arrow,
                rawMode_arrow_colored,
                rawMode_point,
                rawMode_point_colored,
                rawMode_point_v,
                rawMode_line,
                rawMode_line_colored,
                rawMode_line_v,
                rawMode_triangle,
                rawMode_triangle_colored,
                rawMode_triangle_v,
                rawMode_quad,
                rawMode_quad_colored,
                rawMode_quad_v,
                rawMode_vertex,
                rawMode_color_v,
                rawMode_not_in_raw_section};


  void                           constructDisplayList                  (RenderParameters&         pParams);

  void                           executeCommand                        (const std::string&        pCommand,
                                                                        RenderParameters&         pParams) const;

  PrimitiveAccumulator&          getCurrentPrimitiveAccumulator        ();

  VertexAccumulator&             getCurrentVertexAccumulator           ();

  VertexedPrimitiveAccumulator&  getCurrentVertexedPrimitiveAccumulator();

  GLuint&                        getGLDisplayList                      (const RenderParameters&   pParams);


  BoundingBox                    aBoundingBox;
  Commands                       aCommands;
  bool                           aFrozen;
  GLuint                         aGLDisplayListBoundingBox;
  GLuint                         aGLDisplayListFast;
  GLuint                         aGLDisplayListFull;
  std::string                    aName;
  bool                           aNewPrimitiveAccumulatorNeeded;
  bool                           aNewVertexAccumulatorNeeded;
  bool                           aNewVertexedPrimitiveAccumulatorNeeded;
  PrimitiveAccumulators          aPrimitiveAccumulators;
  RawMode                        aRawMode;
  int                            aRawModeArrowTipNbPolygons;
  float                          aRawModeArrowTipProportion;
  SubObjects                     aSubObjects;
  VertexAccumulators             aVertexAccumulators;
  VertexedPrimitiveAccumulators  aVertexedPrimitiveAccumulators;

};

#endif // OBJECT_H

