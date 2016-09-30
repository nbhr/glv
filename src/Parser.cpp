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

#include "Parser.h"
#include "assert_glv.h"
#include "Snapshot.h"
#include "Object.h"
#include "string_utils.h"
#include "WindowGLV.h"

// For usleep
#ifndef WIN32
#include <unistd.h>
#else
#include <stdlib.h>
static void usleep(int pLen) {
  _sleep(pLen/1000.);
}
#endif

#ifdef WIN32
const char Parser::aDirectorySeparator('\\');
#else
const char Parser::aDirectorySeparator('/');
#endif

const std::string::size_type Parser::aMaxLineLenght = 1023;


static
bool isBlank(const std::string& pLine)
{
  return pLine.find_first_not_of(" \n\t") == std::string::npos;
}

// Extract the command word from the line, giving also the
// position of the blank after the word.
std::string extractCommandWord(const std::string&      pLine,
                               std::string::size_type& pEndWord)
{
  unsigned int lStartWord = pLine.find_first_not_of(" \t");

  if (lStartWord == std::string::npos) {
    pEndWord = std::string::npos;
    return ""; // Empty command
  }

  pEndWord = pLine.find_first_of(" \t\n",lStartWord+1);
  return pLine.substr(lStartWord, pEndWord-lStartWord);
}

Parser::Parser()
  : aDirectoryStack  (),
    aFilenameStack   (),
    aFlagIgnoreErrors(false),
    aFlagNewData     (false),
    aObjectStack     ()
{
  // Add one default filename that represents stdin and line number.
  // This way, we won't have to check that !aFilenameStack.empty()
  // every time we want to display add an error message
  aFilenameStack  .push_back("stdin");
  aLineNumberStack.push_back(1);
}

Parser::~Parser()
{}

void Parser::enableIgnoreErrorMode()
{
  aFlagIgnoreErrors = true;
}

const std::string& Parser::getCurrentFilename() const
{
  GLV_ASSERT(!aFilenameStack.empty());
  GLV_ASSERT(aFilenameStack.size() == aLineNumberStack.size());
  return aFilenameStack.back();
}

int Parser::getCurrentLineNumber() const
{
  GLV_ASSERT(!aLineNumberStack.empty());
  GLV_ASSERT(aFilenameStack.size() == aLineNumberStack.size());
  return aLineNumberStack.back();
}

bool Parser::newDataParsed()
{
  bool lReturnValue = aFlagNewData;
  aFlagNewData      = false;
  return lReturnValue;
}

