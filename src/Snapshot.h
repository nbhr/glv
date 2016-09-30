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

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <string>

class Parser;

class Snapshot
{
public:

  Snapshot (const int     pWidth,
            const int     pHeight,
            const Parser& pCurrentParser);
  ~Snapshot();

  static int getMaxPixmapSize();

  void       render          (const std::string& pFilename,
                              std::string&       pError);
private:

  // Block the use of those
  Snapshot();
  Snapshot(const Snapshot&);
  Snapshot& operator=(const Snapshot&);


#ifdef GLV_USE_GLX
  void renderGLX();
#endif // #ifdef GLV_USE_GLX

#ifdef GLV_USE_PNG
  void saveAsPNG(const std::string& pFilename,
                 std::string&       pError);
#endif // #ifdef GLV_USE_PNG


  static int aTileWidthAndHeight;

  unsigned char* aBufferImage;
  unsigned char* aBufferTile;
  const Parser&  aCurrentParser;
  int            aHeight;
  int            aWidth;


};

#endif // SNAPSHOT_H

