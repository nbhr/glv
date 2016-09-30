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

#include "Snapshot.h"
#include "assert_glv.h"
#include "glinclude.h"
#include "Parser.h"
#include "Tile.h"
#include "WindowGLV.h"

#include <string>

#ifdef GLV_USE_GLX
#include "glx.h"
#include "X11/Xlib.h"
#endif // #ifdef GLV_USE_GLX

#ifdef GLV_USE_PNG
#include "png.h"
#endif // #ifdef GLV_USE_PNG


// Choose a value that will require only
// one rendering for commmonly used resolutions
// (640x480 and 800x600)
int Snapshot::aTileWidthAndHeight = 800;


Snapshot::Snapshot(const int     pWidth,
                   const int     pHeight,
                   const Parser& pCurrentParser)
  : aBufferImage  (0),
    aBufferTile   (0),
    aCurrentParser(pCurrentParser),
    aHeight       (pHeight),
    aWidth        (pWidth)
{
  GLV_ASSERT(pWidth > 0);
  GLV_ASSERT(pHeight > 0);
  GLV_ASSERT(static_cast<double>(pWidth)*static_cast<double>(pHeight) <= getMaxPixmapSize());

  // 24 bits RGB images
  aBufferImage = new unsigned char[3*pWidth*pHeight];
  aBufferTile  = new unsigned char[3*aTileWidthAndHeight*aTileWidthAndHeight];
}

Snapshot::~Snapshot()
{
  GLV_ASSERT(aBufferImage != 0);
  GLV_ASSERT(aBufferTile  != 0);
  delete [] aBufferImage;
  delete [] aBufferTile;
}

// Returns the maximum number of pixels supported
// when creating a Snapshot
int Snapshot::getMaxPixmapSize()
{
  // One Gigabyte max for a 24 bits/pixel image.
  // (1024*1024*1024-1)/3 = 357913941
  return 357913941;
}

void Snapshot::render(const std::string& pFilename,
                      std::string&       pError)
{
  GLV_ASSERT(aBufferImage   != 0);
  GLV_ASSERT(aBufferTile    != 0);

  // Get the filename extension
  std::string::size_type lLastDotIndex = pFilename.find_last_of(".");
  std::string            lExtension;

  if (lLastDotIndex != std::string::npos) {
    lExtension = pFilename.substr(lLastDotIndex + 1);
  }

  // Unfortunately, for portability issues we do not use
  // std::transform
  std::string::iterator       lIter    = lExtension.begin();
  const std::string::iterator lIterEnd = lExtension.end  ();

  while (lIter != lIterEnd) {
    *lIter = tolower(*lIter);
    ++lIter;
  }

  bool lFileTypeAvailable = false;
  bool lSnapshotAvailable = false;

#ifdef GLV_USE_PNG
  if(lExtension == "png") {
    lFileTypeAvailable = true;
  }
#endif // #ifdef GLV_USE_PNG

#ifdef GLV_USE_GLX
  lSnapshotAvailable = true;
#endif // #ifdef GLV_USE_GLX

  if(!lFileTypeAvailable) {
    std::string lString  = std::string("Unsupported file format extension \"") +
                           lExtension +
                           std::string("\" in \"snapshot\"");
    addError(lString, aCurrentParser, pError);
  }
  else if (!lSnapshotAvailable) {
    addError("Snapshots support not compiled", aCurrentParser, pError);
  }
  else {

    std::cerr << "Rendering and writing: " << pFilename << std::endl;

#ifdef GLV_USE_GLX
    renderGLX();
#endif // #ifdef GLV_USE_GLX

#ifdef GLV_USE_PNG
    if(lExtension == "png") {

      saveAsPNG(pFilename, pError);

      lFileTypeAvailable = true;

      if (pError.empty()) {
        std::cerr << "Successful writing: " << pFilename << std::endl;
      }
    }
#endif // #ifdef GLV_USE_PNG

  }

}

#ifdef GLV_USE_GLX