// Parse the file located at pFilename
// pFilename can contain the path to the file
// If pError.empty() == 0 on exit, then everything was fine
void Parser::parseInputFile(const std::string& pFilename,
                            std::string&       pError)
{
  GLV_ASSERT(pError.empty());

  // Extract the path and object name from the filename
  std::string            lDirectory;
  std::string            lObjectName;
  std::string::size_type lLastSeparatorIndex = pFilename.find_last_of(aDirectorySeparator);

  if (lLastSeparatorIndex != std::string::npos) {
    lDirectory  = pFilename.substr(0, lLastSeparatorIndex+1);
    lObjectName = pFilename.substr(lLastSeparatorIndex+1);
  }
  else {
    lObjectName = pFilename;
  }

  bool lFileIsGzip  = (pFilename.size() > 3 && pFilename.substr(pFilename.size()-3) == ".gz");
  bool lFileIsBzip  = (pFilename.size() > 3 && pFilename.substr(pFilename.size()-3) == ".bz");
  bool lFileIsBzip2 = (pFilename.size() > 4 && pFilename.substr(pFilename.size()-4) == ".bz2");

  // Remove .gz extension from object name if one is present
  if(lFileIsGzip || lFileIsBzip) {
    lObjectName = lObjectName.substr(0, lObjectName.size()-3);
  }
  else if(lFileIsBzip2) {
    lObjectName = lObjectName.substr(0, lObjectName.size()-4);
  }

  // Remove .gl extension from object name if one is present
  if (lObjectName.substr(lObjectName.size()-3) == ".gl") {
    lObjectName = lObjectName.substr(0, lObjectName.size()-3);
  }


  FILE* lFilePtr = 0;

#ifndef WIN32
  // Check if we have to use gzip -dc first


  if(lFileIsGzip) {

    // First, we check if we can open the .gz file
    lFilePtr = fopen(pFilename.c_str(), "r");

    if (lFilePtr == 0) {
      addError(std::string("Can't open file : ") + pFilename, *this, pError);
    }
    else {
      fclose(lFilePtr);
      lFilePtr = 0;

      std::cerr << "Deflating/reading: " << pFilename << std::endl;

      std::string lGzipCommand = std::string("gzip -dc ") + pFilename;

      lFilePtr = popen(lGzipCommand.c_str(), "r");

      if (lFilePtr == 0) {
        addError(std::string("Could not deflate .gz file : ") + pFilename, *this, pError);
      }
    }
  }
  else if(lFileIsBzip || lFileIsBzip2) {

    // First, we check if we can open the .gz file
    lFilePtr = fopen(pFilename.c_str(), "r");

    if (lFilePtr == 0) {
      addError(std::string("Can't open file : ") + pFilename, *this, pError);
    }
    else {
      fclose(lFilePtr);
      lFilePtr = 0;

      std::cerr << "Deflating/reading: " << pFilename << std::endl;

      std::string lBzip2Command = std::string("bzip2 -dc ") + pFilename;

      lFilePtr = popen(lBzip2Command.c_str(), "r");

      if (lFilePtr == 0) {
        if (lFileIsBzip) {
          addError(std::string("Could not deflate .bz file : ") + pFilename, *this, pError);
        }
        else {
          GLV_ASSERT(lFileIsBzip2);
          addError(std::string("Could not deflate .bz2 file : ") + pFilename, *this, pError);
        }
      }
    }
  }
  else {
#endif // #ifndef WIN32

    lFilePtr = fopen(pFilename.c_str(), "r");

    if (lFilePtr == 0) {
      addError(std::string("Can't open file : ") + pFilename, *this, pError);
    }

#ifndef WIN32
  }
#endif // #ifndef WIN32

  if (pError.empty()) {

    std::cerr << "Reading : " << pFilename << std::endl;

    GLV_ASSERT(lFilePtr != 0);

    aLineNumberStack.push_back(1);
    aFilenameStack  .push_back(pFilename);
    aDirectoryStack .push_back(lDirectory);

    // We want to have a separate object for each file
    // Extract the name of the object from the filename

    std::string lLocalError;
    aObjectStack.back()->addCommand("object_begin",
                                    lObjectName,
                                    *this,
                                    lLocalError);

    // Read file
    readNewDataFromStream(lFilePtr, pError);

    // Call object_end even if pError.empty() is false
    aObjectStack.back()->addCommand("object_end",
                                    "",
                                    *this,
                                    lLocalError);

    aDirectoryStack .pop_back();
    aFilenameStack  .pop_back();
    aLineNumberStack.pop_back();

    fclose(lFilePtr);
  }

  if (aFlagIgnoreErrors && !pError.empty()) {
    std::cerr << pError << std::endl;
    pError = "";
  }
}

void Parser::popObject(Object* pObjPtr)
{
  GLV_ASSERT(!aObjectStack.empty());
  GLV_ASSERT(aObjectStack.back() == pObjPtr);
  aObjectStack.pop_back();
}

void Parser::pushObject(Object* pObjPtr)
{
  GLV_ASSERT(pObjPtr != 0);
  aObjectStack.push_back(pObjPtr);
}

