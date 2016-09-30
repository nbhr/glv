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

#ifndef GRAPHICDATA_H
#define GRAPHICDATA_H

#include <string>

class BoundingBox;
class Object;
class Parser;
class RenderParameters;

class GraphicData {
public:

  GraphicData ();
  ~GraphicData();

  void                deleteDisplayLists   ();

  void                dumpCharacteristics  (std::ostream&      pOstream,
                                            const std::string& pIndentation);

  void                enableIgnoreErrorMode();

  void                enableSmoothingMode  ();

  void                enableStdinMode      ();

  const BoundingBox&  getGlobalBoundingBox () const;

  bool                newDataParsed        ();

  void                readDataFile         (const std::string& pFilename,
                                            std::string&       pError);

  void                render               (RenderParameters&  pParams);

  void                reset                ();

  void                setOptimizerValue    (const int          pOptimizerValue);

  bool                timerCallback        ();

private:

  // Block the use of those
  GraphicData(const GraphicData&);
  GraphicData& operator=(const GraphicData&);

  bool    aFlagCheckStdin;
  bool    aFlagNewData;
  bool    aFlagSmoothing;
  int     aOptimizerValue;
  Parser* aParser;
  Object* aRootObject;

};

#endif // GRAPHICDATA_H

