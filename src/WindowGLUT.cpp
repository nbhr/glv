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

#include "WindowGLUT.h"
#include "assert_glv.h"

#ifndef WIN32
#include <unistd.h>
#endif


// IDs for menu items
#define MENU_VIEW_RESET_ID 0
#define MENU_VIEW_TOP 1
#define MENU_VIEW_LEFT 2
#define MENU_VIEW_FRONT 3
#define MENU_VIEW_BOTTOM 4
#define MENU_VIEW_RIGHT 5
#define MENU_VIEW_BACK 6
#define MENU_VIEW_FULLSCREEN 10
#define MENU_VIEW_ECHO 11
#define MENU_VIEW_NEXT 12
#define MENU_VIEW_PREVIOUS 13
#define MENU_VIEW_FULL 14
#define MENU_VIEW_BOUNDINGBOX 15
#define MENU_VIEW_FAST 16
#define MENU_VIEW_BOOKMARK 17
#define MENU_VIEW_ECHO_ALL 18

#define MENU_CONFIG_AXES 0
#define MENU_CONFIG_GRID 1
#define MENU_CONFIG_BACKGROUND 2
#define MENU_CONFIG_LIGHTING 3
#define MENU_CONFIG_DEFAULTS 20
#define MENU_CONFIG_SAVE 21

#define MENU_GENERAL_RESET 0
#define MENU_GENERAL_DUMP_CHARACTERISTICS 1
#define MENU_GENERAL_EXIT 2

// GLUT callbacks initialized in constructor
//  All these functions will call the associated method
//  on the object identified in the singleton pointer
void displayFunc()
{
  WindowGLUT::getInstance().paintGL();
}

void mouseFunc(int p1,int p2,int p3,int p4)
{
  WindowGLUT::getInstance().mouse(p1,p2,p3,p4);
}

void motionFunc(int p3,int p4)
{
  WindowGLUT::getInstance().mouse(-1,-1,p3,p4);
}

void idlefunc()
{
  WindowGLUT::getInstance().idle();
}

void resizeFunc(int pWidth,int pHeight)
{
  GLV_ASSERT(pWidth > 0);
  GLV_ASSERT(pHeight > 0);

  WindowGLUT::getInstance().resizeGL(pWidth, pHeight);
}

void keyboardFunc(unsigned char pKey, int pX, int pY)
{
  WindowGLUT::getInstance().key(pKey);
}

void specialFunc(int pKey, int pX, int pY)
{
  WindowGLUT::getInstance().skey(pKey);
}

void mainMenuFunc(int pFuncId)
{
}

void viewMenuFunc(int pFuncId)
{
  WindowGLUT& lWindow = WindowGLUT::getInstance();

  if( pFuncId == MENU_VIEW_ECHO ) {
    lWindow.menu("view_echo");
  }
  else if( pFuncId == MENU_VIEW_ECHO_ALL ) {
    lWindow.menu("view_echo_all");
  }
  else if( pFuncId == MENU_VIEW_BOOKMARK ) {
    lWindow.menu("view_bookmark");
  }
  else if( pFuncId == MENU_VIEW_NEXT) {
    lWindow.menu("view_next");
  }
  else if( pFuncId == MENU_VIEW_PREVIOUS ) {
    lWindow.menu("view_previous");
  }
  else if( pFuncId == MENU_VIEW_FULL ) {
    lWindow.menu("view_full");
  }
  else if( pFuncId == MENU_VIEW_BOUNDINGBOX ) {
    lWindow.menu("view_bounding_box");
  }
  else if( pFuncId == MENU_VIEW_FAST ) {
    lWindow.menu("view_fast");
  }
  else if( pFuncId == MENU_VIEW_RESET_ID ) {
    lWindow.menu("view_reset");
  }
  else if( pFuncId == MENU_VIEW_TOP) {
    lWindow.menu("view_top");
  }
  else if( pFuncId == MENU_VIEW_BOTTOM) {
    lWindow.menu("view_bottom");
  }
  else if( pFuncId == MENU_VIEW_RIGHT) {
    lWindow.menu("view_right");
  }
  else if( pFuncId == MENU_VIEW_LEFT) {
    lWindow.menu("view_left");
  }
  else if( pFuncId == MENU_VIEW_FRONT) {
    lWindow.menu("view_front");
  }
  else if( pFuncId == MENU_VIEW_BACK) {
    lWindow.menu("view_back");
  }
  else if( pFuncId == MENU_VIEW_FULLSCREEN) {
    lWindow.menu("view_fullscreen");
  }
  else {
    GLV_ASSERT(false);
  }
}

