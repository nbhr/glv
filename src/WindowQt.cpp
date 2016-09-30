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

#include "WindowQt.h"
#include "glv_minibar.xpm"
#include "assert_glv.h"

#include "qfiledialog.h"
#include "qmenubar.h"
#include "qmessagebox.h"
#include "qpopupmenu.h"

#ifndef WIN32
#include <unistd.h>
#endif

GLVOpenGLWidget::GLVOpenGLWidget(QWidget*     pParent,
                                 ViewManager& pViewManager)
  : QGLWidget   (pParent),
    aMouseState (0),
    aRootMenu   (0),
    aViewManager(pViewManager)
{
  GLV_ASSERT(pParent != 0);
}

GLVOpenGLWidget::~GLVOpenGLWidget()
{}

// Callback; transmit the message to the ViewManager
//  After rendering, swap buffers (the window is double-buffered)
void GLVOpenGLWidget::paintGL()
{
  aViewManager.displayCallback();
  swapBuffers();
}

// Callback; transmit the message to the ViewManager
//  Resize the openGL viewport information
void GLVOpenGLWidget::resizeGL(int pWidth, int pHeight)
{
  GLV_ASSERT(pWidth > 0);
  GLV_ASSERT(pHeight > 0);

  glViewport(0, 0, pWidth, pHeight);

  aViewManager.reshapeCallback(pWidth, pHeight);
}

void GLVOpenGLWidget::setRootMenu(QPopupMenu* pRootMenu)
{
  GLV_ASSERT(pRootMenu != 0);

  aRootMenu = pRootMenu;
}

void GLVOpenGLWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
  GLV_ASSERT(pEvent != 0);

  if(aMouseState) {
    if(aViewManager.mouseMotionCallback(pEvent->x(),pEvent->y())) {
      repaint();
    }
  }
}

void GLVOpenGLWidget::mousePressEvent(QMouseEvent* pEvent)
{
  GLV_ASSERT(pEvent != 0);

  int button = static_cast<int>(pEvent->stateAfter());

  if(button & Qt::LeftButton) {
    button = 0;
  }
  else if(button & Qt::MidButton) {
    button = 1;
  }
  else if(button & Qt::RightButton) {
    if(aRootMenu) {
      aRootMenu->popup(pEvent->globalPos());
      return;
    }
    button = 2;
  }

  int key   = static_cast<int>(pEvent->stateAfter());
  int modif = 0;

  if(key & Qt::ShiftButton) {
    modif |= 0x1;
  }
  if(key & Qt::ControlButton) {
    modif |= 0x2;
  }
  if(key & Qt::AltButton) {
    modif |= 0x4;
  }
  if(aViewManager.mouseButtonCallback(button,0,modif,pEvent->x(),pEvent->y())) {
    repaint();
  }
  aMouseState = 1;
}

void GLVOpenGLWidget::mouseReleaseEvent(QMouseEvent* pEvent)
{
  GLV_ASSERT(pEvent != 0);

  int button = static_cast<int>(pEvent->state());

  if(button & Qt::LeftButton) {
    button = 0;
  }
  else if(button & Qt::MidButton) {
    button = 1;
  }
  else if(button & Qt::RightButton) {
    button = 2;
  }

  int key   = static_cast<int>(pEvent->stateAfter());
  int modif = 0;

  if(key & Qt::ShiftButton) {
    modif |= 0x1;
  }
  if(key & Qt::ControlButton) {
    modif |= 0x2;
  }
  if(key & Qt::AltButton) {
    modif |= 0x4;
  }
  if(aViewManager.mouseButtonCallback(button,1,modif,pEvent->x(),pEvent->y())) {
    repaint();
  }
  aMouseState = 0;
}

void GLVOpenGLWidget::wheelEvent(QWheelEvent* pEvent)
{
  GLV_ASSERT(pEvent != 0);

  if(pEvent->delta() > 0) {
    if(aViewManager.mouseButtonCallback(3,0,0,0,0)) {
      repaint();
    }
  }
  else if(pEvent->delta() < 0) {
    if(aViewManager.mouseButtonCallback(4,0,0,0,0)) {
      repaint();
    }
  }
  else {
    if(aViewManager.mouseButtonCallback(4,1,0,0,0)) {
      repaint();
    }
  }
}



// WindowQt methods:

