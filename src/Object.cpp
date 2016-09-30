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

#include <cstring>
#include "Object.h"
#include "assert_glv.h"
#include "glut_utils.h"
#include "limits_glv.h"
#include "Matrix4x4.h"
#include "Parser.h"
#include "PrimitiveAccumulator.h"
#include "string_utils.h"
#include "Vector3D.h"
#include "VertexAccumulator.h"
#include "VertexedPrimitiveAccumulator.h"
#include <iostream>
#include <stdio.h>

// Should be put in a header file
std::string extractCommandWord(const std::string&      pLine,
                               std::string::size_type& pEndWord);


Object::Object()
  : aBoundingBox                          (),
    aCommands                             (),
    aFrozen                               (false),
    aGLDisplayListBoundingBox             (0),
    aGLDisplayListFast                    (0),
    aGLDisplayListFull                    (0),
    aName                                 (),
    aNewPrimitiveAccumulatorNeeded        (true),
    aNewVertexAccumulatorNeeded           (true),
    aNewVertexedPrimitiveAccumulatorNeeded(true),
    aPrimitiveAccumulators                (),
    aRawMode                              (rawMode_not_in_raw_section),
    aRawModeArrowTipNbPolygons            (-1),
    aRawModeArrowTipProportion            (-1.0f),
    aSubObjects                           (),
    aVertexAccumulators                   (),
    aVertexedPrimitiveAccumulators        ()
{}

Object::~Object()
{
  if (aGLDisplayListFull != 0) {
    glDeleteLists(aGLDisplayListFull, 1);
  }
  if (aGLDisplayListBoundingBox != 0) {
    glDeleteLists(aGLDisplayListBoundingBox, 1);
  }
  if (aGLDisplayListFast != 0) {
    glDeleteLists(aGLDisplayListFast, 1);
  }

  {
    SubObjects::iterator       lIter    = aSubObjects.begin();
    const SubObjects::iterator lIterEnd = aSubObjects.end  ();
    while (lIter != lIterEnd) {
      delete *lIter;
      ++lIter;
    }
  }

  {
    PrimitiveAccumulators::iterator       lIter    = aPrimitiveAccumulators.begin();
    const PrimitiveAccumulators::iterator lIterEnd = aPrimitiveAccumulators.end  ();
    while (lIter != lIterEnd) {
      delete *lIter;
      ++lIter;
    }
  }

  {
    VertexedPrimitiveAccumulators::iterator       lIter    = aVertexedPrimitiveAccumulators.begin();
    const VertexedPrimitiveAccumulators::iterator lIterEnd = aVertexedPrimitiveAccumulators.end  ();
    while (lIter != lIterEnd) {
      delete *lIter;
      ++lIter;
    }
  }

  {
    VertexAccumulators::iterator       lIter    = aVertexAccumulators.begin();
    const VertexAccumulators::iterator lIterEnd = aVertexAccumulators.end  ();
    while (lIter != lIterEnd) {
      delete *lIter;
      ++lIter;
    }
  }

}

// Delete the display lists of the current Object and all its SubObjects.
// Used primarily to force the reconstruction of the display lists when
// different OpenGL contexts can't share them
void Object::deleteDisplayLists()
{

  if (aGLDisplayListFull != 0) {
    glDeleteLists(aGLDisplayListFull, 1);
    aGLDisplayListFull = 0;
  }
  if (aGLDisplayListBoundingBox != 0) {
    glDeleteLists(aGLDisplayListBoundingBox, 1);
    aGLDisplayListBoundingBox = 0;
  }
  if (aGLDisplayListFast != 0) {
    glDeleteLists(aGLDisplayListFast, 1);
    aGLDisplayListFast = 0;
  }

  SubObjects::iterator       lIter    = aSubObjects.begin();
  const SubObjects::iterator lIterEnd = aSubObjects.end  ();

  while (lIter != lIterEnd) {

    // The sub-Object might have been deleted
    Object* lSubObject = *lIter;

    if (lSubObject != 0) {
      lSubObject->deleteDisplayLists();
    }
    ++lIter;
  }
}

// Dump in ASCII the caracteristics of the Object and its parts
void Object::dumpCharacteristics(std::ostream&       pOstream,
                                 const std::string&  pIndentation,
                                 const Matrix4x4&    pTransformation)
{
  pOstream << pIndentation << "Object " << std::endl;
  std::string lIndentation = pIndentation + "  ";

  // Update the bounding box if necessary by calling getBoundingBox
  // instead of accessing directly aBoundingBox
  getBoundingBox();
  aBoundingBox.dumpCharacteristics(pOstream, lIndentation, pTransformation);

  pOstream << lIndentation << "Name               = " << aName << std::endl;

  if (aFrozen) {
    pOstream << lIndentation << "Frozen             = true" << std::endl;
  }
  else {
    pOstream << lIndentation << "Frozen             = false" << std::endl;
  }

#ifdef GLV_DUMP_MEMORY_USAGE
  {
    typedef  std::vector<std::string>::size_type  SizeType;

    pOstream << lIndentation << "Memory used by the Object = " << sizeof(*this) << std::endl;
    pOstream << lIndentation << "Memory used by aName      = "
             << aName.size() << "/"
             << aName.capacity() << std::endl;

    Commands::const_iterator       lIterCommands     = aCommands.begin();
    const Commands::const_iterator lIterCommandsEnd  = aCommands.end  ();
    SizeType                       lCommandsSize     = sizeof(std::string)*aCommands.size    ();
    SizeType                       lCommandsCapacity = sizeof(std::string)*aCommands.capacity();

    while (lIterCommands != lIterCommandsEnd) {
      lCommandsSize     += lIterCommands->size();
      lCommandsCapacity += lIterCommands->capacity();
      ++lIterCommands;
    }

    pOstream << lIndentation << "Memory used by aCommands  = "
             << lCommandsSize << "/"
             << lCommandsCapacity << std::endl;
  }
#endif // #ifdef GLV_DUMP_MEMORY_USAGE

  pOstream << lIndentation << "Number of commands = " << aCommands.size() << std::endl;

  if (aCommands.size() > 0) {
    pOstream << lIndentation << "Commands (**): " << std::endl;
  }

  lIndentation += "  ";

  // Output every command and the dumpCharacteristics() on
  // commands that support it
  Commands::const_iterator       lIterCommands    = aCommands.begin();
  const Commands::const_iterator lIterCommandsEnd = aCommands.end  ();

  // Take a local copy of pTransformation in order to update
  // it when we encounter a gltranslate or glscale command
  Matrix4x4 lTransformation = pTransformation;

  while (lIterCommands != lIterCommandsEnd) {

    std::string::size_type lEndWord = std::string::npos;
    std::string            lCommand = extractCommandWord(*lIterCommands, lEndWord);

    std::string lParameters;
    if(lEndWord != std::string::npos) {
      lParameters = trimString(lIterCommands->substr(lEndWord+1), " \t\n");
    }

    GLV_ASSERT(!lCommand.empty());

    pOstream << lIndentation << "** " << *lIterCommands << std::endl;

    if (lCommand == "execute_primitive_accumulator_id") {
      GLV_ASSERT(countWords(lParameters) == 1);
      int lPrimitiveAccumulatorId = atoi(lParameters.c_str());
      GLV_ASSERT(lPrimitiveAccumulatorId >= 0);
      GLV_ASSERT(lPrimitiveAccumulatorId <  static_cast<int>(aPrimitiveAccumulators.size()));
      GLV_ASSERT(aPrimitiveAccumulators[lPrimitiveAccumulatorId] != 0);

      aPrimitiveAccumulators[lPrimitiveAccumulatorId]->dumpCharacteristics(pOstream,
                                                                           lIndentation + "  ",
                                                                           lTransformation);

    }
    else if (lCommand == "execute_vertex_primitive_accumulator_id") {
      GLV_ASSERT(countWords(lParameters) == 1);
      int lVertexedPrimitiveAccumulatorId = atoi(lParameters.c_str());
      GLV_ASSERT(lVertexedPrimitiveAccumulatorId >= 0);
      GLV_ASSERT(lVertexedPrimitiveAccumulatorId <  static_cast<int>(aVertexedPrimitiveAccumulators.size()));
      GLV_ASSERT(aVertexedPrimitiveAccumulators[lVertexedPrimitiveAccumulatorId] != 0);

      aVertexedPrimitiveAccumulators[lVertexedPrimitiveAccumulatorId]->dumpCharacteristics(pOstream,
                                                                                           lIndentation + "  ",
                                                                                           lTransformation);

    }
    else if (lCommand == "execute_subobjects_id") {
      GLV_ASSERT(countWords(lParameters) == 1);
      int lSubObjectId = atoi(lParameters.c_str());
      GLV_ASSERT(lSubObjectId >= 0);
      GLV_ASSERT(lSubObjectId <  static_cast<int>(aSubObjects.size()));

      // The sub-Object might have been deleted
      if (aSubObjects[lSubObjectId] != 0) {
        aSubObjects[lSubObjectId]->dumpCharacteristics(pOstream,
                                                       lIndentation + "  ",
                                                       lTransformation);
      }
      else {
        pOstream << lIndentation << "  Object deleted" << std::endl;
      }
    }
    // DIRECT OPENGL CALLS
    else if(lCommand == "gltranslate") {
      float tx, ty, tz;
      sscanf(lParameters.c_str(), "%f %f %f", &tx, &ty, &tz);

      // Add the translation to the transformation matrix lTM
      lTransformation.translate(Vector3D(tx, ty, tz));
    }
    else if(lCommand == "glscale") {
      float sx, sy, sz;
      sscanf(lParameters.c_str(), "%f %f %f", &sx, &sy, &sz);

      // Add the scaling to the transformation matrix lTM
      lTransformation.scale(Vector3D(sx, sy, sz));

    }

    ++lIterCommands;
  }
}