// Renders tiles in memory using the GLX extension
// and puts the result in aBufferImage
void Snapshot::renderGLX()
{
  GLV_ASSERT(aBufferImage != 0);
  GLV_ASSERT(aBufferTile  != 0);

  ViewManager& lViewManager = WindowGLV::getInstance().getViewManager();

  // We can't use the same display lists as the one
  // created for the display in WindowGLV, so we force
  // the reconstruction their reconstruction
  lViewManager.getGraphicData().deleteDisplayLists();

  // Remove any message that is currently displayed
  lViewManager.clearStatusMessages();

  // Create the new GLX context that will be used to
  // render the image offscreen.
  Display*     lDisplay      = XOpenDisplay(NULL);
  int          lScreen       = DefaultScreen(lDisplay);
  int          lAttributes[] = {GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1, None};
  XVisualInfo* lXVisualInfo  = glXChooseVisual(lDisplay, lScreen, lAttributes);

  GLV_ASSERT(lXVisualInfo != 0);

  Window               lRootWindow = DefaultRootWindow(lDisplay);
  unsigned long        lMask       = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
  XSetWindowAttributes lXSetWindowAttributes;

  lXSetWindowAttributes.background_pixel = 0;
  lXSetWindowAttributes.border_pixel     = 0;
  lXSetWindowAttributes.colormap         = XCreateColormap(lDisplay, lRootWindow, lXVisualInfo->visual, AllocNone);
  lXSetWindowAttributes.event_mask       = StructureNotifyMask | ExposureMask;

  Window lWindow = XCreateWindow(lDisplay,
                                 lRootWindow,
                                 0,
                                 0,
                                 aTileWidthAndHeight,
                                 aTileWidthAndHeight,
                                 0,
                                 lXVisualInfo->depth,
                                 InputOutput,
                                 lXVisualInfo->visual,
                                 lMask,
                                 &lXSetWindowAttributes);

  Pixmap lPixmap = XCreatePixmap(lDisplay,
                                 lWindow,
                                 aTileWidthAndHeight,
                                 aTileWidthAndHeight,
                                 lXVisualInfo->depth);

  GLXPixmap lGLXPixmap = glXCreateGLXPixmap(lDisplay,
                                            lXVisualInfo,
                                            lPixmap);

  GLXContext lGLXContext = glXCreateContext(lDisplay,
                                            lXVisualInfo,
                                            NULL,
                                            False);

  glXMakeCurrent(lDisplay,
                 lGLXPixmap,
                 lGLXContext);

  const int lNbTilesX = (aWidth-1) /aTileWidthAndHeight + 1;
  const int lNbTilesY = (aHeight-1)/aTileWidthAndHeight + 1;

  for(int lTileIndexX=0; lTileIndexX<lNbTilesX; ++lTileIndexX) {

    // Since the Pixmap is of fixed size, we allow the
    // tile to go outside of the image
    const int lTileXMin  = lTileIndexX*aTileWidthAndHeight;
    const int lTileXMax  = lTileXMin + aTileWidthAndHeight - 1;

    // But we'll only copy the pixels that are located inside the image
    const int lTileWidth = std::min(lTileXMax - lTileXMin + 1, aWidth - lTileXMin);;

    for(int lTileIndexY=0; lTileIndexY<lNbTilesY; ++lTileIndexY) {

      // Since the Pixmap is of fixed size, we allow the
      // tile to go outside of the image
      const int lTileYMin  = lTileIndexY*aTileWidthAndHeight;
      const int lTileYMax  = lTileYMin + aTileWidthAndHeight - 1;

      // But we'll only copy the pixels that are located inside the image
      const int lTileHeight = std::min(lTileYMax - lTileYMin + 1, aHeight - lTileYMin);;

      const Tile lTile(aWidth,
                       aHeight,
                       lTileXMin,
                       lTileXMax,
                       lTileYMin,
                       lTileYMax);

      // Render for the current tile
      lViewManager.display(lTile);

      // Wait for the OpenGL operations to finish before reading the pixels
      glXWaitGL();

      // Read the pixels from the OpenGL buffer
      // For some reason, the OpenGL I'm using (nvidia)
      // is forcing that the lTileWidth is a multiple of 4
      // in glReadPixels.

      // Make sure that our buffer width is a multiple of 4
      GLV_ASSERT(aTileWidthAndHeight % 4 == 0);

      const int lPaddedTileWidth = lTileWidth - ((lTileWidth-1) % 4) + 3;

      GLV_ASSERT(lPaddedTileWidth >= lTileWidth);
      GLV_ASSERT(lPaddedTileWidth <= aTileWidthAndHeight);
      GLV_ASSERT(lPaddedTileWidth % 4 == 0);

      glReadPixels(0,
                   aTileWidthAndHeight - lTileHeight,
                   lPaddedTileWidth,
                   lTileHeight,
                   GL_RGB,
                   GL_UNSIGNED_BYTE,
                   aBufferTile);

      // Copy tile buffer in the image buffer
      // OpenGL has the origin in the lower left corner, that's
      // why we are using "--lImageI"
      for(int i=0, lImageI=lTileYMin+lTileHeight-1; i<lTileHeight; ++i, --lImageI) {
        GLV_ASSERT(lImageI >= 0);
        GLV_ASSERT(lImageI < aHeight);
        GLV_ASSERT(lTileXMin + lTileWidth <= aWidth);
        memcpy(aBufferImage + 3*(lImageI*aWidth + lTileXMin), aBufferTile + 3*i*lPaddedTileWidth, 3*lTileWidth);
      }
    }
  }


  // Destroy the context used for the offsreen rendering
  XDestroyWindow     (lDisplay, lWindow);
  glXDestroyContext  (lDisplay, lGLXContext);
  glXDestroyGLXPixmap(lDisplay, lGLXPixmap);
  XFreePixmap        (lDisplay, lPixmap);

  // Force the reconstruction of the display lists
  // for the display in WindowGLV
  lViewManager.getGraphicData().deleteDisplayLists();

}
#endif // #ifdef GLV_USE_GLX