// Static method
// Get the only instance of type WindowGLUT
// One and only one object must be created
// before calling this static member function
WindowQt& WindowQt::getInstance()
{
  // Only one Window can be used
  GLV_ASSERT(aSingleton != 0);

  WindowQt* lSingleton = dynamic_cast<WindowQt*>(aSingleton);

  // This should never happen since we choose
  // the window type at compilation time
  GLV_ASSERT(lSingleton != 0);

  return *lSingleton;
}

WindowQt::WindowQt()
  : WindowGLV    (),
    aOpenGLWidget(0),
    aRootMenu    (0)
{
  // Only one Window can be used
  GLV_ASSERT(aSingleton == this);

  aOpenGLWidget = new GLVOpenGLWidget(this, getViewManager());
  setCentralWidget(aOpenGLWidget);
  resize(800,600);
  setCaption("glv - OpenGL Viewer");

  startTimer(100);

  int lId;
  // File menu
  QPopupMenu* lFileMenu = new QPopupMenu();
  lFileMenu->insertItem("Open",this,SLOT(menu_File_Open()),Qt::CTRL+Qt::Key_O);
  lFileMenu->insertItem("Clear data",this,SLOT(menu_File_Clear()));
  lFileMenu->insertItem("Dump characteristics",this,SLOT(menu_File_Dump()));
  lFileMenu->insertSeparator();
  lFileMenu->insertItem("Exit",this,SLOT(menu_File_Exit()),Qt::CTRL+Qt::Key_Q);
  menuBar()->insertItem("File",lFileMenu);

  // View menu
  QPopupMenu* lViewMenu = new QPopupMenu();
  lViewMenu->insertItem("Reset view",this,SLOT(menu_View_Reset()),Qt::Key_1);
  lViewMenu->insertItem("Top",this,SLOT(menu_View_Top()),Qt::Key_2);
  lViewMenu->insertItem("Bottom",this,SLOT(menu_View_Bottom()),Qt::Key_3);
  lViewMenu->insertItem("Left",this,SLOT(menu_View_Left()),Qt::Key_4);
  lViewMenu->insertItem("Right",this,SLOT(menu_View_Right()),Qt::Key_5);
  lViewMenu->insertItem("Front",this,SLOT(menu_View_Front()),Qt::Key_6);
  lViewMenu->insertItem("Back",this,SLOT(menu_View_Back()),Qt::Key_7);
  lViewMenu->insertSeparator();
  lViewMenu->insertItem("\"Full\" mode",this,SLOT(menu_View_Full()),Qt::SHIFT+Qt::Key_F);
  lViewMenu->insertItem("\"Bounding box\" mode",this,SLOT(menu_View_BoundingBox()),Qt::SHIFT+Qt::Key_D);
  lViewMenu->insertItem("\"Fast\" mode",this,SLOT(menu_View_Fast()),Qt::SHIFT+Qt::Key_S);
  lViewMenu->insertSeparator();
  lViewMenu->insertItem("Add view to list",this,SLOT(menu_View_Add()),Qt::Key_AsciiTilde);
  lViewMenu->insertItem("Echo current view",this,SLOT(menu_View_EchoView()),Qt::SHIFT+Qt::Key_V);
  lViewMenu->insertItem("Echo all user defined views",this,SLOT(menu_View_EchoViews()));
  lViewMenu->insertItem("Next user defined view",this,SLOT(menu_View_Next()),Qt::Key_Greater);
  lViewMenu->insertItem("Previous user defined view",this,SLOT(menu_View_Previous()),Qt::Key_Less);
  menuBar()->insertItem("View",lViewMenu);

  // Config menu
  QPopupMenu* lConfigMenu = new QPopupMenu();
  lConfigMenu->insertItem("Axes",this,SLOT(menu_Config_Axes()),Qt::SHIFT+Qt::Key_A);
  lConfigMenu->insertItem("Grid",this,SLOT(menu_Config_Grid()),Qt::SHIFT+Qt::Key_G);
  lConfigMenu->insertItem("Background color",this,SLOT(menu_Config_Background()),Qt::SHIFT+Qt::Key_B);
  lConfigMenu->insertItem("Lighting model",this,SLOT(menu_Config_Lighting()),Qt::SHIFT+Qt::Key_L);
  lConfigMenu->insertSeparator();
  lConfigMenu->insertItem("Reset to defaults",this,SLOT(menu_Config_Defaults()));
  lConfigMenu->insertItem("Save to ~/.glrc",this,SLOT(menu_Config_SaveGLRC()));
  menuBar()->insertItem("Config",lConfigMenu);

  // Root menu
  aRootMenu = new QPopupMenu();
  lId = aRootMenu->insertItem("openGL viewer");
  aRootMenu->changeItem(lId,QPixmap((const char**)glv_minibar_xpm));
  aRootMenu->insertSeparator();
  aRootMenu->insertItem("General",lFileMenu);
  aRootMenu->insertItem("View",lViewMenu);
  aRootMenu->insertItem("Config",lConfigMenu);
  aRootMenu->insertSeparator();
  aRootMenu->insertItem(" Left mouse= rotate");
  aRootMenu->insertItem(" Center mouse= translate");
  aRootMenu->insertItem(" Mouse wheel = Near/far");
  aRootMenu->insertItem(" Shift-left = Zoom");
  aRootMenu->insertItem(" ( glv.sourceforge.net )");

  aOpenGLWidget->setRootMenu(aRootMenu);
}