// Add a command to the object and parse the parameters
// If pError.empty() == 0 on exit, then everything was fine
void Object::addCommand(const std::string& pCommand,
                        const std::string& pParameters,
                        Parser&            pCurrentParser,
                        std::string&       pError)
{
  GLV_ASSERT(pError.empty());
  GLV_ASSERT(!pCommand.empty());
  GLV_ASSERT(pCommand   .find_first_not_of(" \t\n") == 0);
  GLV_ASSERT(pCommand   .find_last_not_of (" \t\n") == pCommand.size()-1);
  GLV_ASSERT(pParameters.find_first_not_of(" \t\n") == 0 || pParameters.empty());
  GLV_ASSERT(pParameters.find_last_not_of (" \t\n") == pParameters.size()-1 || pParameters.empty());


  if (aRawMode == rawMode_arrow) {

    GLV_ASSERT(aRawModeArrowTipProportion  > 0.0f);
    GLV_ASSERT(aRawModeArrowTipProportion <= 1.0f);
    GLV_ASSERT(aRawModeArrowTipNbPolygons >= 1);

    float x1,y1,z1,x2,y2,z2;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_arrow_item" ||
            6 != countWords(pParameters) ||
            6 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f", &x1, &y1, &z1, &x2, &y2, &z2)) {
      addRawSectionError("raw_arrow", "x1 y1 z1 x2 y2 z2 ", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addArrow(Vector3D(x1,y1,z1),
                                     Vector3D(x2,y2,z2),
                                     aRawModeArrowTipProportion,
                                     aRawModeArrowTipNbPolygons);
    }

  }
  else if (aRawMode == rawMode_arrow_colored) {

    GLV_ASSERT(aRawModeArrowTipProportion  > 0.0f);
    GLV_ASSERT(aRawModeArrowTipProportion <= 1.0f);
    GLV_ASSERT(aRawModeArrowTipNbPolygons >= 1);

    float x1,y1,z1,x2,y2,z2;
    float r1,g1,b1,r2,g2,b2;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_arrow_colored_item" ||
            12 != countWords(pParameters) ||
            12 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f",
                         &x1, &y1, &z1, &r1, &g1, &b1,
                         &x2, &y2, &z2, &r2, &g2, &b2)) {
      addRawSectionError("raw_arrow_colored", "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addArrowColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                            Vector3D(x2,y2,z2), Vector3D(r2,g2,b2),
                                            aRawModeArrowTipProportion,
                                            aRawModeArrowTipNbPolygons);
    }

  }
  else if (aRawMode == rawMode_point) {

    float x,y,z;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if (pCommand != "raw_point_item" ||
             3 != countWords(pParameters) ||
             3 != sscanf(pParameters.c_str(), "%f %f %f", &x, &y, &z)) {
      addRawSectionError("raw_point", "x y z", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addPoint(Vector3D(x,y,z));
    }

  }
  else if (aRawMode == rawMode_point_colored) {

    float x,y,z,r,g,b;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_point_colored_item" ||
            6 != countWords(pParameters) ||
            6 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b)) {
      addRawSectionError("raw_point_colored", "x y z r g b", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addPointColored(Vector3D(x,y,z), Vector3D(r,g,b));
    }

  }
  else if (aRawMode == rawMode_point_v) {

    int pt;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_point_v_item" ||
            1 != countWords(pParameters) ||
            1 != sscanf(pParameters.c_str(), "%d", &pt)) {
      addRawSectionError("raw_point_v", "pt", pCurrentParser, pError);
    }
    else {
      VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
      if (!lVertexedPrimitiveAccumulator.addPoint(pt)) {
        addError("Parameter out of range in raw_point_v", pCurrentParser, pError);
      }
    }

  }
  else if (aRawMode == rawMode_line) {

    float x1,y1,z1,x2,y2,z2;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_line_item" ||
            6 != countWords(pParameters) ||
            6 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f", &x1, &y1, &z1, &x2, &y2, &z2)) {
      addRawSectionError("raw_line", "x1 y1 z1 x2 y2 z2", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addLine(Vector3D(x1,y1,z1),
                                    Vector3D(x2,y2,z2));
    }

  }
  else if (aRawMode == rawMode_line_colored) {

    float x1,y1,z1,x2,y2,z2;
    float r1,g1,b1,r2,g2,b2;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_line_colored_item" ||
            12 != countWords(pParameters) ||
            12 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f",
                         &x1, &y1, &z1, &r1, &g1, &b1,
                         &x2, &y2, &z2, &r2, &g2, &b2)) {
      addRawSectionError("raw_line_colored", "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addLineColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                           Vector3D(x2,y2,z2), Vector3D(r2,g2,b2));
    }

  }
  else if (aRawMode == rawMode_line_v) {

    int pt0, pt1;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_line_v_item" ||
            2 != countWords(pParameters) ||
            2 != sscanf(pParameters.c_str(), "%d %d", &pt0, &pt1)) {
      addRawSectionError("raw_line_v", "pt0 pt1", pCurrentParser, pError);
    }
    else {
      VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
      if (!lVertexedPrimitiveAccumulator.addLine(pt0, pt1)) {
        addError("Parameter out of range in raw_line_v", pCurrentParser, pError);
      }
    }

  }
  else if (aRawMode == rawMode_triangle) {

    float x1,y1,z1,x2,y2,z2,x3,y3,z3;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_triangle_item" ||
            9 != countWords(pParameters) ||
            9 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f",
                        &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3)) {
      addRawSectionError("raw_triangle", "x1 y1 z1 x2 y2 z2 x3 y3 z3", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addTriangle(Vector3D(x1,y1,z1),
                                        Vector3D(x2,y2,z2),
                                        Vector3D(x3,y3,z3));
    }

  }
  else if (aRawMode == rawMode_triangle_colored) {

    float x1,y1,z1,x2,y2,z2,x3,y3,z3;
    float r1,g1,b1,r2,g2,b2,r3,g3,b3;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_triangle_colored_item" ||
            18 != countWords(pParameters) ||
            18 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                         &x1, &y1, &z1, &r1, &g1, &b1,
                         &x2, &y2, &z2, &r2, &g2, &b2,
                         &x3, &y3, &z3, &r3, &g3, &b3)) {
      addRawSectionError("raw_triangle_colored", "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2 x3 y3 z3 r3 g3 b3", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addTriangleColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                               Vector3D(x2,y2,z2), Vector3D(r2,g2,b2),
                                               Vector3D(x3,y3,z3), Vector3D(r3,g3,b3));
    }

  }
  else if (aRawMode == rawMode_triangle_v) {

    int pt0, pt1, pt2;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_triangle_v_item" ||
            3 != countWords(pParameters) ||
            3 != sscanf(pParameters.c_str(), "%d %d %d", &pt0, &pt1, &pt2)) {
      addRawSectionError("raw_triangle_v", "pt0 pt1 pt2", pCurrentParser, pError);
    }
    else {
      VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
      if (!lVertexedPrimitiveAccumulator.addTriangle(pt0, pt1, pt2)) {
        addError("Parameter out of range in raw_triangle_v", pCurrentParser, pError);
      }
    }

  }
  else if (aRawMode == rawMode_quad) {

    float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_quad_item" ||
            12 != countWords(pParameters) ||
            12 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f",
                         &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4)) {
      addRawSectionError("raw_quad", "x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addQuad(Vector3D(x1,y1,z1),
                                    Vector3D(x2,y2,z2),
                                    Vector3D(x3,y3,z3),
                                    Vector3D(x4,y4,z4));
    }

  }
  else if (aRawMode == rawMode_quad_colored) {

    float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
    float r1,g1,b1,r2,g2,b2,r3,g3,b3,r4,g4,b4;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_quad_colored_item" ||
            24 != countWords(pParameters) ||
            24 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                         &x1, &y1, &z1, &r1, &g1, &b1,
                         &x2, &y2, &z2, &r2, &g2, &b2,
                         &x3, &y3, &z3, &r3, &g3, &b3,
                         &x4, &y4, &z4, &r4, &g4, &b4)) {
      addRawSectionError("raw_quad_colored", "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2 x3 y3 z3 r3 g3 b3 x4 y4 z4 r4 g4 b4", pCurrentParser, pError);
    }
    else {
      PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
      lPrimitiveAccumulator.addQuadColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                           Vector3D(x2,y2,z2), Vector3D(r2,g2,b2),
                                           Vector3D(x3,y3,z3), Vector3D(r3,g3,b3),
                                           Vector3D(x4,y4,z4), Vector3D(r4,g4,b4));
    }

  }
  else if (aRawMode == rawMode_quad_v) {

    int pt0, pt1, pt2, pt3;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
      }
    }
    else if(pCommand != "raw_quad_v_item" ||
            4 != countWords(pParameters) ||
            4 != sscanf(pParameters.c_str(), "%d %d %d %d", &pt0, &pt1, &pt2, &pt3)) {
      addRawSectionError("raw_quad_v", "pt0, pt1, pt2, pt3", pCurrentParser, pError);
    }
    else {
      VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
      if (!lVertexedPrimitiveAccumulator.addQuad(pt0, pt1, pt2, pt3)) {
        addError("Parameter out of range in raw_quad_v", pCurrentParser, pError);
      }
    }

  }
  else if (aRawMode == rawMode_vertex) {

    float x,y,z;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
        GLV_ASSERT(!aVertexAccumulators.empty());
        GLV_ASSERT(aVertexAccumulators.back() != 0);
        aVertexAccumulators.back()->freezeVertices();
      }
    }
    else if(pCommand != "raw_vertex_item" ||
            3 != countWords(pParameters) ||
            3 != sscanf(pParameters.c_str(), "%f %f %f", &x, &y, &z)) {
      addRawSectionError("raw_vertex", "x y z", pCurrentParser, pError);
    }
    else {
      VertexAccumulator& lVertexAccumulator = getCurrentVertexAccumulator();
      lVertexAccumulator.addVertex(Vector3D(x,y,z));
    }

  }
  else if (aRawMode == rawMode_color_v) {

    float r,g,b;
    if (pCommand == "raw_end") {
      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aRawMode = rawMode_not_in_raw_section;
        GLV_ASSERT(!aVertexAccumulators.empty());
        GLV_ASSERT(aVertexAccumulators.back() != 0);
        if (!aVertexAccumulators.back()->freezeColors()) {
          addError("Incompatible raw_color_v section size", pCurrentParser, pError);
        }
      }
    }
    else if(pCommand != "raw_color_v_item" ||
            3 != countWords(pParameters) ||
            3 != sscanf(pParameters.c_str(), "%f %f %f", &r, &g, &b)) {
      addRawSectionError("raw_color_v", "r g b", pCurrentParser, pError);
    }
    else {
      VertexAccumulator& lVertexAccumulator = getCurrentVertexAccumulator();
      lVertexAccumulator.addColor(Vector3D(r,g,b));
    }

  }
  else {
    GLV_ASSERT(aRawMode == rawMode_not_in_raw_section);


    // SIMPLE PRIMITIVES
    if(pCommand == "arrow") {

      float x1,y1,z1,x2,y2,z2;
      float tipprop;
      int   tippoly;
      if(8 != countWords(pParameters) ||
         8 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %d",
                     &x1, &y1, &z1, &x2, &y2, &z2, &tipprop, &tippoly)) {
        addSyntaxError(pCommand, "x1 y1 z1 x2 y2 z2 tipprop tippoly", pCurrentParser, pError);
      }
      else {
        if (tipprop <= 0.0f || tipprop > 1.0f || tippoly < 1) {
          addError("Parameter out of range in arrow", pCurrentParser, pError);
        }
        else {
          PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
          lPrimitiveAccumulator.addArrow(Vector3D(x1,y1,z1),
                                         Vector3D(x2,y2,z2),
                                         tipprop, tippoly);
        }
      }

    }
    else if(pCommand == "arrow_colored") {

      float x1,y1,z1,x2,y2,z2;
      float r1,g1,b1,r2,g2,b2;
      float tipprop;
      int   tippoly;
      if(14 != countWords(pParameters) ||
         14 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f %d",
                      &x1, &y1, &z1, &r1, &g1, &b1,
                      &x2, &y2, &z2, &r2, &g2, &b2,
                      &tipprop, &tippoly)) {
        addSyntaxError(pCommand, "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2 tipprop tippoly", pCurrentParser, pError);
      }
      else {
        if (tipprop <= 0.0f || tipprop > 1.0f || tippoly < 1) {
          addError("Parameter out of range in arrow_colored", pCurrentParser, pError);
        }
        else {
          PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
          lPrimitiveAccumulator.addArrowColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                                Vector3D(x2,y2,z2), Vector3D(r2,g2,b2),
                                                tipprop, tippoly);
        }
      }

    }
    else if(pCommand == "point") {

      float x,y,z;
      if(3 != countWords(pParameters) ||
         3 != sscanf(pParameters.c_str(), "%f %f %f", &x, &y, &z)) {
        addSyntaxError(pCommand, "x y z", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addPoint(Vector3D(x,y,z));
      }

    }
    else if(pCommand == "point_colored") {

      float x1,y1,z1,r1,g1,b1;
      if(6 != countWords(pParameters) ||
         6 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f", &x1, &y1, &z1, &r1, &g1, &b1)) {
        addSyntaxError(pCommand, "x1 y1 z1 r1 g1 b1", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addPointColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1));
      }

    }
    else if(pCommand == "line") {

      float x1,y1,z1,x2,y2,z2;
      if(6 != countWords(pParameters) ||
         6 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f", &x1, &y1, &z1, &x2, &y2, &z2)) {
        addSyntaxError(pCommand, "x1 y1 z1 x2 y2 z2", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addLine(Vector3D(x1,y1,z1),
                                      Vector3D(x2,y2,z2));
      }

    }
    else if(pCommand == "line_colored") {

      float x1,y1,z1,x2,y2,z2;
      float r1,g1,b1,r2,g2,b2;
      if(12 != countWords(pParameters) ||
         12 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f",
                      &x1, &y1, &z1, &r1, &g1, &b1,
                      &x2, &y2, &z2, &r2, &g2, &b2)) {
        addSyntaxError(pCommand, "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addLineColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                             Vector3D(x2,y2,z2), Vector3D(r2,g2,b2));
      }

    }
    else if(pCommand == "triangle") {

      float x1,y1,z1,x2,y2,z2,x3,y3,z3;
      if(9 != countWords(pParameters) ||
         9 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f",
                     &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3)) {
        addSyntaxError(pCommand, "x1 y1 z1 x2 y2 z2 x3 y3 z3", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addTriangle(Vector3D(x1,y1,z1),
                                          Vector3D(x2,y2,z2),
                                          Vector3D(x3,y3,z3));
      }

    }
    else if(pCommand == "triangle_colored") {

      float x1,y1,z1,x2,y2,z2,x3,y3,z3;
      float r1,g1,b1,r2,g2,b2,r3,g3,b3;
      if(18 != countWords(pParameters) ||
         18 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                      &x1, &y1, &z1, &r1, &g1, &b1,
                      &x2, &y2, &z2, &r2, &g2, &b2,
                      &x3, &y3, &z3, &r3, &g3, &b3)) {
        addSyntaxError(pCommand, "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2 x3 y3 z3 r3 g3 b3", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addTriangleColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                                 Vector3D(x2,y2,z2), Vector3D(r2,g2,b2),
                                                 Vector3D(x3,y3,z3), Vector3D(r3,g3,b3));
      }

    }
    else if(pCommand == "quad") {

      float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
      if(12 != countWords(pParameters) ||
         12 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f",
                      &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4)) {
        addSyntaxError(pCommand, "x1 y1 z1 x2 y2 z2 x3 y3 z3 x4 y4 z4", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addQuad(Vector3D(x1,y1,z1),
                                      Vector3D(x2,y2,z2),
                                      Vector3D(x3,y3,z3),
                                      Vector3D(x4,y4,z4));
      }

    }
    else if(pCommand == "quad_colored") {

      float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
      float r1,g1,b1,r2,g2,b2,r3,g3,b3,r4,g4,b4;
      if(24 != countWords(pParameters) ||
         24 != sscanf(pParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                      &x1, &y1, &z1, &r1, &g1, &b1,
                      &x2, &y2, &z2, &r2, &g2, &b2,
                      &x3, &y3, &z3, &r3, &g3, &b3,
                      &x4, &y4, &z4, &r4, &g4, &b4)) {
        addSyntaxError(pCommand, "x1 y1 z1 r1 g1 b1 x2 y2 z2 r2 g2 b2 x3 y3 z3 r3 g3 b3 x4 y4 z4 r4 g4 b4", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addQuadColored(Vector3D(x1,y1,z1), Vector3D(r1,g1,b1),
                                             Vector3D(x2,y2,z2), Vector3D(r2,g2,b2),
                                             Vector3D(x3,y3,z3), Vector3D(r3,g3,b3),
                                             Vector3D(x4,y4,z4), Vector3D(r4,g4,b4));
      }

    }
    // VERTEX MODE
    else if(pCommand == "point_v") {

      int pt;
      if (aVertexAccumulators.empty()) {
        addError("Encountered a point_v before a raw_vertex", pCurrentParser, pError);
      }
      else if (1 != countWords(pParameters) ||
               1 != sscanf(pParameters.c_str(), "%d", &pt)) {
        addSyntaxError(pCommand, "pt", pCurrentParser, pError);
      }
      else {
        VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
        if (!lVertexedPrimitiveAccumulator.addPoint(pt)) {
          addError("Parameter out of range in point_v", pCurrentParser, pError);
        }
      }

    }
    else if(pCommand == "line_v") {

      int pt0, pt1;
      if (aVertexAccumulators.empty()) {
        addError("Encountered a line_v before a raw_vertex", pCurrentParser, pError);
      }
      else if (2 != countWords(pParameters) ||
               2 != sscanf(pParameters.c_str(), "%d %d", &pt0, &pt1)) {
        addSyntaxError(pCommand, "pt0 pt1", pCurrentParser, pError);
      }
      else {
        VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
        if (!lVertexedPrimitiveAccumulator.addLine(pt0, pt1)) {
          addError("Parameter out of range in line_v", pCurrentParser, pError);
        }
      }

    }
    else if(pCommand == "triangle_v") {

      int pt0, pt1, pt2;
      if (aVertexAccumulators.empty()) {
        addError("Encountered a triangle_v before a raw_vertex", pCurrentParser, pError);
      }
      else if (3 != countWords(pParameters) ||
               3 != sscanf(pParameters.c_str(), "%d %d %d", &pt0, &pt1, &pt2)) {
        addSyntaxError(pCommand, "pt0 pt1 pt2", pCurrentParser, pError);
      }
      else {
        VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
        if (!lVertexedPrimitiveAccumulator.addTriangle(pt0, pt1, pt2)) {
          addError("Parameter out of range in triangle_v", pCurrentParser, pError);
        }
      }

    }
    else if(pCommand == "quad_v") {

      int pt0, pt1, pt2, pt3;
      if (aVertexAccumulators.empty()) {
        addError("Encountered a quad_v before a raw_vertex", pCurrentParser, pError);
      }
      else if(4 != countWords(pParameters) ||
              4 != sscanf(pParameters.c_str(), "%d %d %d %d", &pt0, &pt1, &pt2, &pt3)) {
        addSyntaxError(pCommand, "pt0 pt1 pt2 pt3", pCurrentParser, pError);
      }
      else {
        VertexedPrimitiveAccumulator& lVertexedPrimitiveAccumulator = getCurrentVertexedPrimitiveAccumulator();
        if (!lVertexedPrimitiveAccumulator.addQuad(pt0, pt1, pt2, pt3)) {
          addError("Parameter out of range in quad_v", pCurrentParser, pError);
        }
      }

    }
    // RAW COMMANDS
    else if(pCommand == "raw_end") {
      addError("No raw section opened", pCurrentParser, pError);

    }
    else if(pCommand == "raw_arrow") {

      if (2 != countWords(pParameters) ||
          2 != sscanf(pParameters.c_str(), "%f %d",
                      &aRawModeArrowTipProportion,
                      &aRawModeArrowTipNbPolygons)) {
        addSyntaxError(pCommand, "tipprop tippoly", pCurrentParser, pError);
      }
      else {
        if (aRawModeArrowTipProportion <= 0.0f ||
            aRawModeArrowTipProportion  > 1.0f  ||
            aRawModeArrowTipNbPolygons  < 1)
        {
          addError("Parameter out of range in raw_arrow", pCurrentParser, pError);
          aRawModeArrowTipProportion  = -1.0f;
          aRawModeArrowTipNbPolygons  = -1;
        }
        else {
          if (aRawMode != rawMode_not_in_raw_section) {
            addError("Nested raw sections are not supported", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_arrow;
          }
        }
      }

    }
    else if(pCommand == "raw_arrow_colored") {

      if (2 != countWords(pParameters) ||
          2 != sscanf(pParameters.c_str(), "%f %d",
                      &aRawModeArrowTipProportion,
                      &aRawModeArrowTipNbPolygons)) {
        addSyntaxError(pCommand, "tipprop tippoly", pCurrentParser, pError);
      }
      else {
        if (aRawModeArrowTipProportion <= 0.0f ||
            aRawModeArrowTipProportion  > 1.0f  ||
            aRawModeArrowTipNbPolygons  < 1)
        {
          addError("Parameter out of range in raw_arrow_colored", pCurrentParser, pError);
          aRawModeArrowTipProportion  = -1.0f;
          aRawModeArrowTipNbPolygons  = -1;
        }
        else {
          if (aRawMode != rawMode_not_in_raw_section) {
            addError("Nested raw sections are not supported", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_arrow_colored;
          }
        }
      }

    }
    else if(pCommand == "raw_point") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_point;
        }
      }

    }
    else if(pCommand == "raw_point_colored") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_point_colored;
        }
      }

    }
    else if(pCommand == "raw_point_v") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          if (aVertexAccumulators.empty()) {
            addError("Encountered a raw_point_v before a raw_vertex", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_point_v;
          }
        }
      }

    }
    else if(pCommand == "raw_line") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_line;
        }
      }

    }
    else if(pCommand == "raw_line_colored") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_line_colored;
        }
      }

    }
    else if(pCommand == "raw_line_v") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          if (aVertexAccumulators.empty()) {
            addError("Encountered a raw_line_v before a raw_vertex", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_line_v;
          }
        }
      }

    }
    else if(pCommand == "raw_triangle") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_triangle;
        }
      }

    }
    else if(pCommand == "raw_triangle_colored") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_triangle_colored;
        }
      }

    }
    else if(pCommand == "raw_triangle_v") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          if (aVertexAccumulators.empty()) {
            addError("Encountered a raw_triangle_v before a raw_vertex", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_triangle_v;
          }
        }
      }

    }
    else if(pCommand == "raw_quad") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_quad;
        }
      }
    }
    else if(pCommand == "raw_quad_colored") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode = rawMode_quad_colored;
        }
      }
    }
    else if(pCommand == "raw_quad_v") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          if (aVertexAccumulators.empty()) {
            addError("Encountered a raw_quad_v before a raw_vertex", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_quad_v;
          }
        }
      }
    }
    else if(pCommand == "raw_vertex") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          aRawMode                               = rawMode_vertex;
          aNewVertexedPrimitiveAccumulatorNeeded = true;
        }
      }

    }
    else if(pCommand == "raw_color_v") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        if (aRawMode != rawMode_not_in_raw_section) {
          addError("Nested raw sections are not supported", pCurrentParser, pError);
        }
        else {
          if (aVertexAccumulators.empty()) {
            addError("Encountered a raw_color_v before a raw_vertex", pCurrentParser, pError);
          }
          else {
            aRawMode = rawMode_color_v;
          }
        }
      }

    }
    // OBJECT COMMANDS
    else if (pCommand == "object_begin") {

      Object* lNewObject = new Object();
      aSubObjects.push_back(lNewObject);

      // The name might be empty, but it doesn't matter
      lNewObject->aName = pParameters;

      char lObjCommand[64];
      sprintf(lObjCommand,"execute_subobjects_id %d", aSubObjects.size()-1);
      aCommands.push_back(lObjCommand);

      pCurrentParser.pushObject(lNewObject);

    }
    else if (pCommand == "execute_object") {

      if (countWords(pParameters) != 1) {
        addSyntaxError(pCommand, "OBJECTNAME", pCurrentParser, pError);
      }
      else {

        // Find the object with the given id
        const int lNbSubObjects = static_cast<int>(aSubObjects.size());
        bool      lFound        = false;

        for(int i=0; i<lNbSubObjects; ++i) {

          Object* lSubObjectPtr = aSubObjects[i];

          if (lSubObjectPtr != 0) {
            if(lSubObjectPtr->aName == pParameters) {
              char lObjCommand[64];
              sprintf(lObjCommand,"execute_subobjects_id %d", i);
              aCommands.push_back(lObjCommand);
              lFound = true;
            }
          }
        }

        if(lFound) {

          // We have to recompute the display lists and
          // the BoundingBox. So we force it to happen.
          const bool lFrozen = aFrozen;
          if (aGLDisplayListFull != 0) {
            glDeleteLists(aGLDisplayListFull, 1);
          }
          if (aGLDisplayListBoundingBox != 0) {
            glDeleteLists(aGLDisplayListBoundingBox, 1);
          }
          if (aGLDisplayListFast != 0) {
            glDeleteLists(aGLDisplayListFast, 1);
          }
          aFrozen = false;
          getBoundingBox();
          aFrozen = lFrozen;
        }
        else {
          addError("execute_object: Can't find the named object in the current object sub-objects",
                   pCurrentParser, pError);
        }
      }
    }
    else if(pCommand == "object_end") {

      if (!pParameters.empty()) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }

      pCurrentParser.popObject(this);

      // Mark the Object frozen and compute the
      // BoundingBox for the last time
      getBoundingBox();
      aFrozen = true;
    }
    else if(pCommand == "delete_object") {

      if (countWords(pParameters) != 1) {
        addSyntaxError(pCommand, "OBJECTNAME", pCurrentParser, pError);
      }
      else {

        // Find the object with the given id
        SubObjects::iterator       lIterSubObjects    = aSubObjects.begin();
        const SubObjects::iterator lIterSubObjectsEnd = aSubObjects.end  ();
        bool                       lFound             = false;

        while (lIterSubObjects != lIterSubObjectsEnd) {
          Object* lSubObjectPtr = *lIterSubObjects;

          if (lSubObjectPtr != 0) {
            if(lSubObjectPtr->aName == pParameters) {
              delete lSubObjectPtr;
              *lIterSubObjects = 0;
              lFound = true;
            }
          }
          ++lIterSubObjects;
        }

        if(lFound) {

          // We have to recompute the display lists and
          // the BoundingBox. So we force it to happen.
          const bool lFrozen = aFrozen;
          if (aGLDisplayListFull != 0) {
            glDeleteLists(aGLDisplayListFull, 1);
          }
          if (aGLDisplayListBoundingBox != 0) {
            glDeleteLists(aGLDisplayListBoundingBox, 1);
          }
          if (aGLDisplayListFast != 0) {
            glDeleteLists(aGLDisplayListFast, 1);
          }
          aFrozen = false;
          getBoundingBox();
          aFrozen = lFrozen;
        }
        else {
          addError("delete_object: Can't find the named object in the current object sub-objects",
                   pCurrentParser, pError);
        }
      }
    }
    // DIRECT OPENGL CALLS
    // DRAWING PARAMETERS
    else if(pCommand == "glcolor") {

      float r,g,b;
      if(3 != countWords(pParameters) ||
         3 != sscanf(pParameters.c_str(), "%f %f %f", &r, &g, &b)) {
        addSyntaxError(pCommand, "r g b", pCurrentParser, pError);
      }
      else if (r < 0.0 || r > 1.0 ||
               g < 0.0 || g > 1.0 ||
               b < 0.0 || b > 1.0) {
        addError("Colors out of range in glcolor", pCurrentParser, pError);
      }
      else {
        aCommands.push_back(pCommand + " " + pParameters);
        aNewPrimitiveAccumulatorNeeded         = true;
        aNewVertexedPrimitiveAccumulatorNeeded = true;
      }

    }
    else if(pCommand == "glpushmatrix"                 ||
            pCommand == "glpopmatrix"                  ||
            pCommand == "glbegin_triangles"            ||
            pCommand == "glbegin_lines"                ||
            pCommand == "glbegin_points"               ||
            pCommand == "glend"                        ||
            pCommand == "draw_single_sided"            ||
            pCommand == "draw_double_sided"            ||
            pCommand == "glenable_polygonoffset_fill"  ||
            pCommand == "gldisable_polygonoffset_fill" ||
            pCommand == "draw_facetboundary_disable"     ) {
      if(0 != countWords(pParameters)) {
        addSyntaxError(pCommand, "", pCurrentParser, pError);
      }
      else {
        aCommands.push_back(pCommand + " " + pParameters);
        aNewPrimitiveAccumulatorNeeded         = true;
        aNewVertexedPrimitiveAccumulatorNeeded = true;
      }

    }
    else if(pCommand == "glvertex"    ||
            pCommand == "gltranslate" ||
            pCommand == "glscale"     ||
            pCommand == "draw_facetboundary_enable") {

      float x,y,z;
      if(3 != countWords(pParameters) ||
         3 != sscanf(pParameters.c_str(), "%f %f %f", &x, &y, &z)) {
        addSyntaxError(pCommand, "x y z", pCurrentParser, pError);
      }
      else {
        aCommands.push_back(pCommand + " " + pParameters);
        aNewPrimitiveAccumulatorNeeded         = true;
        aNewVertexedPrimitiveAccumulatorNeeded = true;
      }

    }
    else if(pCommand == "glpointsize" ||
            pCommand == "gllinewidth"   ) {

      float lSize;
      if(1 != countWords(pParameters) ||
         1 != sscanf(pParameters.c_str(), "%f", &lSize)) {
        addSyntaxError(pCommand, "size", pCurrentParser, pError);
      }
      else {
        aCommands.push_back(pCommand + " " + pParameters);
        aNewPrimitiveAccumulatorNeeded         = true;
        aNewVertexedPrimitiveAccumulatorNeeded = true;
      }

    }
    // GLUT UTILS
    else if(pCommand == "glutwirecube") {

      float x,y,z,lSize;
      if(4 != countWords(pParameters) ||
         4 != sscanf(pParameters.c_str(), "%f %f %f %f", &x, &y, &z, &lSize)) {
        addSyntaxError(pCommand, "x y z size", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addWireCube(Vector3D(x,y,z), lSize);
      }

    }
    else if(pCommand == "glutsolidcube") {

      float x,y,z,lSize;
      if(4 != countWords(pParameters) ||
         4 != sscanf(pParameters.c_str(), "%f %f %f %f", &x, &y, &z, &lSize)) {
        addSyntaxError(pCommand, "x y z size", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addSolidCube(Vector3D(x,y,z), lSize);
      }

    }
    else if(pCommand == "glutsolidsphere" ) {

      float x,y,z,lSize;
      if(4 != countWords(pParameters) ||
         4 != sscanf(pParameters.c_str(), "%f %f %f %f", &x, &y, &z, &lSize)) {
        addSyntaxError(pCommand, "x y z size", pCurrentParser, pError);
      }
      else {
        PrimitiveAccumulator& lPrimitiveAccumulator = getCurrentPrimitiveAccumulator();
        lPrimitiveAccumulator.addSolidSphere(Vector3D(x,y,z), lSize, 20, 20);
      }

    }
    else if(pCommand == "text") {

      char lParamFont[1024];
      float x,y,z;

      if(4 != sscanf(pParameters.c_str(), "%f %f %f %s", &x, &y, &z, lParamFont)) {
        addSyntaxError(pCommand, "x y z font \"text to display\"", pCurrentParser, pError);
      }
      else {

        std::string            lParametersStr(pParameters);
        std::string::size_type lStart = lParametersStr.find("\"");

        if(lStart == std::string::npos) {
          addSyntaxError(pCommand, "x y z font \"text to display\"", pCurrentParser, pError);
        }
        else {

          lStart += 1;

          std::string::size_type lEnd = lParametersStr.find("\"",lStart);
          if(lEnd == std::string::npos) {
            addSyntaxError(pCommand, "x y z font \"text to display\"", pCurrentParser, pError);
          }
          else {
            aCommands.push_back(pCommand + " " + pParameters);
          }
        }
      }
    }
    // UNKNOWN COMMAND
    else {
      std::string lError = "Unknown command\n   ";
      addError(lError + pCommand, pCurrentParser, pError);
    }
  }
}

