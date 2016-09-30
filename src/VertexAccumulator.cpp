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

#include "VertexAccumulator.h"
#include "limits_glv.h"
#include <cmath>
#include <iostream>
#include <stdio.h>

VertexAccumulator::VertexAccumulator()
  :
    aColors                     (),
    aColorsFrozen               (false),
    aNormals                    (),
    aSimplifiedDirty            (true),
    aVertices                   (),
    aVerticesFrozen             (false)
{
}

VertexAccumulator::~VertexAccumulator()
{
}

void VertexAccumulator::addColor(const Vector3D& pColor)
{
  aColors.push_back(pColor);

  aSimplifiedDirty = true;
}

void VertexAccumulator::addVertex(const Vector3D& pVertex)
{
  aVertices.push_back(pVertex);

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

// Dump in ASCII the caracteristics of the VertexAccumulator
void VertexAccumulator::dumpCharacteristics(std::ostream&       pOstream,
                                            const std::string&  pIndentation,
                                            const Matrix4x4&    pTransformation) const
{
  pOstream << pIndentation << "VertexAccumulator " << std::endl;
  std::string lIndentation = pIndentation + "  ";

  if (aVerticesFrozen) {
    pOstream << lIndentation << "Vertices frozen = true" << std::endl;
  }
  else {
    pOstream << lIndentation << "Vertices frozen = false" << std::endl;
  }

  if (aColorsFrozen) {
    pOstream << lIndentation << "Colors frozen   = true" << std::endl;
  }
  else {
    pOstream << lIndentation << "Colors frozen   = false" << std::endl;
  }

#ifdef GLV_DUMP_MEMORY_USAGE
  {
    pOstream << lIndentation << "Memory used by the VertexAccumulator = " << sizeof(*this) << std::endl;

    pOstream << lIndentation << "Memory used by aColors   = " << getStringSizeAndCapacity(aColors  ) << std::endl;
    pOstream << lIndentation << "Memory used by aNormals  = " << getStringSizeAndCapacity(aNormals ) << std::endl;
    pOstream << lIndentation << "Memory used by aVertices = " << getStringSizeAndCapacity(aVertices) << std::endl;

  }
#endif // #ifdef GLV_DUMP_MEMORY_USAGE

  pOstream << lIndentation << "Number of vertex  = " << aVertices  .size() << std::endl;
  pOstream << lIndentation << "Number of color_v = " << aColors    .size() << std::endl;
}

// Tell the VertexAccumulator that no more colors
// will be added to *this.
// Returns false if the number of colors is different
// from the number of vertices (this i bad and should
// be used to generate an error message
bool VertexAccumulator::freezeColors()
{
  GLV_ASSERT(aVerticesFrozen);
  aColorsFrozen = true;

  const bool lColorsOk = (aColors.size() == aVertices.size());

  if (!lColorsOk) {
    // Remove the colors since they're not usable
    std::vector<Vector3D> lTmp;
    aColors.swap(lTmp);
  }

  return lColorsOk;
}

// Tell the VertexAccumulator that no more vertices
// will be added to *this.
void VertexAccumulator::freezeVertices()
{
  GLV_ASSERT(!aColorsFrozen);
  aVerticesFrozen = true;
}

// Return a reference to the internal data
const VertexAccumulator::Colors& VertexAccumulator::getColors() const
{
  return aColors;
}

// Return a reference to the internal data
const VertexAccumulator::Vertices& VertexAccumulator::getVertices() const
{
  return aVertices;
}

// Return a reference to the internal data
//  non-const because of it's computed into the VertexedPrimitiveAccumulators
VertexAccumulator::Normals& VertexAccumulator::getNormals()
{
  return aNormals;
}

// Return the state of the frozen flag; used for GLV_ASSERTs
bool VertexAccumulator::getVerticesFrozen() const
{
  return aVerticesFrozen;
}

// Return the state of the frozen flag; used for GLV_ASSERTs
bool VertexAccumulator::getColorsFrozen() const
{
  return aColorsFrozen;
}

