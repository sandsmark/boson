/***************************************************************************
                          boeditor.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <qframe.h>
#include <qdir.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <khelpmenu.h>
#include <kmessagebox.h>

#include "common/boconfig.h"
#include "common/bobuffer.h"
#include "common/log.h"

#include "visual.h"
#include "ressource.h"

#include "boeditor.h"
#include "newDlg.h"


FILE *logfile = (FILE *) 0L;

/*
 * visual/visual.h
 */
visualCanvas		*vcanvas;
groundTheme		*ground = 0;
speciesTheme		*species[BOSON_MAX_PLAYERS] = {0l, 0l};
int			nb_player;


BoEditorApp::BoEditorApp(char *filename)
{

  logfile = fopen(BOSON_LOGFILE_EDITOR, "a+b");
  if (!logfile) {
	logfile = stderr;
	logf(LOG_ERROR, "Can't open logfile, using stderr");
	}

  logf(LOG_INFO, "========= New Log File ==============");

  setCaption("BoEditor " VERSION);
  ///////////////////////////////////////////////////////////////////
  // create basics 
  recentList = new QStrList(true);
  recentList->setAutoDelete(true);

  ///////////////////////////////////////////////////////////////////
  // read the config file options
  //readOptions();

  ///////////////////////////////////////////////////////////////////
  // call init() to invoke all other construction parts
  init(filename);
}

BoEditorApp::~BoEditorApp()
{

delete currentFile;
delete recentList;
delete field;
logf(LOG_INFO, "Closing logfile\n");
if (logfile && logfile != stderr) fclose(logfile);
}

void BoEditorApp::enableCommand(int id_)
{
  ///////////////////////////////////////////////////////////////////
  // enable menu and toolbar functions by their ID's
  menu_bar->setItemEnabled(id_,true);
  tool_bar_0->setItemEnabled(id_,true);
}

void BoEditorApp::disableCommand(int id_)
{
  ///////////////////////////////////////////////////////////////////
  // disable menu and toolbar functions by their ID's
  menu_bar->setItemEnabled(id_,false);
  tool_bar_0->setItemEnabled(id_,false);
}


void BoEditorApp::init(char *filename )
{ 
  field = 0;
  currentFile = new QString();

  ///////////////////////////////////////////////////////////////////
  // set up the base application features
  initMenuBar();
  initToolBars();
  initStatusBar();

  ///////////////////////////////////////////////////////////////////
  // disable menu and toolbar items at startup
  disableCommand(ID_FILE_SAVE);
  disableCommand(ID_FILE_SAVE_AS);
  disableCommand(ID_FILE_CLOSE);
  disableCommand(ID_FILE_PRINT);

  disableCommand(ID_EDIT_CUT);
  disableCommand(ID_EDIT_COPY);
  
  if (filename) doLoad(QString(filename));
}


