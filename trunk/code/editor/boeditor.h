/***************************************************************************
                          boeditor.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef BOEDITOR_H 
#define BOEDITOR_H 
 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for QT
#include <qprinter.h>
#include <qpainter.h>

// include files for KDE 
#include <kapp.h> 
#include <ktmainwindow.h> 
#include <kaccel.h>
#include <kiconloader.h>
#include <kmsgbox.h>

// application specific includes
#include "../common/msgData.h"

#include "editorField.h"
#include "mainView.h"

/**
  * This Class is the base class for your application. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar. For the main view, an instance of class BoEditorApp is
  * created which creates your view.
  */
class BoEditorApp : public KTMainWindow 
{

	Q_OBJECT 

public: 
  /** construtor */
  BoEditorApp(char *filename=0l); 
  /** destructor */
  ~BoEditorApp();

 public slots:
  /** open a new application window by creating a new instance of BoEditorApp */
  void slotFileNewWindow();
  /** clears the document in the actual view to reuse it as the new document */
  void slotFileNew();
  /** open a file and load it into the document*/
  void slotFileOpen();
  /** opens a file from the recent files menu */
  void slotFileOpenRecent(int id_);
  /** save a document */
  void slotFileSave();
  /** save a document by a new filename*/
  void slotFileSaveAs();
  /** asks for saving if the file is modified, then closes the actual file and window*/
  void slotFileClose();



  /** enable menuentries/toolbar items */
   void enableCommand(int id_);
   /** disable menuentries/toolbar items */
   void disableCommand(int id_);
   /** switch argument for slot selection by menu or toolbar ID */
   void commandCallback(int id_);
   /** switch argument for Statusbar help entries on slot selection */
   void statusCallback(int id_);

protected:

  /** init() sets the Application title, reads the config file entries
    * by calling readOptions() and calls the initXX functions to set up
    * the main view items
    */
  void init(char *filename=0l);  
  /** initMenuBar creates the menu_bar and inserts the menuitems */
  void initMenuBar(); 
  /** this creates the toolbars. Change the toobar look and add more in this
    * function 
    */ 
  void initToolBars();
  /** setup the statusbar */
  void initStatusBar(); 
  /** setup the mainview*/
  void initView();
  /** resizeEvent for the main view */
  virtual void resizeEvent(QResizeEvent *evt);
  /** add filename to the recentList */
  void addRecentFile(const char *filename);
  /** method asks the modified flag and creates a modified dialog for saving */
  void dlgModified();
  /** overloaded for Message box on last window exit */
  bool queryExit();
  /** save the app-specific options on slotAppExit or by an Options dialog */
  //void saveOptions();
  /** read the app-specific options on init() or by an Options dialog */
  //void readOptions();

  /** receive Server message */
  void handleDialogMessage(bosonMsgTag, int, bosonMsgData *);
  /** 3rd layer : receiving playing info from server */
  void handleGameMessage(bosonMsgTag, int, bosonMsgData *);


  protected slots:      
    /** exits the application */
    void slotAppExit();
    /** open a recently open map */
    void slotFileRecent(int);
    /** toggle the toolbar*/
//    void slotViewToolBar_0(); 
    /** toggle the statusbar*/
//    void slotViewStatusBar(); 
    /** change the status message to text */
    void slotStatusMsg(const char *text);
    /** change the status message of the whole statusbar temporary */
    void slotStatusHelpMsg(const char *text);

private: 
    /** menu_bar is the applications main menubar */
    KMenuBar *menu_bar;
    /** tool_bar_0 is the first toolbar. If more toolbars are needed, please
      * increase _0 to _1 etc. */
    KToolBar *tool_bar_0;
    /** status_bar is the default statusbar of the application */
    KStatusBar *status_bar;
    QPopupMenu *file_menu_recent;
    QPopupMenu *view_menu;
    QStrList *recentList; 
    /** rMainGeom keeps the acutal size of your application and is used to
      * restore your last used application size. */
    //QRect rMainGeom;
    /** flag if toolbar is visible or not. Used for kconfig and checking the
      * view-menu entry view toolbar. bViewStatusbar does the same for the
      * statusbar. 
      */
    bool bViewToolbar_0;
    bool bViewStatusbar;
    /** flag if view is there or not (view is deleted on "File"->"Close") */
    bool bViewEnabled;
    /** used for KConfig to store and restore menubar position. Same with
      * tool_bar_0_pos. If more menubars or toolbars are created, you should add
      * positions as well and implement them in saveOptions() and readOptions().
      */    
    KMenuBar::menuPosition menu_bar_pos;
    KToolBar::BarPosition tool_bar_0_pos; 


/* The map which handle grouds and units*/
    editorField		*field;
    QString		*currentFile;

};   
 
#endif // BOEDITOR_H
 
