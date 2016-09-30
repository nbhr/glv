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

#ifndef WINDOWQT_H
#define WINDOWQT_H

#include "ViewManager.h"
#include "WindowGLV.h"

#include "qgl.h"
#include "qmainwindow.h"

class QPopupMenu;

class GLVOpenGLWidget : public QGLWidget
{
public:

  GLVOpenGLWidget         (QWidget*     pParent,
                           ViewManager& pViewManager);
  virtual ~GLVOpenGLWidget();

  virtual void paintGL    ();

  virtual void resizeGL   (int          pWidth,
                           int          pHeight);

  void         setRootMenu(QPopupMenu*  pRootMenu);

private:

  // Block the use of those
  GLVOpenGLWidget();
  GLVOpenGLWidget(const GLVOpenGLWidget&);
  GLVOpenGLWidget& operator=(const GLVOpenGLWidget&);

  void mouseMoveEvent   (QMouseEvent* pEvent);
  void mousePressEvent  (QMouseEvent* pEvent);
  void mouseReleaseEvent(QMouseEvent* pEvent);
  void wheelEvent       (QWheelEvent* pEvent);

  int          aMouseState;
  QPopupMenu*  aRootMenu;
  ViewManager& aViewManager;

};

// Qt toolkit implementation of a opengl render window
// Only one instance is permitted (see Window)
class WindowQt : public QMainWindow, public WindowGLV
{
  Q_OBJECT;

public:

  static WindowQt&  getInstance();

  WindowQt();
  virtual ~WindowQt();

private:

  // Block the use of those
  WindowQt(const WindowQt&);
  WindowQt& operator=(const WindowQt&);

  void keyPressEvent(QKeyEvent*   pEvent);
  void timerEvent   (QTimerEvent* pEvent);

  GLVOpenGLWidget* aOpenGLWidget;
  QPopupMenu*      aRootMenu;

private slots:

  void menu_Config_Axes      ();
  void menu_Config_Background();
  void menu_Config_Defaults  ();
  void menu_Config_Grid      ();
  void menu_Config_Lighting  ();
  void menu_Config_SaveGLRC  ();

  void menu_File_Clear();
  void menu_File_Dump ();
  void menu_File_Exit ();
  void menu_File_Open ();

  void menu_View_Add        ();
  void menu_View_Back       ();
  void menu_View_Bottom     ();
  void menu_View_BoundingBox();
  void menu_View_EchoView   ();
  void menu_View_EchoViews  ();
  void menu_View_Fast       ();
  void menu_View_Front      ();
  void menu_View_Full       ();
  void menu_View_Left       ();
  void menu_View_Next       ();
  void menu_View_Previous   ();
  void menu_View_Reset      ();
  void menu_View_Right      ();
  void menu_View_Top        ();

};

#endif // WINDOWQT_H