const BoundingBox& Object::getBoundingBox()
{

  // If the object is frozen, then the BoundingBox is frozen too
  if (!aFrozen) {

    // We have to use the commands so that we can provide
    // an accurate BoundingBox by taking into account
    // the commands gltranslate and glscale. Glut
    // primitives are also taken into account as well as
    // the text command.
    aBoundingBox = BoundingBox();

    // At first, we set the transformation to the identity
    Matrix4x4 lTM;

    Commands::const_iterator       lIterCommands    = aCommands.begin();
    const Commands::const_iterator lIterCommandsEnd = aCommands.end  ();

    while (lIterCommands != lIterCommandsEnd) {

      std::string::size_type lEndWord = std::string::npos;
      std::string            lCommand = extractCommandWord(*lIterCommands, lEndWord);

      std::string lParameters;
      if(lEndWord != std::string::npos) {
        lParameters = trimString(lIterCommands->substr(lEndWord+1), " \t\n");
      }

      GLV_ASSERT(!lCommand.empty());

      if (lCommand == "execute_primitive_accumulator_id") {
        GLV_ASSERT(countWords(lParameters) == 1);
        int lPrimitiveAccumulatorId = atoi(lParameters.c_str());
        GLV_ASSERT(lPrimitiveAccumulatorId >= 0);
        GLV_ASSERT(lPrimitiveAccumulatorId <  static_cast<int>(aPrimitiveAccumulators.size()));
        GLV_ASSERT(aPrimitiveAccumulators[lPrimitiveAccumulatorId] != 0);

        const BoundingBox lBoundingBox = aPrimitiveAccumulators[lPrimitiveAccumulatorId]->getBoundingBox();

        // Apply the transformation to the Bounding box
        // and add it to the object bounding box
        aBoundingBox += lTM * lBoundingBox;

      }
      else if (lCommand == "execute_vertex_primitive_accumulator_id") {
        GLV_ASSERT(countWords(lParameters) == 1);
        int lVertexedPrimitiveAccumulatorId = atoi(lParameters.c_str());
        GLV_ASSERT(lVertexedPrimitiveAccumulatorId >= 0);
        GLV_ASSERT(lVertexedPrimitiveAccumulatorId <  static_cast<int>(aVertexedPrimitiveAccumulators.size()));
        GLV_ASSERT(aVertexedPrimitiveAccumulators[lVertexedPrimitiveAccumulatorId] != 0);

        const BoundingBox lBoundingBox = aVertexedPrimitiveAccumulators[lVertexedPrimitiveAccumulatorId]->getBoundingBox();

        // Apply the transformation to the Bounding box
        // and add it to the object bounding box
        aBoundingBox += lTM * lBoundingBox;

      }
      else if (lCommand == "execute_subobjects_id") {
        GLV_ASSERT(countWords(lParameters) == 1);
        int lSubObjectId = atoi(lParameters.c_str());
        GLV_ASSERT(lSubObjectId >= 0);
        GLV_ASSERT(lSubObjectId <  static_cast<int>(aSubObjects.size()));

        // The sub-Object might have been deleted
        if (aSubObjects[lSubObjectId] != 0) {
          BoundingBox lBoundingBox = aSubObjects[lSubObjectId]->getBoundingBox();

          // Apply the transformation to the Bounding box
          // and add it to the object bounding box
          aBoundingBox += lTM * lBoundingBox;
        }

      }
      // DIRECT OPENGL CALLS
      else if(lCommand == "gltranslate") {
        float tx, ty, tz;
        sscanf(lParameters.c_str(), "%f %f %f", &tx, &ty, &tz);

        // Add the translation to the transformation matrix lTM
        lTM.translate(Vector3D(tx, ty, tz));

      }
      else if(lCommand == "glscale") {
        float sx, sy, sz;
        sscanf(lParameters.c_str(), "%f %f %f", &sx, &sy, &sz);

        // Add the scaling to the transformation matrix lTM
        lTM.scale(Vector3D(sx, sy, sz));

      }
      else if(lCommand == "text") {

        // Add just the position of the text in the BoundingBox
        float x, y, z;

        sscanf(lParameters.c_str(), "%f %f %f", &x, &y, &z);

        aBoundingBox += lTM * Vector3D(x, y, z);

      }

      ++lIterCommands;
    }

  }

  return aBoundingBox;
}