void BoEditorApp::initMenuBar()
{

  ///////////////////////////////////////////////////////////////////
  // MENUBAR  


  ///////////////////////////////////////////////////////////////////
  // file_menu entry file_menu_recent  (for recent documents)
	file_menu_recent = new QPopupMenu();
	for (uint i = 0; i < recentList->count(); i++)
		file_menu_recent->insertItem(recentList->at(i));

	connect( file_menu_recent, SIGNAL(activated(int)), SLOT(slotFileOpenRecent(int)) );


  ///////////////////////////////////////////////////////////////////
  // menuBar entry file_menu

  QPopupMenu *file_menu = new QPopupMenu();
  file_menu->insertItem(BarIcon("mini/bosonapp"), i18n("New &Window"), ID_FILE_NEW_WINDOW );
  file_menu->insertSeparator();
  file_menu->insertItem(BarIcon("filenew"), i18n("&New"), ID_FILE_NEW );
  file_menu->insertItem(BarIcon("fileopen"), i18n("&Open..."), ID_FILE_OPEN );
  file_menu->insertItem(i18n("Open &recent..."), file_menu_recent, ID_FILE_RECENT );
  file_menu->insertSeparator();
  file_menu->insertItem(BarIcon("filefloppy") ,i18n("&Save"), ID_FILE_SAVE );
  file_menu->insertItem(i18n("Save &as"), ID_FILE_SAVE_AS );
  file_menu->insertItem(i18n("&Close"), ID_FILE_CLOSE );
  file_menu->insertSeparator();
  file_menu->insertItem(BarIcon("fileprint"), i18n("&Print"), ID_FILE_PRINT );
  file_menu->insertSeparator(); 
  file_menu->insertItem(i18n("C&lose Window"), ID_FILE_CLOSE_WINDOW);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("E&xit"), ID_APP_EXIT );

  // file_menu key accelerators
  file_menu->setAccel(CTRL+Key_N, ID_FILE_NEW);
  file_menu->setAccel(CTRL+Key_O, ID_FILE_OPEN);
  file_menu->setAccel(CTRL+Key_S, ID_FILE_SAVE);
  file_menu->setAccel(CTRL+Key_W, ID_FILE_CLOSE);
  file_menu->setAccel(CTRL+Key_P, ID_FILE_PRINT); 
  file_menu->setAccel(CTRL+Key_Q, ID_APP_EXIT);


  ///////////////////////////////////////////////////////////////////
  // menuBar entry edit_menu
  // 
  QPopupMenu *edit_menu = new QPopupMenu();

  edit_menu->insertItem(BarIcon("mini.destroy"), i18n("&Destroy"), ID_EDIT_DESTROY );
  edit_menu->setAccel(CTRL+Key_D, ID_EDIT_DESTROY);

  /*
  edit_menu->insertItem(BarIcon("editcopy"), i18n("&Copy"), ID_EDIT_COPY );
  edit_menu->insertItem(BarIcon("editpaste"), i18n("&Paste"), ID_EDIT_PASTE );
 
  //edit_menu key accelerators
  edit_menu->setAccel(CTRL+Key_X, ID_EDIT_CUT);
  edit_menu->setAccel(CTRL+Key_C, ID_EDIT_COPY);
  edit_menu->setAccel(CTRL+Key_V, ID_EDIT_PASTE);
 
  ///////////////////////////////////////////////////////////////////
  // menuBar entry view_menu
  QPopupMenu *view_menu = new QPopupMenu();
  view_menu->setCheckable(true);
  view_menu->insertItem(i18n("Tool&bar"), ID_VIEW_TOOLBAR_0);
  view_menu->insertItem(i18n("&Statusbar"), ID_VIEW_STATUSBAR );

  view_menu->setItemChecked(ID_VIEW_TOOLBAR_0, bViewToolbar_0);
  view_menu->setItemChecked(ID_VIEW_STATUSBAR, bViewStatusbar); */

  ///////////////////////////////////////////////////////////////////
  // EDIT YOUR APPLICATION SPECIFIC MENUENTRIES HERE
  

  ///////////////////////////////////////////////////////////////////
  // MENUBAR CONFIGURATION
  // set menu_bar the current menuBar and the position due to config file
  menu_bar = menuBar();
  menu_bar->insertItem(i18n("&File"), file_menu);
  menu_bar->insertItem(i18n("&Edit"), edit_menu);

  KHelpMenu *m_helpMenu = new KHelpMenu(this, instance()->aboutData(), true,
			                                       actionCollection()); 
  menu_bar->insertItem(i18n("&Help"), m_helpMenu->menu() );
//  menu_bar->insertItem(i18n("&View"), view_menu);

  ///////////////////////////////////////////////////////////////////
  // INSERT YOUR APPLICATION SPECIFIC MENUENTRIES HERE


  ///////////////////////////////////////////////////////////////////
  // CONNECT THE SUBMENU SLOTS WITH SIGNALS

  CONNECT_CMD(file_menu);
  CONNECT_CMD(edit_menu);
