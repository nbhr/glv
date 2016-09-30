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

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string>
#include <vector>

class Object;

class Parser
{
public:

  Parser();
  ~Parser();

  void                enableIgnoreErrorMode();

  const std::string&  getCurrentFilename   () const;

  int                 getCurrentLineNumber () const;

  bool                newDataParsed        ();

  void                parseInputFile       (const std::string& pFilename,
                                            std::string&       pError);

  void                popObject            (Object* pObjPtr);

  void                pushObject           (Object* pObjPtr);

  void                readNewDataFromStream(FILE*              pFilePtr,
                                            std::string&       pError);

private:

  void parseLineExit      (const std::string& pLine,
                           std::string&       pError);
  void parseLineInclude   (const std::string& pLine,
                           std::string&       pError);
  void parseLineQuit      (const std::string& pLine,
                           std::string&       pError);
  void parseLineRaw       (FILE*              pFilePtr,
                           const std::string& pLine,
                           std::string&       pError);
  void parseLineSnapshot  (const std::string& pLine,
                           std::string&       pError);
  void parseLineTitle     (const std::string& pLine,
                           std::string&       pError);
  void parseLineView      (const std::string& pLine,
                           std::string&       pError);


  static const char                    aDirectorySeparator;
  static const std::string::size_type  aMaxLineLenght;

  std::vector<std::string> aDirectoryStack;
  std::vector<std::string> aFilenameStack;
  std::vector<int>         aLineNumberStack;
  bool                     aFlagIgnoreErrors;
  bool                     aFlagNewData;
  bool                     aFlagNewView;
  std::vector<Object*>     aObjectStack;

};


void addError          (const std::string& pString,
                        const Parser&      pCurrentParser,
                        std::string&       pError);

void addRawSectionError(const std::string& pSection,
                        const std::string& pArguements,
                        const Parser&      pCurrentParser,
                        std::string&       pError);

void addSyntaxError    (const std::string& pCommand,
                        const std::string& pArguements,
                        const Parser&      pCurrentParser,
                        std::string&       pError);

#endif // PARSER_H