void Object::render(RenderParameters& pParams)
{
  GLuint& lGLDisplayList = getGLDisplayList(pParams);

  if (lGLDisplayList == 0) {
    if (aFrozen) {
      constructDisplayList(pParams);
    }
  }

  if (lGLDisplayList != 0) {
    glCallList(lGLDisplayList);
  }
  else {
    // The Object is not frozen *or* GL doesn't want
    // to return a valid aGLDisplayList*. So we just
    // execute the commands
    Commands::const_iterator       lIterCommands    = aCommands.begin();
    const Commands::const_iterator lIterCommandsEnd = aCommands.end  ();

    while (lIterCommands != lIterCommandsEnd) {
      executeCommand(*lIterCommands, pParams);
      ++lIterCommands;
    }
  }
}

void Object::constructDisplayList(RenderParameters& pParams)
{
  // First, we make sure that all the display lists for that
  // RenderParameters::RenderMode for all the children are constructed
  // We have to loop on the aCommands and not directly on aSubObjects
  // because some commands change the attributes of pParams and
  // influence the rendering
  Commands::const_iterator       lIterCommands    = aCommands.begin();
  const Commands::const_iterator lIterCommandsEnd = aCommands.end  ();
  bool                           lAllChildrenOk   = true;
  RenderParameters               lParams          = pParams;

  while (lIterCommands != lIterCommandsEnd) {

    std::string::size_type lEndWord = std::string::npos;
    std::string            lCommand = extractCommandWord(*lIterCommands, lEndWord);

    std::string lParameters;
    if(lEndWord != std::string::npos) {
      lParameters = trimString(lIterCommands->substr(lEndWord+1), " \t\n");
    }

    if (lCommand == "execute_subobjects_id") {

      GLV_ASSERT(countWords(lParameters) == 1);
      int lSubObjectId = atoi(lParameters.c_str());
      GLV_ASSERT(lSubObjectId >= 0);
      GLV_ASSERT(lSubObjectId <  static_cast<int>(aSubObjects.size()));

      Object* lSubObject = aSubObjects[lSubObjectId];

      // The sub-Object might have been deleted
      if (lSubObject != 0) {

        // this->aFrozen is true, then all the SubObjects and their
        // children should have aFrozen set to true. In other words,
        // an Object can't be frozen if one of its parts are still
        // subject to change. This is automatic in the recursive
        // parsing order, but we make sure here that we don't have a bug.
        GLV_ASSERT(lSubObject->aFrozen);

        GLuint& lGLDisplayListSubObject = lSubObject->getGLDisplayList(lParams);

        if (lGLDisplayListSubObject == 0 || glIsList(lGLDisplayListSubObject) == GL_FALSE) {
          lSubObject->constructDisplayList(lParams);
        }

        // If the construction of one display list fails,
        // then we are not in a position to construct the
        // current display list
        lAllChildrenOk = (lAllChildrenOk                         &&
                          lGLDisplayListSubObject           != 0 &&
                          glIsList(lGLDisplayListSubObject) == GL_TRUE);

      }
    }
    else if(lCommand == "draw_facetboundary_enable") {
      lParams.aFlagRenderFacetFrame = true;
    }
    else if(lCommand == "draw_facetboundary_disable") {
      lParams.aFlagRenderFacetFrame = false;
    }

    ++lIterCommands;
  }


  if (!lAllChildrenOk) {
    // We can't create a display list  for the current Object
    // because one of the children wasn't able to create its own
    // display list (OpenGL ressources exhausted?)
    GLV_ASSERT(getGLDisplayList(pParams) == 0);
  }
  else {

    GLuint& lGLDisplayList = getGLDisplayList(pParams);

    // Starts recording lGLDisplayList
    // Reuse the already allocated aGLDisplayList if possibility

    if (lGLDisplayList == 0) {
      lGLDisplayList = glGenLists(1);
    }

    // There is a possibility that GL refuses to
    // generate a new display list
    if (glIsList(lGLDisplayList) == GL_FALSE) {
      // For the rest of the code, if for some obscur reason
      // glIsList(aGLDisplayList) return GL_FALSE even if
      // glGenLists returned a non-zero value, then we
      // consider the display list id as not initialised
      lGLDisplayList = 0;
    }

    if (lGLDisplayList != 0) {

      glNewList(lGLDisplayList, GL_COMPILE);

      lIterCommands = aCommands.begin();

      while (lIterCommands != lIterCommandsEnd) {
        executeCommand(*lIterCommands, pParams);
        ++lIterCommands;
      }

      glEndList();
    }

    GLV_ASSERT(lGLDisplayList == 0 || glIsList(lGLDisplayList) == GL_TRUE);
  }
}