// Parse an already opened file pointed of file pointer pFilePtr
// pFilename here is
// If pError.empty() == 0 on exit, then everything was fine
// Returns true if new data was present
void Parser::readNewDataFromStream(FILE*        pFilePtr,
                                   std::string& pError)
{
  GLV_ASSERT(pError.empty());
  GLV_ASSERT(!aObjectStack.empty()); // forgot to do a pushObject?

  char  lBuffer[aMaxLineLenght+1];
  char* lGotPtr = 0;

  // Read line from the stream; until end of file , or end of
  // available data (null pointer, because the stream is non-blocking in some
  // situations (stdin read))
  lGotPtr = fgets(lBuffer, aMaxLineLenght+1, pFilePtr);

  while(!feof(pFilePtr) && lGotPtr) {

    std::string lLine(lBuffer);

    // Make sure the read line is a complete line... if the data we got in the
    // input is not terminated by a end-of-line; we have to wait until
    // we get a end of line.
    // This could happen with slow pipes, interrupted processes, and slow
    // input filters.
    while(lLine[lLine.size()-1] != '\n' && !feof(pFilePtr)) {
      usleep(10);
      if(fgets(lBuffer, aMaxLineLenght+1, pFilePtr)) {
        lLine += lBuffer;
      }
    }

    const std::string::size_type lSz = lLine.size();
    if(lSz >= aMaxLineLenght) {
      addError("Input file - line too long", *this, pError);
    }

    if (aFlagIgnoreErrors && !pError.empty()) {
      std::cerr << pError << std::endl;
      pError = "";
    }

    if (pError.empty()) {

      if(lLine[lSz-1] == '\n') {
        // Strip end of line
        lLine.resize(lSz-1);
      }

      if(lLine[0] == '#') {
        lLine = ""; // comment
      }
      // COMMAND "include".
      else if(lLine.find("include ") == 0) {
        parseLineInclude(lLine, pError);
        lLine = ""; // nothing else to parse
      }
      else if(lLine.find("raw_") == 0) {
        parseLineRaw(pFilePtr, lLine, pError);
        lLine = ""; // nothing else to parse
      }
      else if(lLine.find("title") == 0) {
        parseLineTitle(lLine, pError);
        lLine = ""; // nothing else to parse
      }
      else if(lLine.find("snapshot ") == 0) {
        parseLineSnapshot(lLine, pError);
        lLine = ""; // nothing else to parse
      }
      else if(lLine.find("view ") == 0) {
        parseLineView(lLine, pError);
        lLine = ""; // nothing else to parse
      }
      else if(lLine.find("exit") == 0) {
        parseLineExit(lLine, pError);
        lLine = ""; // nothing else to parse
      }
      else if(lLine.find("quit") == 0) {
        parseLineQuit(lLine, pError);
        lLine = ""; // nothing else to parse
      }
    }

    if (aFlagIgnoreErrors && !pError.empty()) {
      std::cerr << pError << std::endl;
      pError = "";
    }

    if (pError.empty()) {

      // Command processing:
      if(!isBlank(lLine)) {

        std::string::size_type lEndWord = std::string::npos;
        std::string            lCommand = extractCommandWord(lLine, lEndWord);

        GLV_ASSERT(!lCommand.empty());

        std::string lParameters;
        if(lEndWord != std::string::npos) {
          lParameters = trimString(lLine.substr(lEndWord+1), " \n\t");
        }

        aObjectStack.back()->addCommand(lCommand,
                                        lParameters,
                                        *this,
                                        pError);
        if (pError.empty()) {
          aFlagNewData = true;
        }
      }
    }

    if (aFlagIgnoreErrors && !pError.empty()) {
      std::cerr << pError << std::endl;
      pError = "";
    }

    if (pError.empty()) {
      lGotPtr = fgets(lBuffer, aMaxLineLenght+1, pFilePtr);
      if (lGotPtr) {
        GLV_ASSERT(!aLineNumberStack.empty());
        ++(aLineNumberStack.back());
      }
    }
    else {
      lGotPtr = 0;
    }
  }

  if (aFlagIgnoreErrors && !pError.empty()) {
    std::cerr << pError << std::endl;
    pError = "";
  }
}

void Parser::parseLineExit(const std::string& pLine,
                           std::string&       pError)
{
  GLV_ASSERT(pLine.find("exit") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);

  if (trimString(pLine, " \t\n") == "exit") {
    exit(0);
  }
  else {
    addSyntaxError("exit", "", *this, pError);
  }
}

