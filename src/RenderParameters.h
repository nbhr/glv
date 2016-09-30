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

#ifndef RENDERPARAMETERS_H
#define RENDERPARAMETERS_H

// Parameter structure to share parameters
// with the cmd_* commands.
class RenderParameters
{
public:

  enum RenderMode {renderMode_full,
                   renderMode_bounding_box,
                   renderMode_fast};

  bool       aFlagSmoothNormals;    // Smooth normals, where applicable, and apply them to the primitive
  bool       aFlagRenderFacetFrame; // Render the boundaries of a facet
  bool       aFlagDoubleSided;      // polygons are not culled but also drawn if seen from the back side
  float      aFacetBoundaryR;
  float      aFacetBoundaryG;
  float      aFacetBoundaryB;
  int        aPrimitiveOptimizerValue; // Determine the maximum length of a glBegin/glEnd sequence
                                       //  default value of 100 is an acceptable performance/memory
                                       //  tradeoff.
  RenderMode aRenderMode;

  RenderParameters()
    : aFlagSmoothNormals      (false),
      aFlagRenderFacetFrame   (false),
      aFlagDoubleSided        (false),
      aFacetBoundaryR         (0.0f),
      aFacetBoundaryG         (0.0f),
      aFacetBoundaryB         (0.0f),
      aPrimitiveOptimizerValue(100),
      aRenderMode             (renderMode_full)
    {}

private:

};

#endif // RENDERPARAMETERS_H

