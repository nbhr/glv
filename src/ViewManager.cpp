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

#include "ViewManager.h"
#include "BoundingBox.h"
#include "glinclude.h"
#include "glut_utils.h"
#include "Parser.h"
#include "RenderParameters.h"
#include "string_utils.h"
#include "Tile.h"

#include <iostream>
#include <string>
#ifndef WIN32
#include <unistd.h>
#endif

// Constructor; initialize the viewport and load preferences
ViewManager::ViewManager()
  : aAxes          (),
    aCurrentView   (),
    aGraphicData   (),
    aGrid          (),
    aLastMouseX    (0),
    aLastMouseY    (0),
    aMoveMode      (mouseMove_none),
    aNewViewAdded  (false),
    aStatusMessages(),
    aTitle         (),
    aUserSettings  (),
    aViewIndex     (-1),
    aViews         (),
    aWindowHeight  (600),
    aWindowWidth   (800)
{
  // Read the .glvrc file.  If it's not existing; it is created
  char* lHomeDir = getenv("HOME");

  if(lHomeDir) {
    std::string lConfigFile(lHomeDir);
    lConfigFile += "/.glvrc";

    std::string lError;
    int lCode = aUserSettings.loadFile(lConfigFile, lError);
    if(lCode == 2) {
      // No such file, write a default file
      std::string lIgnoreError;
      aUserSettings.saveFile(lConfigFile, lIgnoreError);
    }
    else if(lCode == 1) {
      // Format error
      std::cerr << lError << std::endl;
    }
  }

  // Initialise the axes once and for all
  Parser      lParser;
  std::string lError;

  aAxes.addCommand("arrow_colored", "0 0 0 0.5 0 0 1 0 0 0.5 0 0 0.05 10", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("arrow_colored", "0 0 0 0 0.5 0 0 1 0 0 0.5 0 0.05 10", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("arrow_colored", "0 0 0 0 0 0.5 0 0 1 0 0 0.5 0.05 10", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("glcolor", "1 0 0", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("text", "1.02 0.02 0.02 fixed13 \"X\"", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("glcolor", "0 1 0", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("text", "0.02 1.02 0.02 fixed13 \"Y\"", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("glcolor", "0 0 1", lParser, lError);
  GLV_ASSERT(lError.empty());
  aAxes.addCommand("text", "0.02 0.02 1.02 fixed13 \"Z\"", lParser, lError);
  GLV_ASSERT(lError.empty());

  // Initialise the grid once and for all

  const std::string lZero ("0 ");
  const std::string lOne  ("1 ");
  const std::string lMOne ("-1 ");
  const std::string lColor("0.3 0.3 0.3 ");
  char              lBuffer[32];
  const float       lDelta = 1.0f/16.0f;

  for (int i=-16; i<=16; ++i) {

    sprintf(lBuffer, "%f ", static_cast<float>(i)*lDelta);

    const std::string lI(lBuffer);

    aGrid.addCommand("line_colored", trimString(lI    + lZero + lMOne + lColor + lI   + lZero + lOne + lColor, " "), lParser, lError);
    GLV_ASSERT(lError.empty());
    aGrid.addCommand("line_colored", trimString(lMOne + lZero + lI    + lColor + lOne + lZero + lI   + lColor, " "), lParser, lError);
    GLV_ASSERT(lError.empty());
  }
}

// Destructor
ViewManager::~ViewManager()
{}

void ViewManager::addView(const View& pView)
{
  aCurrentView = pView;
  aViews.push_back(pView);

  int lNbViews = static_cast<int>(aViews.size());

  aViewIndex    = lNbViews - 1;
  aNewViewAdded = true;

  char lMessage[128];
  sprintf(lMessage,"Adding view %d", lNbViews);
  addStatusMessage(lMessage);
}

// Add a status message. (temporary text in the window)
void ViewManager::addStatusMessage(const std::string& pMessage)
{
  if(aStatusMessages.size() == 5) {
    aStatusMessages.erase(aStatusMessages.begin());
  }
  aStatusMessages.push_back(pMessage);
}

// Clear all status messages
void ViewManager::clearStatusMessages()
{
  aStatusMessages.clear();
}

// prepare the display and renders the scene
void ViewManager::display(const Tile& pTile)
{
  // Clear the window
  glClearColor(aUserSettings.aBackgroundR,
               aUserSettings.aBackgroundG,
               aUserSettings.aBackgroundB,
               0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Renders the background
  pTile.initViewport();

  pTile.initOrtho2DMatrix();

  glMatrixMode  (GL_MODELVIEW);
  glLoadIdentity();

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable  (GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  GLV_ASSERT(aUserSettings.aBackgroundMode >= 0);
  GLV_ASSERT(aUserSettings.aBackgroundMode <  5);

  if(aUserSettings.aBackgroundMode < 3) {
    // Plain color
  }
  else if(aUserSettings.aBackgroundMode == 3) {
    glBegin(GL_QUADS);
    glColor3f (0.7f, 0.7f, 0.7f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);
    glColor3f (0.4f, 0.4f, 0.4f);
    glVertex2f(1.0f, 1.0f);
    glColor3f (0.3f, 0.3f, 0.3f);
    glVertex2f(1.0f, 0.0f);
    glEnd();
  }
  else if(aUserSettings.aBackgroundMode == 4) {
    glBegin(GL_QUADS);
    glColor3f (0.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glColor3f (0.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);
    glColor3f (0.3f, 0.3f, 0.3f);
    glVertex2f(1.0f, 1.0f);
    glColor3f (0.2f, 0.2f, 0.2f);
    glVertex2f(1.0f, 0.0f);
    glEnd();
  }

  glDepthMask(GL_TRUE);
  glEnable   (GL_DEPTH_TEST);

  glMatrixMode  (GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode  (GL_MODELVIEW);
  glLoadIdentity();

  // Render the grid, axes, graphic data
  render(pTile);

  // Reset openGL attribute state
  glPopAttrib();
}

// Callback: Called when the scene has to be rendered.
void ViewManager::displayCallback()
{
  display(Tile(aWindowWidth, aWindowHeight));
}

// Return the graphic data structure; used to load input data
GraphicData& ViewManager::getGraphicData()
{
  return aGraphicData;
}

// Return the user settings structure
UserSettings& ViewManager::getUserSettings()
{
  return aUserSettings;
}

// Callback: Called when the app is idle.
// We check if there is new data available.
// If so, we return true (that will tell the window to redraw)
bool ViewManager::idleCallback()
{
  const bool lNewDataParsed = aGraphicData.timerCallback();

  // Change aCurrentView only if there is new data parsed
  // and that it wasn't a new view
  if (lNewDataParsed && !aNewViewAdded) {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox());
  }

  aNewViewAdded = false;

  return lNewDataParsed;
}

// Callback: Called when the keyboard is used
bool ViewManager::keyboardCallback(unsigned char pKey)
{
  if(pKey == '1') {
    menuCallback("view_reset");
  }
  else if(pKey == '2') {
    menuCallback("view_top");
  }
  else if(pKey == '3') {
    menuCallback("view_bottom");
  }
  else if(pKey == '4') {
    menuCallback("view_front");
  }
  else if(pKey == '5') {
    menuCallback("view_back");
  }
  else if(pKey == '6') {
    menuCallback("view_left");
  }
  else if(pKey == '7') {
    menuCallback("view_right");
  }
  else if(static_cast<int>(pKey) == 17) {
    // control-q
    menuCallback("general_exit");
  }
  else if(pKey == 'F') {
    menuCallback("view_full");
  }
  else if(pKey == 'D') {
    menuCallback("view_bounding_box");
  }
  else if(pKey == 'S') {
    menuCallback("view_fast");
  }
  else if(pKey == 'V') {
    menuCallback("view_echo");
  }
  else if(pKey == '~') {
    menuCallback("view_bookmark");
  }
  else if(pKey == '>') {
    menuCallback("view_next");
  }
  else if(pKey == '<') {
    menuCallback("view_previous");
  }
  else if(pKey == 'G') {
    menuCallback("config_grid");
  }
  else if(pKey == 'A') {
    menuCallback("config_axes");
  }
  else if(pKey == 'B') {
    menuCallback("config_background");
  }
  else if(pKey == 'L') {
    menuCallback("config_lighting");
  }
  return true;
}

// Callback : Called when special keys on the keyboard are used
bool ViewManager::keyboardSpecialCallback(int pKey)
{
  return true;
}

// Callback: menus commands with "event" string
bool ViewManager::menuCallback(const char* pMenuEvent)
{
  std::string lEvent(pMenuEvent);

  if(lEvent == "general_reset") {
    aGraphicData.reset();
    aViews.clear();
    aViewIndex   = -1;
    aCurrentView = View();
  }
  else if(lEvent == "general_dump_characteristics") {
    aGraphicData.dumpCharacteristics(std::cerr, "");
    std::cerr.flush();
  }
  else if(lEvent == "general_exit") {
    // The exit is handled in WindowQt/WindowGLUT,
    // after general_exit has processed
  }
  else if(lEvent == "config_grid") {
    aUserSettings.aFlagGrid = !aUserSettings.aFlagGrid;
    if(aUserSettings.aFlagGrid) {
      addStatusMessage("Grid display enabled");
    }
    else {
      addStatusMessage("Grid display disabled");
    }
  }
  else if(lEvent == "config_axes") {
    aUserSettings.aFlagAxes = !aUserSettings.aFlagAxes;
    if(aUserSettings.aFlagAxes) {
      addStatusMessage("Axes display enabled");
    }
    else {
      addStatusMessage("Axes display disabled");
    }
  }
  else if(lEvent == "config_background") {

    aUserSettings.aBackgroundMode = (aUserSettings.aBackgroundMode + 1) % 5;

    if(aUserSettings.aBackgroundMode == 0) {
      aUserSettings.aBackgroundR = 0.0f;
      aUserSettings.aBackgroundG = 0.0f;
      aUserSettings.aBackgroundB = 0.0f;
    }
    else if(aUserSettings.aBackgroundMode == 1) {
      aUserSettings.aBackgroundR = 1.0f;
      aUserSettings.aBackgroundG = 1.0f;
      aUserSettings.aBackgroundB = 1.0f;
    }
    else if(aUserSettings.aBackgroundMode == 2) {
      aUserSettings.aBackgroundR = 70.0f/255.0f;
      aUserSettings.aBackgroundG = 70.0f/255.0f;
      aUserSettings.aBackgroundB = 70.0f/255.0f;
    }
  }
  else if(lEvent == "config_lighting") {

    aUserSettings.aLightModel = (aUserSettings.aLightModel + 1) % 3;

    if(aUserSettings.aLightModel == 0) {
      addStatusMessage("Lighting model = Flat shading");
    }
    else if(aUserSettings.aLightModel == 1) {
      addStatusMessage("Lighting model = Light on camera");
    }
    else {
      GLV_ASSERT(aUserSettings.aLightModel == 2);
      addStatusMessage("Lighting model = Fixed light (see ~/.glvrc for settings)");
    }
  }
  else if(lEvent == "config_defaults") {
    aUserSettings = UserSettings();
  }
  else if(lEvent == "config_save") {

    char* lHomeDir = getenv("HOME");
    std::string lConfigFile(lHomeDir);
    lConfigFile += "/.glvrc";

    std::string lError;
    aUserSettings.saveFile(lConfigFile, lError);

    if(!lError.empty()) {
      addStatusMessage(lError);
      std::cerr << lError << std::endl;
    }
  }
  else if(lEvent == "view_reset") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox());
  }
  else if(lEvent == "view_top") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox(),
                        Vector3D(0.0f, 1.0f,  0.0f),
                        Vector3D(0.0f, 0.0f, -1.0f));
  }
  else if(lEvent == "view_bottom") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox(),
                        Vector3D(0.0f, -1.0f,  0.0f),
                        Vector3D(0.0f,  0.0f, -1.0f));
  }
  else if(lEvent == "view_left") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox(),
                        Vector3D(-1.0f, 0.0f, 0.0f),
                        Vector3D( 0.0f, 1.0f, 0.0f));
  }
  else if(lEvent == "view_right") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox(),
                        Vector3D(1.0f, 0.0f, 0.0f),
                        Vector3D(0.0f, 1.0f, 0.0f));
  }
  else if(lEvent == "view_front") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox(),
                        Vector3D(0.0f, 0.0f, 1.0f),
                        Vector3D(0.0f, 1.0f, 0.0f));
  }
  else if(lEvent == "view_back") {
    aCurrentView = View(aGraphicData.getGlobalBoundingBox(),
                        Vector3D(0.0f, 0.0f, -1.0f),
                        Vector3D(0.0f, 1.0f, 0.0f));
  }
  else if(lEvent == "view_fullscreen") {
    glutFullScreen();
  }
  else if(lEvent == "view_full") {
    aUserSettings.aSimplicationMode = UserSettings::simplificationMode_none;
  }
  else if(lEvent == "view_bounding_box") {
    aUserSettings.aSimplicationMode = UserSettings::simplificationMode_bounding_box;
  }
  else if(lEvent == "view_fast") {
    aUserSettings.aSimplicationMode = UserSettings::simplificationMode_fast;
  }
  else if(lEvent == "view_echo") {
    aCurrentView.dump(std::cerr);
    aCurrentView.dump(*this);
  }
  else if(lEvent == "view_echo_all") {

    int lNbViews = static_cast<int>(aViews.size());

    if(lNbViews == 0) {
      std::cerr << "No user defined views" << std::endl;
      addStatusMessage("No user defined views");
    }
    else {

      std::vector<View>::const_iterator       lViewIter    = aViews.begin();
      const std::vector<View>::const_iterator lViewIterEnd = aViews.end  ();

      while (lViewIter != lViewIterEnd) {
        lViewIter->dump(std::cerr);
        ++lViewIter;
      }
    }
  }
  else if(lEvent == "view_next") {

    int lNbViews = static_cast<int>(aViews.size());

    if(lNbViews > 0) {

      aViewIndex = (aViewIndex + 1) % lNbViews;

      GLV_ASSERT(aViewIndex >= 0);
      GLV_ASSERT(aViewIndex <  lNbViews);
      aCurrentView = aViews[aViewIndex];

      char lMessage[128];
      sprintf(lMessage,"Using view %d/%d", aViewIndex+1, lNbViews);
      addStatusMessage(lMessage);
    }
    else {
      addStatusMessage("No user defined view");
    }
  }
  else if(lEvent == "view_previous") {

    int lNbViews = static_cast<int>(aViews.size());

    if(lNbViews > 0) {

      if (aViewIndex == -1) {
        aViewIndex = 1;
      }

      aViewIndex = (aViewIndex + lNbViews - 1) % lNbViews;

      GLV_ASSERT(aViewIndex >= 0);
      GLV_ASSERT(aViewIndex <  lNbViews);
      aCurrentView = aViews[aViewIndex];

      char lMessage[128];
      sprintf(lMessage,"Using view %d/%d", aViewIndex+1, lNbViews);
      addStatusMessage(lMessage);
    }
    else {
      addStatusMessage("No user defined view");
    }
  }
  else if(lEvent == "view_bookmark") {

    addView(aCurrentView);

  }
  else {
    std::cerr << "Unhandled MENU " << pMenuEvent << std::endl;
    GLV_ASSERT(false);
  }

  return true;
}