void Object::executeCommand(const std::string& pCommand,
                            RenderParameters&  pParams) const
{
  // All the data is validated at this point, so we can
  // assert all the way!!!! :-)
  GLV_ASSERT(!pCommand.empty());

  // The stored commands in aCommands could be stored in
  // a structure instead of strings. This would speed up
  // things most definitely, but for now we keep the
  // string representation until we see the need to
  // change it.

  std::string::size_type lEndWord = std::string::npos;
  std::string            lCommand = extractCommandWord(pCommand, lEndWord);

  std::string lParameters;
  if(lEndWord != std::string::npos) {
    lParameters = trimString(pCommand.substr(lEndWord+1), " \t\n");
  }

  GLV_ASSERT(!lCommand.empty());

  if (lCommand == "execute_primitive_accumulator_id") {
    GLV_ASSERT(countWords(lParameters) == 1);
    int lPrimitiveAccumulatorId = atoi(lParameters.c_str());
    GLV_ASSERT(lPrimitiveAccumulatorId >= 0);
    GLV_ASSERT(lPrimitiveAccumulatorId <  static_cast<int>(aPrimitiveAccumulators.size()));
    GLV_ASSERT(aPrimitiveAccumulators[lPrimitiveAccumulatorId] != 0);

    aPrimitiveAccumulators[lPrimitiveAccumulatorId]->render(pParams);

  }
  else if (lCommand == "execute_vertex_primitive_accumulator_id") {
    GLV_ASSERT(countWords(lParameters) == 1);
    int lVertexedPrimitiveAccumulatorId = atoi(lParameters.c_str());
    GLV_ASSERT(lVertexedPrimitiveAccumulatorId >= 0);
    GLV_ASSERT(lVertexedPrimitiveAccumulatorId <  static_cast<int>(aVertexedPrimitiveAccumulators.size()));
    GLV_ASSERT(aVertexedPrimitiveAccumulators[lVertexedPrimitiveAccumulatorId] != 0);

    aVertexedPrimitiveAccumulators[lVertexedPrimitiveAccumulatorId]->render(pParams);

  }
  else if (lCommand == "execute_subobjects_id") {
    GLV_ASSERT(countWords(lParameters) == 1);
    int lSubObjectId = atoi(lParameters.c_str());
    GLV_ASSERT(lSubObjectId >= 0);
    GLV_ASSERT(lSubObjectId <  static_cast<int>(aSubObjects.size()));

    // The sub-Object might have been deleted
    if (aSubObjects[lSubObjectId] != 0) {

      // We don't want the transformations of that
      // sub-Object to influence the transformation
      // of other sub-Objects and the parents ;-)
      glPushMatrix();
      glPushAttrib(GL_CURRENT_BIT | GL_POINT_BIT | GL_LINE_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT);

      aSubObjects[lSubObjectId]->render(pParams);

      glPopAttrib();
      glPopMatrix();
    }

  }
  // DIRECT OPENGL CALLS
  else if(lCommand == "glcolor") {
    float r,g,b;
    sscanf(lParameters.c_str(), "%f %f %f", &r, &g, &b);
    glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(r, g, b);

  }
  else if(lCommand == "glpushmatrix") {
    GLV_ASSERT(lParameters == "");
    glPushMatrix();

  }
  else if(lCommand == "glpopmatrix") {
    GLV_ASSERT(lParameters == "");
    glPopMatrix();

  }
  else if(lCommand == "glbegin_triangles") {
    GLV_ASSERT(lParameters == "");
    glBegin(GL_TRIANGLES);

  }
  else if(lCommand == "glbegin_lines") {
    GLV_ASSERT(lParameters == "");
    glBegin(GL_LINES);

  }
  else if(lCommand == "glbegin_points") {
    GLV_ASSERT(lParameters == "");
    glBegin(GL_POINTS);

  }
  else if(lCommand == "glend") {
    GLV_ASSERT(lParameters == "");
    glEnd();

  }
  else if(lCommand == "glvertex") {
    float x,y,z;
    sscanf(lParameters.c_str(), "%f %f %f", &x, &y, &z);
    glVertex3f(x, y, z);

  }
  else if(lCommand == "gltranslate") {
    float x,y,z;
    sscanf(lParameters.c_str(), "%f %f %f", &x, &y, &z);
    glTranslatef(x, y, z);

  }
  else if(lCommand == "glscale") {
    float x,y,z;
    sscanf(lParameters.c_str(), "%f %f %f", &x, &y, &z);
    glScalef(x, y, z);

  }
  else if(lCommand == "glpointsize") {
    float lSize;
    sscanf(lParameters.c_str(), "%f", &lSize);
    glPointSize(lSize);

  }
  else if(lCommand == "gllinewidth") {
    float lSize;
    sscanf(lParameters.c_str(), "%f", &lSize);
    glLineWidth(lSize);

  }
  // DRAWING PARAMETERS
  else if(lCommand == "draw_single_sided") {
    GLV_ASSERT(lParameters == "");
    glEnable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);

  }
  else if(lCommand == "draw_double_sided") {
    GLV_ASSERT(lParameters == "");
    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

  }
  else if(lCommand == "glenable_polygonoffset_fill") {
    GLV_ASSERT(lParameters == "");
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);

  }
  else if(lCommand == "gldisable_polygonoffset_fill") {
    GLV_ASSERT(lParameters == "");
    glDisable(GL_POLYGON_OFFSET_FILL);

  }
  else if(lCommand == "draw_facetboundary_enable") {
    pParams.aFlagRenderFacetFrame = true;
    sscanf(lParameters.c_str(), "%f %f %f", &pParams.aFacetBoundaryR, &pParams.aFacetBoundaryG, &pParams.aFacetBoundaryB);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);

  }
  else if(lCommand == "draw_facetboundary_disable") {
    pParams.aFlagRenderFacetFrame = false;

  }
  else if(lCommand == "text") {
    char lParamFont[64];
    float x,y,z;

    sscanf(lParameters.c_str(), "%f %f %f %s", &x, &y, &z, lParamFont);

    GLV_ASSERT(lParameters.find("\"") != std::string::npos);
    std::string::size_type lStart = lParameters.find("\"") + 1;

    GLV_ASSERT(lParameters.find("\"",lStart) != std::string::npos);
    std::string::size_type lEnd   = lParameters.find("\"",lStart);

    std::string lText = lParameters.substr(lStart,lEnd-lStart);

    // Make sure we dont draw with lighting enabled
    // but.. we dont want to mess around with the lighting settings
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    void* lFont = GLUT_BITMAP_8_BY_13;
    if(!strcmp(lParamFont,"fixed13")) {
      lFont = GLUT_BITMAP_8_BY_13;
    }
    else if(!strcmp(lParamFont,"fixed15")) {
      lFont = GLUT_BITMAP_9_BY_15;
    }
    else if(!strcmp(lParamFont,"times10")) {
      lFont = GLUT_BITMAP_TIMES_ROMAN_10;
    }
    else if(!strcmp(lParamFont,"times24")) {
      lFont = GLUT_BITMAP_TIMES_ROMAN_24;
    }
    else if(!strcmp(lParamFont,"helvetica10")) {
      lFont = GLUT_BITMAP_HELVETICA_10;
    }
    else if(!strcmp(lParamFont,"helvetica12")) {
      lFont = GLUT_BITMAP_HELVETICA_12;
    }
    else if(!strcmp(lParamFont,"helvetica18")) {
      lFont = GLUT_BITMAP_HELVETICA_18;
    }

    drawText3D(lText, x, y, z, lFont);

    // Revert to previous lighting settings
    glPopAttrib();

  }
  else {
    GLV_ASSERT(false);
  }
}