void configMenuFunc(int pFuncId)
{
  WindowGLUT& lWindow = WindowGLUT::getInstance();

  if (pFuncId == MENU_CONFIG_AXES) {
    lWindow.menu("config_axes");
  }
  else if (pFuncId == MENU_CONFIG_GRID) {
    lWindow.menu("config_grid");
  }
  else if (pFuncId == MENU_CONFIG_BACKGROUND ) {
    lWindow.menu("config_background");
  }
  else if (pFuncId == MENU_CONFIG_LIGHTING) {
    lWindow.menu("config_lighting");
  }
  else if (pFuncId == MENU_CONFIG_DEFAULTS) {
    lWindow.menu("config_defaults");
  }
  else if (pFuncId == MENU_CONFIG_SAVE) {
    lWindow.menu("config_save");
  }
  else {
    GLV_ASSERT(false);
  }
}

void generalMenuFunc(int pFuncId)
{
  WindowGLUT& lWindow = WindowGLUT::getInstance();

  if (pFuncId == MENU_GENERAL_RESET) {
    lWindow.menu("general_reset");
  }
  else if (pFuncId == MENU_GENERAL_DUMP_CHARACTERISTICS) {
    lWindow.menu("general_dump_characteristics");
  }
  else if (pFuncId == MENU_GENERAL_EXIT) {
    lWindow.menu("general_exit");
    exit(0);
  }
  else {
    GLV_ASSERT(false);
  }
}

// WindowGLUT methods:

// Static method
// Get the only instance of type WindowGLUT
// One and only one object must be created
// before calling this static member function
WindowGLUT& WindowGLUT::getInstance()
{
  // Only one Window can be used
  GLV_ASSERT(aSingleton != 0);

  WindowGLUT* lSingleton = dynamic_cast<WindowGLUT*>(aSingleton);

  // This should never happen since we choose
  // the window type at compilation time
  GLV_ASSERT(lSingleton != 0);

  return *lSingleton;
}

WindowGLUT::WindowGLUT()
  : WindowGLV()
{
  // Only one Window can be used
  GLV_ASSERT(aSingleton == this);

  // Register GLUT callbacks and initialize GLUT settings
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(800,600);
  glutInitWindowPosition(100,100);
  glutCreateWindow("glv - OpenGL Viewer");
  glutDisplayFunc(displayFunc);
  glutReshapeFunc(resizeFunc);
  glutMouseFunc(mouseFunc);
  glutMotionFunc(motionFunc);
  glutKeyboardFunc(keyboardFunc);
  glutSpecialFunc(specialFunc);
  glutIdleFunc(idlefunc);

  // Setup menus
  int lMenuViewId = glutCreateMenu(viewMenuFunc);
  glutAddMenuEntry("Reset view (1)",MENU_VIEW_RESET_ID);
  glutAddMenuEntry("Top (2)",MENU_VIEW_TOP);
  glutAddMenuEntry("Bottom (3)",MENU_VIEW_BOTTOM);
  glutAddMenuEntry("Left (4)",MENU_VIEW_LEFT);
  glutAddMenuEntry("Right (5)",MENU_VIEW_RIGHT);
  glutAddMenuEntry("Front (6)",MENU_VIEW_FRONT);
  glutAddMenuEntry("Back (7)",MENU_VIEW_BACK);
  glutAddMenuEntry("---------------",-1);
  glutAddMenuEntry("\"Full\" mode (F)",MENU_VIEW_FULL);
  glutAddMenuEntry("\"Bounding Box\" mode (D)",MENU_VIEW_BOUNDINGBOX);
  glutAddMenuEntry("\"Fast\" mode (S)",MENU_VIEW_FAST);
  glutAddMenuEntry("---------------",-1);
  glutAddMenuEntry("Add view to user-defined views (~)",MENU_VIEW_BOOKMARK);
  glutAddMenuEntry("Echo current settings (V)",MENU_VIEW_ECHO);
  glutAddMenuEntry("Echo all user defined views",MENU_VIEW_ECHO_ALL);
  glutAddMenuEntry("Next user-defined view (>)",MENU_VIEW_NEXT);
  glutAddMenuEntry("Previous user-defined view (<)",MENU_VIEW_PREVIOUS);

  int lMenuConfigId = glutCreateMenu(configMenuFunc);
  glutAddMenuEntry("Axes on/off (A)",MENU_CONFIG_AXES);
  glutAddMenuEntry("Grid on/off (G)",MENU_CONFIG_GRID);
  glutAddMenuEntry("Background color (B)",MENU_CONFIG_BACKGROUND);
  glutAddMenuEntry("Lighting model (L)",MENU_CONFIG_LIGHTING);
  glutAddMenuEntry("---------------",-1);
  glutAddMenuEntry("Reset to defaults",MENU_CONFIG_DEFAULTS);
  glutAddMenuEntry("Save to ~/.glvrc",MENU_CONFIG_SAVE);

  int lMenuGeneralId = glutCreateMenu(generalMenuFunc);
  glutAddMenuEntry("Clear data",MENU_GENERAL_RESET);
  glutAddMenuEntry("Dump characteristics",MENU_GENERAL_DUMP_CHARACTERISTICS);
  glutAddMenuEntry("Exit program (ctrl-q)",MENU_GENERAL_EXIT);

  glutCreateMenu(mainMenuFunc);
  glutAddMenuEntry("openGL viewer",0);
  glutAddMenuEntry("-------------------",0);
  glutAddSubMenu("General",lMenuGeneralId);
  glutAddSubMenu("View",lMenuViewId);
  glutAddSubMenu("Config",lMenuConfigId);
  glutAddMenuEntry("-----Quick help-----",0);
  glutAddMenuEntry(" Left mouse= rotate",0);
  glutAddMenuEntry(" Center mouse= translate",0);
  glutAddMenuEntry(" Mouse wheel = Near/far",0);
  glutAddMenuEntry(" Shift-left = Zoom",0);
  glutAddMenuEntry(" ( glv.sourceforge.net )",0);

  // Connect right-click to menus
  glutAttachMenu(2);
}