//  CONNECT_CMD(view_menu);

  connect(file_menu_recent, SIGNAL(activated(int)),SLOT(slotFileRecent(int)));  

  ///////////////////////////////////////////////////////////////////
  // INSERT YOUR APPLICATION SPECIFIC CONNECTS HERE



}
void BoEditorApp::initToolBars()
{

  ///////////////////////////////////////////////////////////////////
  // TOOLBAR
  // set tool_bar_0 the current toolBar and the position due to config file
  tool_bar_0 = toolBar(0);
  tool_bar_0->insertButton(BarIcon("configure"), ID_FILE_NEW_WINDOW, true, i18n("New window"));
  tool_bar_0->insertButton(BarIcon("flag"), ID_FILE_NEW_WINDOW, true, i18n("Move"));
  tool_bar_0->insertButton(BarIcon("stop"), ID_FILE_NEW_WINDOW, true, i18n("Stop"));
  tool_bar_0->insertButton(BarIcon("filedel"), ID_FILE_NEW_WINDOW, true, i18n("Attack"));
  tool_bar_0->insertButton(BarIcon("home"), ID_FILE_NEW_WINDOW, true, i18n("Go Back"));
  tool_bar_0->insertButton(BarIcon("idea"), ID_FILE_NEW_WINDOW, true, i18n("repair"));
  tool_bar_0->insertSeparator();
  tool_bar_0->insertButton(BarIcon("exit"), ID_APP_EXIT, SIGNAL(pressed()), this, SLOT(slotAppExit()), true, i18n("Exit"));
  tool_bar_0->insertButton(BarIcon("help"), ID_HELP, SIGNAL(pressed()), kapp, SLOT(appHelpActivated()), true, i18n("Help"));

  ///////////////////////////////////////////////////////////////////
  // INSERT YOUR APPLICATION SPECIFIC TOOLBARS HERE -e.g. tool_bar_1:
  // add functionality for new created toolbars in:
  // enableCommand, disableCommand, in the menu_bar and an additional function slotViewToolbar_1
  // for that also create a bViewToolbar_1 and a KConfig entry (see Constructor).
  // Also update ressource values and commands 


  ///////////////////////////////////////////////////////////////////
  // CONNECT THE TOOLBAR SLOTS WITH SIGNALS - add new created toolbars
  CONNECT_TOOLBAR(tool_bar_0);

}

void BoEditorApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  //STATUSBAR
  //set status_bar the current statusBar
  status_bar = statusBar(); 
  status_bar->insertItem(IDS_DEFAULT, ID_STATUS_MSG );

}

void BoEditorApp::slotFileClose()
{
	if (!field) return;
	
	if (field->isModified())
		switch (KMessageBox::warningYesNoCancel(this,
				"Current file isn't saved\n"
				"Do you want to save it, discard change, or cancel ?"
				"File not saved", 
				"Save It",
				"Discard Changes" )) {
			case 1: // save it
				slotFileSave();
				break;
			case 3: //cancel
				return;
				break;
		}
	delete mainview;
	delete field;

	field = 0;
	*currentFile = "";


	enableCommand(ID_FILE_NEW);
	enableCommand(ID_FILE_OPEN);
	enableCommand(ID_FILE_RECENT);
	disableCommand(ID_FILE_SAVE);
	disableCommand(ID_FILE_SAVE_AS);
	disableCommand(ID_FILE_CLOSE);

}


void BoEditorApp::resizeEvent(QResizeEvent *evt)
{
    KTMainWindow::resizeEvent(evt);

    updateRects();
}

void BoEditorApp::addRecentFile(const char *filename)
{
  if(filename && strlen(filename) == 0 || recentList->contains(filename))
    return;

  if(recentList->count() < 5)
    recentList->insert(0,filename);
  else
    {
      recentList->remove(4);
      recentList->insert(0,filename);
    }
  file_menu_recent->clear();
  
  for(int i=0; i< (int)recentList->count(); i++)
    file_menu_recent->insertItem(recentList->at(i));

}