// Callback: Mouse button.  Since the mouse's button 2 is linked
// to the contextual menus, we won't get callbacks for pButton=2
// pKeyState :   0x1 shift     0x2 control     0x4  alt
bool ViewManager::mouseButtonCallback(int pButton, int pState, int pKeyState, int pX, int pY)
{
  GLV_ASSERT(pButton != 2);

  clearStatusMessages();

  pY = aWindowHeight - pY;

  // Disable all modes if the mouse button is released
  aMoveMode = mouseMove_none;

  if(!pState) {

    // Mouse button is pressed:
    if(pButton == 0 && pKeyState & 0x1) {
      aMoveMode = mouseMove_zoom;
    }
    else if(pButton == 1 || (pButton == 0 && pKeyState & 0x2)) {
      aMoveMode = mouseMove_translate;
    }
    else if (pButton == 0){
      aMoveMode = mouseMove_rotate;
    }
    else if(pButton == 4) {
      aCurrentView.scaleFOV(0.9f);
    }
    else if(pButton == 3) {
      aCurrentView.scaleFOV(1.0f/0.9f);
    }
  }

  // If the user used the mouse wheel; we want the state to
  // stay set at mouseMove_zoom
  // This way; in graphic simplification mode; the displays stays simplified
  // between successive mouse-wheel usages.
  if(pButton == 3 || pButton == 4) {
    aMoveMode = mouseMove_zoom;
  }

  aLastMouseX = pX;
  aLastMouseY = pY;

#ifndef WIN32
  usleep(1);
#endif

  return true;
}