void Parser::parseLineInclude(const std::string& pLine,
                              std::string&       pError)
{
  GLV_ASSERT(pError.empty());
  GLV_ASSERT(pLine.find("include ") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);
  GLV_ASSERT(!aObjectStack.empty()); // forgot to do a pushObject?

  // Use the last directory in aDirectoryStack if the path is
  // realive. If aDirectoryStack is empty, then the current
  // path is used. Finally, if an absolute path is used then
  // we do not use aDirectoryStack.

  // Extract the filename
  std::string lFilename = trimString(pLine.substr(8), " \t\n");

  if (lFilename.empty()) {
    addSyntaxError("include", "filename", *this, pError);
  }
  else {

#ifdef WIN32
    const bool lAbsolutePath = ((lFilename[0] >= 'a' && lFilename[0] <= 'z' ||
                                 lFilename[0] >= 'A' && lFilename[0] <= 'Z') &&
                                lFilename[1] == ':' &&
                                lFilename[2] == aDirectorySeparator);
#else // #ifdef WIN32
    const bool lAbsolutePath = (lFilename[0] == aDirectorySeparator);
#endif // #ifdef WIN32

    if (!lAbsolutePath) {

      std::string lDirectory;

      if(!aDirectoryStack.empty()) {
        lDirectory = aDirectoryStack.back();
      }

      // Add directory component to relative filenames
      lFilename = lDirectory + lFilename;
    }

    parseInputFile(lFilename, pError);
  }
}

void Parser::parseLineQuit(const std::string& pLine,
                           std::string&       pError)
{
  GLV_ASSERT(pLine.find("quit") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);

  if (trimString(pLine, " \t\n") == "quit") {
    exit(0);
  }
  else {
    addSyntaxError("quit", "", *this, pError);
  }
}

void Parser::parseLineRaw(FILE*              pFilePtr,
                          const std::string& pLine,
                          std::string&       pError)
{
  GLV_ASSERT(pError.empty());
  GLV_ASSERT(pLine.find("raw_") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);
  GLV_ASSERT(!aObjectStack.empty()); // forgot to do a pushObject?

  std::string::size_type lEndWord  = std::string::npos;
  std::string            lCommand = extractCommandWord(pLine, lEndWord);

  GLV_ASSERT(!lCommand.empty());

  std::string lParameters;
  if(lEndWord != std::string::npos) {
    lParameters = trimString(pLine.substr(lEndWord+1), " \t\n");
  }

  aObjectStack.back()->addCommand(lCommand,
                                  lParameters,
                                  *this,
                                  pError);

  if (pError.empty()) {

    aFlagNewData = true;

    // The object is now in waiting for raw data
    // until it sees the "raw_end" command. To process
    // those lines without a keyword at the beginning
    // more easily, we introduce new commands only
    // valid internally for the raw lines
    const std::string lRawItemCommand = lCommand + "_item";

    char  lBuffer[aMaxLineLenght+1];
    bool  lFlagDisplayWarning = false;
    char* lGotPtr             = 0;

    lGotPtr = fgets(lBuffer, aMaxLineLenght+1, pFilePtr);

    GLV_ASSERT(!aLineNumberStack.empty());
    ++(aLineNumberStack.back());

    while(!feof(pFilePtr) && lGotPtr) {

      std::string lLine(lBuffer);

      // Make sure the read line is a complete line... if the data we got in the
      // input is not terminated by a end-of-line; we have to wait until
      // we get a end of line.
      // This could happen with slow pipes, interrupted processes, and slow
      // input filters.
      while(lLine[lLine.size()-1] != '\n' && !feof(pFilePtr)) {
        if (!lFlagDisplayWarning) {
          std::cerr << "Reading raw data block..." << std::endl;
          lFlagDisplayWarning = true;
        }
        usleep(10);
        if(fgets(lBuffer, aMaxLineLenght+1, pFilePtr)) {
          lLine += lBuffer;
        }
      }

      const std::string::size_type lSz = lLine.size();
      if(lSz >= aMaxLineLenght) {
        addError("Input file - line too long", *this, pError);
      }

      if (aFlagIgnoreErrors && !pError.empty()) {
        std::cerr << pError << std::endl;
        pError = "";
      }

      if (pError.empty()) {

        const std::string lTrimmedLine = trimString(lLine, " \t\n");

        if(lTrimmedLine.find("raw_end") == 0) {

          // We have the raw_end. We send it to the Object
          // and then leave this member function to continue
          // non-raw parsing operations
          lCommand = extractCommandWord(lTrimmedLine, lEndWord);

          GLV_ASSERT(!lCommand.empty());

          lParameters = "";
          if(lEndWord != std::string::npos) {
            lParameters = trimString(pLine.substr(lEndWord+1), " \t\n");
          }

          aObjectStack.back()->addCommand(lCommand,
                                          lParameters,
                                          *this,
                                          pError);

          if (pError.empty()) {
            aFlagNewData = true;
          }

          break;
        }
        else {

          // We send the trimmed line as a raw item
          aObjectStack.back()->addCommand(lRawItemCommand,
                                          lTrimmedLine,
                                          *this,
                                          pError);

          if (pError.empty()) {
            aFlagNewData = true;
          }
        }
      }

      if (aFlagIgnoreErrors && !pError.empty()) {
        std::cerr << pError << std::endl;
        pError = "";
      }

      if (pError.empty()) {
        lGotPtr = fgets(lBuffer, aMaxLineLenght+1, pFilePtr);
        if (lGotPtr) {
          GLV_ASSERT(!aLineNumberStack.empty());
          ++(aLineNumberStack.back());
        }
      }
      else {
        lGotPtr = 0;
      }
    }
    if(lFlagDisplayWarning) {
      std::cerr << "...done" << std::endl;
    }
  }
}