PrimitiveAccumulator& Object::getCurrentPrimitiveAccumulator()
{
  if (aNewPrimitiveAccumulatorNeeded) {

    aPrimitiveAccumulators.push_back(new PrimitiveAccumulator(true));

    char lCommand[64];
    sprintf(lCommand, "execute_primitive_accumulator_id %d", aPrimitiveAccumulators.size()-1);
    aCommands.push_back(lCommand);

    aNewPrimitiveAccumulatorNeeded = false;
  }

  GLV_ASSERT(!aPrimitiveAccumulators.empty());
  GLV_ASSERT(aPrimitiveAccumulators.back() != 0);
  return *(aPrimitiveAccumulators.back());
}

VertexAccumulator& Object::getCurrentVertexAccumulator()
{
  if (aNewVertexAccumulatorNeeded) {

    aVertexAccumulators.push_back(new VertexAccumulator);

    aNewVertexAccumulatorNeeded = false;
  }

  GLV_ASSERT(!aVertexAccumulators.empty());
  GLV_ASSERT(aVertexAccumulators.back() != 0);
  return *(aVertexAccumulators.back());
}

VertexedPrimitiveAccumulator& Object::getCurrentVertexedPrimitiveAccumulator()
{
  if (aNewVertexedPrimitiveAccumulatorNeeded) {

    aVertexedPrimitiveAccumulators.push_back(new VertexedPrimitiveAccumulator(getCurrentVertexAccumulator()));

    char lCommand[64];
    sprintf(lCommand, "execute_vertex_primitive_accumulator_id %d", aVertexedPrimitiveAccumulators.size()-1);
    aCommands.push_back(lCommand);

    aNewVertexedPrimitiveAccumulatorNeeded = false;
  }

  GLV_ASSERT(!aVertexedPrimitiveAccumulators.empty());
  GLV_ASSERT(aVertexedPrimitiveAccumulators.back() != 0);
  return *(aVertexedPrimitiveAccumulators.back());
}

GLuint& Object::getGLDisplayList(const RenderParameters& pParams)
{
  switch (pParams.aRenderMode) {
  case RenderParameters::renderMode_full:
    return aGLDisplayListFull;
    break;
  case RenderParameters::renderMode_bounding_box:
    return aGLDisplayListBoundingBox;
    break;
  case RenderParameters::renderMode_fast:
    return aGLDisplayListFast;
    break;
  default:
    GLV_ASSERT(false);
    break;
  }
  return aGLDisplayListFast;
}