// Callback: Mouse motion. Compute the camera movement from it
bool ViewManager::mouseMotionCallback(int pX, int pY)
{
  pY = aWindowHeight - pY;

  if(aMoveMode == mouseMove_rotate) {

    aCurrentView.rotate(-0.01* static_cast<float>(pX-aLastMouseX), 0.01* static_cast<float>(pY-aLastMouseY));

  }
  else if(aMoveMode == mouseMove_translate) {

    aCurrentView.translate(static_cast<float>(pX-aLastMouseX)/static_cast<float>(aWindowWidth),
                           static_cast<float>(pY-aLastMouseY)/static_cast<float>(aWindowHeight));

  }
  else if(aMoveMode == mouseMove_zoom) {

    aCurrentView.scaleFOV(1.0f+(0.01f* static_cast<float>(pY-aLastMouseY)));

  }
  // Mousewheel processing is done is the mouseButtonCallback

  aLastMouseX = pX;
  aLastMouseY = pY;

  return true;
}

// Callback: Called when the window was resized
void ViewManager::reshapeCallback(int pWidth, int pHeight)
{
  aWindowWidth  = pWidth;
  aWindowHeight = pHeight;
}

void ViewManager::setTitle(const std::string& pTitle)
{
  aTitle = pTitle;
}