void Parser::parseLineSnapshot(const std::string& pLine,
                               std::string&       pError)
{
  GLV_ASSERT(pLine.find("snapshot ") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);

  std::string lParameters = trimString(pLine.substr(9), " \t\n");

  int lWordCount = countWords(lParameters);

  if(lWordCount == 2) {

    std::string::size_type lSpaceIndex = lParameters.find_first_of(" \t");

    GLV_ASSERT(lSpaceIndex != std::string::npos);

    std::string lGeometry = trimString(lParameters.substr(0, lSpaceIndex), " \t\n");
    std::string lFilename = trimString(lParameters.substr(lSpaceIndex+1), " \t\n");

    GLV_ASSERT(lGeometry != "");
    GLV_ASSERT(lFilename != "");

    std::string::size_type lXIndex = lGeometry.find_first_of("xX");

    std::string lWidthString;
    std::string lHeightString;

    if (lXIndex != std::string::npos) {
      lWidthString  = lGeometry.substr(0, lXIndex);
      lHeightString = lGeometry.substr(lXIndex+1);
    }

    int       lWidth  = -1;
    int       lHeight = -1;
    const int nb1     = sscanf(lWidthString .c_str(), "%i", &lWidth );
    const int nb2     = sscanf(lHeightString.c_str(), "%i", &lHeight);

    const double lSize = static_cast<double>(lWidth)*static_cast<double>(lHeight);

    if(nb1 != 1 || nb2 != 1) {
      addSyntaxError("snapshot", "<width>x<height> filename.ext", *this, pError);
    }
    else if(lWidth < 1 || lHeight < 1 || lSize > Snapshot::getMaxPixmapSize()) {
      addError("Bad geometry argument in snapshot:\n  <width>x<height> out of range (max size is one Gigabyte)", *this, pError);
    }
    else {
      Snapshot lSnapshot(lWidth, lHeight, *this);

      lSnapshot.render(lFilename, pError);
    }
  }
  else {
    addSyntaxError("snapshot", "<width>x<height> filename.ext", *this, pError);
  }
}

void Parser::parseLineTitle(const std::string& pLine,
                            std::string&       pError)
{
  GLV_ASSERT(pLine.find("title") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);

  std::string lTitle;

  if (pLine.size() == 5) {
    GLV_ASSERT(pLine == "title");
  }
  else {
    GLV_ASSERT(pLine.size() > 5);
    lTitle = trimString(pLine.substr(6), " \t\n");
  }

  WindowGLV::getInstance().getViewManager().setTitle(lTitle);
  aFlagNewData = true;
}