void BoEditorApp::dlgModified()
{
	int qret=KMessageBox::warningYesNoCancel(this,
		i18n("The current file has been modified. \nSave Changes ?"),
		i18n("Warning") );     
  switch (qret)
   {
   case 1:
     break;
   case 2:
     break;
   case 3:
     return;
     break;
   default:
     break;
   }
}

bool BoEditorApp::queryExit()
{

  int exit=KMessageBox::questionYesNo(this, i18n("Really Quit ?"), i18n("Exit") );

  if(exit==1)
    return true;
  else
    return false;
}

/*
void BoEditorApp::saveOptions()
{
	KConfig *config = kapp->getConfig();

	config->setGroup("APPEARANCE");
	config->writeEntry("MainGeometry",rMainGeom);

//  config->writeEntry("ShowToolbar_0",tool_bar_0->isVisible());
//  config->writeEntry("ShowStatusbar",status_bar->isVisible());

	config->writeEntry("MenuBarPos", (int)menu_bar->menuBarPos());
	config->writeEntry("ToolBar_0_Pos", (int)tool_bar_0->barPos());
	config->writeEntry("Recent Files", *recentList);
}



void BoEditorApp::readOptions()
{
	///////////////////////////////////////////////////////////////////
	// read the config file entries
	KConfig *config = kapp->getConfig();
 
	config->setGroup("APPEARANCE");
	rMainGeom = config->readRectEntry("MainGeometry",&QRect(0,0,800,600));

//	bViewToolbar_0 = config->readBoolEntry("ShowToolbar_0", true);
//	bViewStatusbar = config->readBoolEntry("ShowStatusbar", true);
	bViewToolbar_0 = bViewStatusbar = true;

	menu_bar_pos = (KMenuBar::menuPosition)config->readNumEntry("MenuBarPos", KMenuBar::Top); 
	tool_bar_0_pos = (KToolBar::BarPosition)config->readNumEntry("ToolBar_0_Pos", KToolBar::Right);

	config->readListEntry("Recent Files", *recentList);


}
*/


/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void BoEditorApp::slotAppExit()
{ 

  ///////////////////////////////////////////////////////////////////
  // exits the Application
  if(this->queryExit())
    {
      //saveOptions();
//      KTMainWindow::deleteAll();
      kapp->quit();
    }
  else return;
}

/*
void BoEditorApp::slotViewToolBar_0()
{
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  bViewToolbar_0=!bViewToolbar_0;
  menu_bar->setItemChecked(ID_VIEW_TOOLBAR_0, bViewToolbar_0);
  enableToolBar(KToolBar::Toggle,0);

}

void BoEditorApp::slotViewStatusBar()
{

  ///////////////////////////////////////////////////////////////////
  //turns Statusbar on or off
  bViewStatusbar=!bViewStatusbar;
  menu_bar->setItemChecked(ID_VIEW_STATUSBAR, bViewStatusbar);
  enableStatusBar();
}
*/


void BoEditorApp::slotFileNewWindow()
{
/*
	mainView *mainview = new mainView(this, "main_view_0");
	setView(mainview);
*/
}