#ifdef GLV_USE_PNG

void Snapshot::saveAsPNG(const std::string& pFilename,
                         std::string&       pError)
{
  GLV_ASSERT(aBufferImage   != 0);
  GLV_ASSERT(aBufferTile    != 0);

  FILE* lFilePointer = fopen(pFilename.c_str(), "wb");
  if (lFilePointer == 0) {
    addError(std::string("Error opening file ") + pFilename, aCurrentParser, pError);
  }

  png_structp lPNGStructPointer = 0;

  if(pError.empty()) {

    lPNGStructPointer = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (lPNGStructPointer == 0) {
      addError(std::string("Error saving ") + pFilename, aCurrentParser, pError);
    }
  }

  png_infop lPNGInfoPointer = 0;

  if(pError.empty()) {

    lPNGInfoPointer = png_create_info_struct(lPNGStructPointer);

    if (lPNGInfoPointer == 0) {
      addError(std::string("Error saving ") + pFilename, aCurrentParser, pError);
      png_destroy_write_struct(&lPNGStructPointer, 0);
    }
  }

  if(pError.empty()) {
    png_init_io(lPNGStructPointer, lFilePointer);
  }

  if(pError.empty()) {

    const int lBitDepth = 8;

    png_set_IHDR(lPNGStructPointer,
                 lPNGInfoPointer,
                 aWidth,
                 aHeight,
                 lBitDepth,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    const int  lBytesPerLine = 3*aWidth;
    png_bytepp lRowPointers  = new png_bytep[aHeight];

    lRowPointers[0] = aBufferImage;
    for(int i=1; i<aHeight; ++i) {
      lRowPointers[i] = lRowPointers[i-1] + lBytesPerLine;
    }

    png_write_info (lPNGStructPointer, lPNGInfoPointer);
    png_write_image(lPNGStructPointer, lRowPointers);
    png_write_end  (lPNGStructPointer, lPNGInfoPointer);

    delete [] lRowPointers;

    png_destroy_write_struct(&lPNGStructPointer, &lPNGInfoPointer);
  }

  if (lFilePointer != 0) {
    fclose(lFilePointer);
  }
}
#endif // #ifdef GLV_USE_PNG