void Parser::parseLineView(const std::string& pLine,
                           std::string&       pError)
{
  GLV_ASSERT(pError.empty());
  GLV_ASSERT(pLine.find("view ") == 0);
  GLV_ASSERT(pLine.size() < aMaxLineLenght);

  std::string lParameters = pLine.substr(5);
  int         lWordCount  = countWords(lParameters);

  // Multiple input syntax are possible
  if(lWordCount == 3) {

    float x,y,z;
    int nb = sscanf(lParameters.c_str(), "%f %f %f", &x, &y, &z);

    if(nb != 3) {
      addSyntaxError("view", "x y z [dy dy dz [ux uy uz dist zoom] ] ]", *this, pError);
    }
    else {
      View lView(Vector3D(x, y, z));
      WindowGLV::getInstance().getViewManager().addView(lView);
      aFlagNewData = true;
    }
  }
  else if(lWordCount == 6) {

    float x,y,z,dx,dy,dz;
    int   nb = sscanf(lParameters.c_str(), "%f %f %f %f %f %f",
                      &x, &y, &z, &dx, &dy, &dz);

    if(nb != 6) {
      addSyntaxError("view", "x y z [dy dy dz [ux uy uz dist zoom] ] ]", *this, pError);
    }
    else {
      View lView(Vector3D( x,  y,  z),
                 Vector3D(dx, dy, dz));
      WindowGLV::getInstance().getViewManager().addView(lView);
      aFlagNewData = true;
    }
  }
  else if(lWordCount == 11) {

    float x,y,z,dx,dy,dz,ux,uy,uz,dist,zoom;
    int   nb = sscanf(lParameters.c_str(), "%f %f %f %f %f %f %f %f %f %f %f",
                      &x, &y, &z, &dx, &dy, &dz, &ux, &uy, &uz, &dist, &zoom);

    if(nb != 11) {
      addSyntaxError("view", "x y z [dy dy dz [ux uy uz dist zoom] ] ]", *this, pError);
    }
    else {
      View lView(Vector3D( x,  y,  z),
                 Vector3D(dx, dy, dz),
                 Vector3D(ux, uy, uz),
                 dist,
                 zoom);
      WindowGLV::getInstance().getViewManager().addView(lView);
      aFlagNewData = true;
    }
  }
  else {
    addSyntaxError("view", "x y z [dy dy dz [ux uy uz dist zoom] ] ]", *this, pError);
  }
}


void addError(const std::string& pString,
              const Parser&      pCurrentParser,
              std::string&       pError)
{
  GLV_ASSERT(!pString.empty());
  GLV_ASSERT(pError.empty());

  pError += "Parsing error:\n";
  pError += pString + std::string("\n");

  const std::string& lFilename = pCurrentParser.getCurrentFilename();
  const int          lLineNo   = pCurrentParser.getCurrentLineNumber();

  char lLineString[64];
  sprintf(lLineString, "[ On line %d from %s ]", lLineNo, lFilename.c_str());
  pError += lLineString;
}

void addRawSectionError(const std::string& pSection,
                        const std::string& pArguments,
                        const Parser&      pCurrentParser,
                        std::string&       pError)
{
  GLV_ASSERT(!pSection.empty());
  GLV_ASSERT(pError.empty());

  std::string lString  = "Syntax error in a \"";
  lString             += pSection;
  lString             += "\" section\n  correct syntax: ";
  lString             += pArguments;

  addError(lString, pCurrentParser, pError);
}

void addSyntaxError(const std::string& pCommand,
                    const std::string& pArguments,
                    const Parser&      pCurrentParser,
                    std::string&       pError)
{
  GLV_ASSERT(!pCommand.empty());
  GLV_ASSERT(pError.empty());

  std::string lString  = "Syntax error in \"";
  lString             += pCommand;
  lString             += "\"\n  correct syntax: ";
  lString             += pCommand;
  lString             += " ";
  lString             += pArguments;

  addError(lString, pCurrentParser, pError);
}