void BoEditorApp::slotFileNew()
{
	if(field && field->isModified()) {
  		KMessageBox::error(this, i18n("Warning"),	i18n("Present file isn't saved"));	
		return;
	}

	newDlg	*newdlg = new newDlg(0, "New Scenario"); // 0 -> centered
	if (newdlg->exec() != QDialog::Accepted) {
		delete newdlg;
		return;
	}

	/* the physical map is created when a game is created */
	vcanvas = field = new editorCanvas();

	groundType g;
	switch(newdlg->type) {
		default:
			g =  GROUND_WATER;
			break;
		case 1:
			g =  GROUND_GRASS;
			break;
		case 2:
			g =  GROUND_DESERT;
			break;
	};
	if (!field->New(g, newdlg->scb_width->value(), newdlg->scb_height->value(), newdlg->qle_name->text() ) ) {
		delete field;
		field = 0;
		delete newdlg;
  		KMessageBox::error(this, i18n("Warning"),	i18n("Creation of new scenario failed :-((("));	
		return;
	}

	delete newdlg;
	
	*currentFile = "";


	/* a mainView is each window containing : field, mini, order...
	   this one is the first one, other can pop up as well */

	mainview = new mainView( this, "main_view_0");
	connect(this, SIGNAL(destroyObjects()), mainview, SLOT(slotEditDestroy()));
	setView(mainview);
	resize(800,628);

	mainview->show();
	updateRects();

	enableCommand(ID_FILE_SAVE);
	enableCommand(ID_FILE_SAVE_AS);
	enableCommand(ID_FILE_CLOSE);


}

void BoEditorApp::slotFileOpen()
{
	if(field && field->isModified()) {
  		KMessageBox::error(this, i18n("Warning"),	i18n("Present file isn't saved"));	
		return;
	}
	
	QString fileToOpen=KFileDialog::getOpenFileName(QDir::homeDirPath(), "", this, i18n("Open File..."));
	if(!fileToOpen.isEmpty()){
		QFileInfo saveAsInfo(fileToOpen);
		doLoad(fileToOpen);
		setCaption(name("BoEditor") + (": "+saveAsInfo.fileName()) );
		addRecentFile(fileToOpen);
	}

	enableCommand(ID_FILE_SAVE);
	enableCommand(ID_FILE_SAVE_AS);
	enableCommand(ID_FILE_CLOSE);
}


void BoEditorApp::doLoad(QString newname)
{ 

	slotFileClose();
	/* the physical map is created when a game is created */
	vcanvas = field = new editorCanvas();

	*currentFile = newname;
	assert (true == field->Load(*currentFile));
	


	/* a mainView is each window containing : field, mini, order...
	   this one is the first one, other can pop up as well */

	mainview = new mainView( this, "main_view_0");
	connect(this, SIGNAL(destroyObjects()), mainview, SLOT(slotEditDestroy()));
	resize(800,628);
	setView(mainview);

	mainview->show();
	updateRects();

	enableCommand(ID_FILE_SAVE);
	enableCommand(ID_FILE_SAVE_AS);
	enableCommand(ID_FILE_CLOSE);
}


void BoEditorApp::slotFileOpenRecent(int id_)
{

	if(field && field->isModified()) {
  		KMessageBox::error(this, i18n("Warning"),	i18n("Present file isn't saved"));	
		return;
	}

	*currentFile = recentList->at(id_);
	field->Load( *currentFile);
	setCaption( QString (name("BoEditor")) + ": "+recentList->at(id_));

	enableCommand(ID_FILE_SAVE);
	enableCommand(ID_FILE_SAVE_AS);
	enableCommand(ID_FILE_CLOSE);
}

void BoEditorApp::slotFileSave()
{
	if (!field) return;
	if (!currentFile->isEmpty())
		field->Save(*currentFile);
	else slotFileSaveAs();
}

void BoEditorApp::slotFileSaveAs()
{
	QString newName=KFileDialog::getSaveFileName(QDir::currentDirPath(), "", this, i18n("Save As..."));
	if(!newName.isEmpty()){
		QFileInfo saveAsInfo(newName);

		*currentFile = newName;
		assert(field->Save(newName));
		addRecentFile(newName);
		setCaption(name("BoEditor") + (": "+saveAsInfo.fileName()) );
	}
}


void BoEditorApp::slotEditDestroy()
{
	// XXX orzel : dirty, needs to be cleaned up after app/window split
	emit destroyObjects();
}