void ViewManager::drawAxes()
{
  const BoundingBox lBoundingBox = aGraphicData.getGlobalBoundingBox();
  const Vector3D&   lCenter      = lBoundingBox.getCenter();
  float             lLength      = lBoundingBox.getCircumscribedSphereRadius();

  if(lLength == 0.0f) {
    lLength = 1.2f;
  }
  else {
    lLength *= 1.2f;
  }

  glPushMatrix();

  glTranslatef(lCenter.x(), lCenter.y(), lCenter.z());
  glScalef    (lLength, lLength, lLength);

  RenderParameters lParams;
  aAxes.render(lParams);

  glPopMatrix();
}

void ViewManager::drawGrid()
{
  // Disable depth information for grid rendering; they will always be "in the background
  //  and then never intrude over the object data
  glDisable  (GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  const BoundingBox lBoundingBox = aGraphicData.getGlobalBoundingBox();
  const Vector3D&   lCenter      = lBoundingBox.getCenter();
  float             lLength      = lBoundingBox.getCircumscribedSphereRadius();

  glPushMatrix();

  glTranslatef(lCenter.x(), lCenter.y() - lLength, lCenter.z());

  if(lLength == 0.0f) {
    lLength = 4.0f;
  }
  else {
    lLength *= 4.0f;
  }

  glScalef(lLength, lLength, lLength);

  RenderParameters lParams;
  aGrid.render(lParams);

  glPopMatrix();

  // Reactivate Z buffer
  glDepthMask(GL_TRUE);
  glEnable   (GL_DEPTH_TEST);
}

void ViewManager::drawMouseMovementIndicators()
{
  // Viewpoint center is shown using a small orange axe; rendered only if
  // the mouse button is pressed

  aCurrentView.drawCenter();
}

void ViewManager::drawTextLayer(const Tile& pTile)
{
  pTile.initOrtho2DMatrix();

  glMatrixMode  (GL_MODELVIEW);
  glPushMatrix  ();
  glLoadIdentity();

  // Render overlay text
  glColor3f(0.5f, 0.5f, 0.5f);
  std::string lStringText;

  std::vector<std::string>::const_iterator       lMessageIter    = aStatusMessages.begin();
  const std::vector<std::string>::const_iterator lMessageIterEnd = aStatusMessages.end  ();

  while (lMessageIter != lMessageIterEnd) {
    lStringText += *lMessageIter + "\n";
    ++lMessageIter;
  }

  drawText(lStringText, 0.05f, 0.05f, 0.95f, 0.95f, 12, GLUT_BITMAP_8_BY_13, true, false);

  if(!aTitle.empty()) {
    drawText(aTitle, 0.0f, 0.0f, 1.0f, 1.0f, 24, GLUT_BITMAP_TIMES_ROMAN_24, true, true);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix ();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix ();
}

// Render the grid,axes an graphical data
void ViewManager::render(const Tile& pTile)
{
  aCurrentView.initCamera(pTile);

  if(aUserSettings.aFlagGrid) {
    drawGrid();
  }

  if(aMoveMode != mouseMove_none) {
    drawMouseMovementIndicators();
  }

  // LIGHT
  if(aUserSettings.aLightModel) {
    setupLighting();
  }

  // Default settings (commands may override this)
  glLightModeli  (GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
  glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
  glEnable       (GL_COLOR_MATERIAL);
  glColor3f      (1.0f,1.0f,1.0f);

  // Object rendering. only if the mouse button is not down OR graphic simplification
  // is disabled.
  if(aMoveMode == mouseMove_none || aUserSettings.aSimplicationMode == UserSettings::simplificationMode_none) {
    RenderParameters lParams;
    lParams.aRenderMode = RenderParameters::renderMode_full;
    aGraphicData.render(lParams);
  }
  else if(aMoveMode != mouseMove_none && aUserSettings.aSimplicationMode == UserSettings::simplificationMode_fast) {
    RenderParameters lParams;
    lParams.aRenderMode = RenderParameters::renderMode_fast;
    aGraphicData.render(lParams);
  }
  else if(aMoveMode != mouseMove_none && aUserSettings.aSimplicationMode == UserSettings::simplificationMode_bounding_box) {
    RenderParameters lParams;
    lParams.aRenderMode = RenderParameters::renderMode_bounding_box;
    aGraphicData.render(lParams);
  }

  glDisable(GL_COLOR_MATERIAL);

  if(aUserSettings.aLightModel) {
    glDisable(GL_LIGHTING);
  }

  if(aUserSettings.aFlagAxes) {
    drawAxes();
  }

  // Text drawing:
  drawTextLayer(pTile);
}

void ViewManager::setupLighting()
{
  glEnable(GL_LIGHTING);

  // 70% of the object lighting is diffuse
  // 30% is ambient.
  GLfloat light_ambient[]  = {0.3f, 0.3f, 0.3f, 1.0f};
  GLfloat light_diffuse[]  = {0.7f, 0.7f, 0.7f, 1.0f};
  GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat light_position[] = {1.0f, 1.0f, 1.0f, 0.0f};

  light_ambient[0] *= aUserSettings.aLightAmbient;
  light_ambient[1] *= aUserSettings.aLightAmbient;
  light_ambient[2] *= aUserSettings.aLightAmbient;

  if(aUserSettings.aLightModel == 1) {
    // Light is camera
    Vector3D lLightPosition = aCurrentView.getPosition();
    light_position[0] = lLightPosition.x();
    light_position[1] = lLightPosition.y();
    light_position[2] = lLightPosition.z();
  }
  else {
    // Light is fixed
    light_position[0] = aUserSettings.aLightPosition.x();
    light_position[1] = aUserSettings.aLightPosition.y();
    light_position[2] = aUserSettings.aLightPosition.z();
    light_position[3] = aUserSettings.aLightPositionLocal;
  }

  glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_LIGHT0);
}