WindowGLUT::~WindowGLUT()
{}

// Callback; transmit the message to the ViewManager
void WindowGLUT::idle()
{
  //  If the viewmanager want a redisplay; we refresh the display
  if(getViewManager().idleCallback()) {
    glutPostRedisplay();
  }
  else {
    // If we dont let the process sleep; the CPU will be maxed out
    // by GLUT idle func
#ifndef WIN32
    usleep(100);
#endif
  }
}

// Callback; transmit the message to the ViewManager
// and ask for a redisplay if necessary
void WindowGLUT::key(unsigned char pKey)
{
  if(getViewManager().keyboardCallback(pKey)) {
    glutPostRedisplay();
  }

  if(static_cast<int>(pKey) == 17) {
    exit(0);
  }
}

// Callback; transmit the message to the window manager
//  menu events are strings, generated from the GLUT menu id
void WindowGLUT::menu(const char* pMenuEvent)
{
  GLV_ASSERT(pMenuEvent != 0);

  if(getViewManager().menuCallback(pMenuEvent)) {
    glutPostRedisplay();
  }
}

// Callback; transmit the message to the ViewManager if
// the mouse is not moving, meaning we have a click event
void WindowGLUT::mouse(int pButton, int pState, int pX, int pY)
{
  // -1 is a special flag in our mouseMotionFunc
  // ... if pButton == -1; we are in motion; not click event
  if(pButton != -1) {
    int lModifiers = glutGetModifiers();
    if(getViewManager().mouseButtonCallback(pButton, pState, lModifiers, pX, pY)) {
      glutPostRedisplay();
    }
  }
  else if(getViewManager().mouseMotionCallback(pX, pY)) {
    glutPostRedisplay();
  }
}

// Callback; transmit the message to the ViewManager
//  After rendering, swap buffers (the window is double-buffered)
void WindowGLUT::paintGL()
{
  getViewManager().displayCallback();
  glutSwapBuffers();
}

// Callback; transmit the message to the ViewManager
//  Resize the openGL viewport information
void WindowGLUT::resizeGL(int pWidth, int pHeight)
{
  GLV_ASSERT(pWidth > 0);
  GLV_ASSERT(pHeight > 0);

  glViewport(0, 0, pWidth, pHeight);

  getViewManager().reshapeCallback(pWidth, pHeight);
}

// Callback; transmit the message to the ViewManager
// and ask for a redisplay if necessary
void WindowGLUT::skey(int pKey)
{
  if(getViewManager().keyboardSpecialCallback(pKey)) {
    glutPostRedisplay();
  }
}
