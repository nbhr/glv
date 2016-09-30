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

#include "GraphicData.h"
#include "assert_glv.h"
#include "glinclude.h"
#include "Matrix4x4.h"
#include "Object.h"
#include "Parser.h"
#include "RenderParameters.h"
#include "string_utils.h"

#include <fcntl.h>
#include <iostream>
#include <stdio.h>

std::string extractCommandWord(const std::string& pCommand,unsigned int& pEndWord);

// Constructor; initialize with default values
GraphicData::GraphicData()
  : aFlagCheckStdin(false),
    aFlagNewData   (false),
    aFlagSmoothing (false),
    aOptimizerValue(100),
    aParser        (0),
    aRootObject    (0)
{
  aRootObject = new Object;
  aParser     = new Parser;

  GLV_ASSERT(aRootObject != 0);
  GLV_ASSERT(aParser     != 0);
  aParser->pushObject(aRootObject);
}

// Destructor.  Delete allocated openGL display lists
GraphicData::~GraphicData()
{
  GLV_ASSERT(aRootObject != 0);
  GLV_ASSERT(aParser     != 0);

  delete aRootObject;
  delete aParser;
}

// Delete the display lists of the root Object and all its SubObjects.
// Used primarily to force the reconstruction of the display lists when
// different OpenGL contexts can't share them
void GraphicData::deleteDisplayLists()
{
  GLV_ASSERT(aRootObject != 0);

  aRootObject->deleteDisplayLists();
}

// Dump in ASCII the caracteristics of the GraphicData
void GraphicData::dumpCharacteristics(std::ostream&       pOstream,
                                      const std::string&  pIndentation)
{
  pOstream << std::endl;
  pOstream << pIndentation << ">>>>>>>>>>>  Characteristics" << std::endl;
  pOstream << std::endl;

  GLV_ASSERT(aRootObject != 0);
  aRootObject->dumpCharacteristics(pOstream, pIndentation, Matrix4x4());

  pOstream << std::endl;
  pOstream << pIndentation << "<<<<<<<<<<<  Characteristics" << std::endl;
  pOstream << std::endl;
}

void GraphicData::enableIgnoreErrorMode()
{
  GLV_ASSERT(aParser != 0);
  aParser->enableIgnoreErrorMode();
}

// Enable smoothing (only applicable to some primitives)
void GraphicData::enableSmoothingMode()
{
  aFlagSmoothing = true;
}

// Enable the reading of data from the stdin
//  To use in conjunction with files in argument; or for a
//  pure "shell-like" use of the app.
void GraphicData::enableStdinMode()
{
  aFlagCheckStdin = true;

  // Disable the "blocking" state of stdin
#ifndef WIN32
  fcntl(0, F_SETFL, O_NONBLOCK);
#endif // WIN32
}

// Returns the global BoundingBox based on the BoundingBox of all the objects
const BoundingBox& GraphicData::getGlobalBoundingBox() const
{
  GLV_ASSERT(aRootObject != 0);
  return aRootObject->getBoundingBox();
}

bool GraphicData::newDataParsed()
{
  GLV_ASSERT(aParser != 0);
  return aParser->newDataParsed();
}

// Read a command file.
// If pError.empty() is true on exit, then everything was fine
void GraphicData::readDataFile(const std::string& pFilename,
                               std::string&       pError)
{
  GLV_ASSERT(pError.empty());

  GLV_ASSERT(aParser != 0);
  aParser->parseInputFile(pFilename, pError);
}

// Render the accumulated graphic data.
void GraphicData::render(RenderParameters& pParams)
{
  GLV_ASSERT(aRootObject != 0);

  // Default values
  glColor3f(1.0f , 1.0f, 1.0f);

  pParams.aFlagSmoothNormals       = aFlagSmoothing;
  pParams.aPrimitiveOptimizerValue = aOptimizerValue;

  aRootObject->render(pParams);
}

//  Clear all accumulated graphic data
//  Does not clear user settings (such as stdin flags,
//  smoothing flags, ...)
void GraphicData::reset()
{
  GLV_ASSERT(aRootObject != 0);
  GLV_ASSERT(aParser     != 0);

  delete aRootObject;
  delete aParser;

  aRootObject = new Object;
  aParser     = new Parser;

  GLV_ASSERT(aRootObject != 0);
  GLV_ASSERT(aParser     != 0);
  aParser->pushObject(aRootObject);

  aFlagNewData = false;
}

void GraphicData::setOptimizerValue(const int pOptimizerValue)
{
  aOptimizerValue = pOptimizerValue;
}

// Timer callback - checks on the stdin if there is new commands to be read
bool GraphicData::timerCallback()
{
  if(aFlagCheckStdin) {

    std::string lError;

    GLV_ASSERT(aParser != 0);
    aParser->readNewDataFromStream(stdin, lError);

    if(!lError.empty()) {
      std::cerr << "Error parsing input:" << std::endl;
      std::cerr << lError << std::endl;
      exit(1);
    }
  }

  return newDataParsed();
}