void BoEditorApp::slotFileRecent(int w)
{
}


void BoEditorApp::slotStatusMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  status_bar->clear();
  status_bar->changeItem(text, ID_STATUS_MSG );
}


void BoEditorApp::slotStatusHelpMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message of whole statusbar temporary (text, msec)
  status_bar->message(text, 2000);
}




///////////////////////////////////////////////////////////////////
// THESE MACROS CONSTRUCT THE SWITCH FOR THE SIGNAL commandCallback(int)


BEGIN_CMD(BoEditorApp)

  ON_CMD(ID_APP_EXIT,			slotAppExit(),		i18n("Exiting the editor..."))
  ON_CMD(ID_FILE_NEW_WINDOW,		slotFileNewWindow(),	i18n("Opening a new view of the map..."))
  ON_CMD(ID_FILE_NEW       ,		slotFileNew(),		i18n("Creating a new map..."))
  ON_CMD(ID_FILE_OPEN      ,		slotFileOpen(),		i18n("Opening a file..."))

  ON_CMD(ID_FILE_SAVE      ,		slotFileSave(),		i18n("Saving the map"))
  ON_CMD(ID_FILE_SAVE_AS   ,		slotFileSaveAs(),	i18n("Saving the map under a new name..."))
  ON_CMD(ID_FILE_CLOSE     ,		slotFileClose(),	i18n("Closing the file..."))
  ON_CMD(ID_EDIT_DESTROY   ,		slotEditDestroy(),	i18n("Destroying object..."))

//  ON_CMD(ID_VIEW_TOOLBAR_0,           slotViewToolBar_0(),      i18n(""))
//  ON_CMD(ID_VIEW_STATUSBAR,           slotViewStatusBar(),      i18n(""))
END_CMD()


///////////////////////////////////////////////////////////////////
// THESE MACROS CONSTRUCT THE SWITCH FOR THE SIGNAL statusCallback(int)


BEGIN_STATUS_MSG(BoEditorApp)
  ON_STATUS_MSG(ID_FILE_NEW_WINDOW,  i18n("Opens a new view window"))

  ON_STATUS_MSG(ID_FILE_NEW,         i18n("Creates a new document"))
  ON_STATUS_MSG(ID_FILE_OPEN,        i18n("Opens an existing document"))
  ON_STATUS_MSG(ID_FILE_RECENT,      i18n("Opens a recent document"))

  ON_STATUS_MSG(ID_FILE_SAVE,        i18n("Save the actual document"))
  ON_STATUS_MSG(ID_FILE_SAVE_AS,     i18n("Save the document as..."))
  ON_STATUS_MSG(ID_FILE_CLOSE,       i18n("Closes the actual file"))

  ON_STATUS_MSG(ID_FILE_PRINT,       i18n("Prints the current document")) 

  ON_STATUS_MSG(ID_FILE_CLOSE_WINDOW,i18n("Closes the current window"))

  ON_STATUS_MSG(ID_APP_EXIT,         i18n("Exits BoEditor"))  
  ON_STATUS_MSG(ID_EDIT_DESTROY,     i18n("Destroy the current object"))  

/*

  ON_STATUS_MSG(ID_EDIT_CUT,         i18n("Cuts the selected section and puts it to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_COPY,        i18n("Copys the selected section to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_PASTE,       i18n("Pastes the clipboard contents to actual position"))
  ON_STATUS_MSG(ID_EDIT_SELECT_ALL,  i18n("Selects the whole document contents"))


  ON_STATUS_MSG(ID_VIEW_TOOLBAR_0,   i18n("Enables / disables the actual Toolbar"))
  ON_STATUS_MSG(ID_VIEW_STATUSBAR,   i18n("Enables / disables the Statusbar"))
  ON_STATUS_MSG(ID_VIEW_OPTIONS,     i18n("Set program options")) */

END_STATUS_MSG()