WindowQt::~WindowQt()
{
  if (aOpenGLWidget != 0) {
    delete aOpenGLWidget;
  }
  if (aRootMenu != 0) {
    delete aRootMenu;
  }
}

// Callback; transmit the message to the ViewManager
void WindowQt::keyPressEvent(QKeyEvent* pEvent)
{
  GLV_ASSERT(pEvent != 0);

  if(getViewManager().keyboardCallback(pEvent->ascii())) {
    aOpenGLWidget->repaint();
  }
  // NOT IMPLEMENTED YET
  // if(getViewManager().keyboardSpecialCallback(....)) {
  //   aOpenGLWidget->repaint();
  // }
}

// Callback; transmit the message to the ViewManager
void WindowQt::timerEvent(QTimerEvent* pEvent)
{
  GLV_ASSERT(pEvent != 0);

  //  If the viewmanager want a redisplay; we refresh the display
  if(getViewManager().idleCallback()) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_Config_Axes()
{
  if(getViewManager().menuCallback("config_axes")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_Config_Background()
{
  if(getViewManager().menuCallback("config_background")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_Config_Defaults()
{
  if(getViewManager().menuCallback("config_defaults")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_Config_Grid()
{
  if(getViewManager().menuCallback("config_grid")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_Config_Lighting()
{
  if(getViewManager().menuCallback("config_lighting")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_Config_SaveGLRC()
{
  if(getViewManager().menuCallback("config_save")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_File_Clear()
{
  if(getViewManager().menuCallback("general_reset")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_File_Dump()
{
  if(getViewManager().menuCallback("general_dump_characteristics")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_File_Exit()
{
  getViewManager().menuCallback("general_exit");
  close();
}

void WindowQt::menu_File_Open()
{
  QString lFile = QFileDialog::getOpenFileName("",
                                               "Formatted input (*.gl *.gl.gz);;Formatted input (*)",
                                               this,
                                               "",
                                               "Select the input file");
  if(!lFile.isEmpty()) {
    std::string lError;
    getViewManager().getGraphicData().readDataFile(lFile.ascii(), lError);
    if(!lError.empty()) {
      QMessageBox::warning(this,"Error",lError.c_str());
    }
  }
  aOpenGLWidget->repaint();
}

void WindowQt::menu_View_Add()
{
  if(getViewManager().menuCallback("view_bookmark")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Back()
{
  if(getViewManager().menuCallback("view_back")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Bottom()
{
  if(getViewManager().menuCallback("view_bottom")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_BoundingBox()
{
  if(getViewManager().menuCallback("view_bounding_box")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_EchoView()
{
  if(getViewManager().menuCallback("view_echo")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_EchoViews()
{
  if(getViewManager().menuCallback("view_echo_all")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Fast()
{
  if(getViewManager().menuCallback("view_fast")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Front()
{
  if(getViewManager().menuCallback("view_front")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Full()
{
  if(getViewManager().menuCallback("view_full")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Left()
{
  if(getViewManager().menuCallback("view_left")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Next()
{
  if(getViewManager().menuCallback("view_next")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Previous()
{
  if(getViewManager().menuCallback("view_previous")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Reset()
{
  if(getViewManager().menuCallback("view_reset")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Right()
{
  if(getViewManager().menuCallback("view_right")) {
    aOpenGLWidget->repaint();
  }
}

void WindowQt::menu_View_Top()
{
  if(getViewManager().menuCallback("view_top")) {
    aOpenGLWidget->repaint();
  }
}
