/***************************************************************************
                          boson.cpp  -  description                              
                             -------------------                                         

    version              :                                   
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

#include <stdio.h>
#include <unistd.h>
#include <boson.h>
#include <qframe.h>
#include "ressource.h"
#include "connect.h"
#include "../common/boconfig.h"
#include "../common/bobuffer.h"
#include "../common/log.h"

FILE *logfile = (FILE *) 0L;

BosonApp::BosonApp()
{

  logfile = fopen(BOSON_LOGFILE_CLIENT, "a+b");
  if (!logfile) {
	logfile = stderr;
	logf(LOG_ERROR, "Can't open logfile, using stderr");
	}

  logf(LOG_INFO, "========= New Log File ==============");

  setCaption("Boson " VERSION);
  ///////////////////////////////////////////////////////////////////
  // create basics 
  recentList = new QStrList(true);
  recentList->setAutoDelete(true);

  ///////////////////////////////////////////////////////////////////
  // read the config file options
  readOptions();

  ///////////////////////////////////////////////////////////////////
  // call init() to invoke all other construction parts
  init();
}

BosonApp::~BosonApp()
{
logf(LOG_INFO, "Closing logfile.\n+++\n\n");
if (logfile != stderr) fclose(logfile);
}

void BosonApp::enableCommand(int id_)
{
  ///////////////////////////////////////////////////////////////////
  // enable menu and toolbar functions by their ID's
  menu_bar->setItemEnabled(id_,true);
  tool_bar_0->setItemEnabled(id_,true);
}

void BosonApp::disableCommand(int id_)
{
  ///////////////////////////////////////////////////////////////////
  // disable menu and toolbar functions by their ID's
  menu_bar->setItemEnabled(id_,false);
  tool_bar_0->setItemEnabled(id_,false);
}


void BosonApp::init()
{ 

  jiffies = 0;
  ///////////////////////////////////////////////////////////////////
  // set up the base application features
  initMenuBar();
  initToolBars();
  initStatusBar();
  initView();
  initSocket();

  ///////////////////////////////////////////////////////////////////
  // enable bars dependend on config file setups
  if (!bViewToolbar_0)
    enableToolBar(KToolBar::Hide,0);
  if (!bViewStatusbar)
    enableStatusBar(KStatusBar::Hide);

  menu_bar->setMenuBarPos(menu_bar_pos);
  tool_bar_0->setBarPos(tool_bar_0_pos);

  ///////////////////////////////////////////////////////////////////
  // disable menu and toolbar items at startup
/*  disableCommand(ID_FILE_SAVE);
  disableCommand(ID_FILE_SAVE_AS);
  disableCommand(ID_FILE_PRINT);
  disableCommand(ID_EDIT_CUT);
  disableCommand(ID_EDIT_COPY);*/
}

void BosonApp::initSocket()
{
char charbuf[1024];

if (gethostname(charbuf, 1023)) {
	printf("can't get hostname, aborting\n");
	}

/* let's say the server is on the local machine, hum..  */
socket = new KSocket(charbuf,  BOSON_DEFAULT_PORT);

if (-1 == socket->socket())  {
	logf(LOG_FATAL, "BosonApp : beuh, unable to connect socket\n");
	socketState = PSS_CONNECT_DOWN;
	return;
	}

buffer = new boBuffer(socket->socket(), BOSON_BUFFER_SIZE );

logf(LOG_COMM, "KSocket connect ok");
logf(LOG_COMM, "\tsocket = %d, addr = %lu",
			socket->socket(), socket->getAddr());

socketState = PSS_INIT;

connect (
	socket, SIGNAL(readEvent(KSocket *)), 
	this, SLOT(handleSocketMessage(KSocket*) ) );
socket->enableRead(TRUE);

/*
bosonMsgData	data;
bosonMsgTag	tag;
int		len;
*/

logf(LOG_LAYER1, "Sending MSG_HS_INIT...");
sendMsg(buffer, MSG_HS_INIT, BOSON_NO_DATA );

socketState = PSS_WAIT_CONFIRM_INIT ;
}

void BosonApp::initMenuBar()
{

  ///////////////////////////////////////////////////////////////////
  // MENUBAR  


  ///////////////////////////////////////////////////////////////////
  // file_menu entry file_menu_recent  (for recent documents)


  QPopupMenu *file_menu_recent = new QPopupMenu();
  for (uint i = 0; i < recentList->count(); i++)
    file_menu_recent->insertItem(recentList->at(i));


  ///////////////////////////////////////////////////////////////////
  // menuBar entry file_menu
  QPopupMenu *file_menu = new QPopupMenu();
  file_menu->insertItem(Icon("mini/bosonapp.xpm"), i18n("New &Window"), ID_FILE_NEW_WINDOW );
  file_menu->insertSeparator();
  file_menu->insertItem(Icon("filenew.xpm"), i18n("&New"), ID_FILE_NEW );
  file_menu->insertItem(Icon("fileopen.xpm"), i18n("&Open..."), ID_FILE_OPEN );
  file_menu->insertItem(i18n("Open &recent..."), file_menu_recent, ID_FILE_RECENT );
  file_menu->insertSeparator();
  file_menu->insertItem(Icon("filefloppy.xpm") ,i18n("&Save"), ID_FILE_SAVE );
  file_menu->insertItem(i18n("Save &as"), ID_FILE_SAVE_AS );
  file_menu->insertItem(i18n("&Close"), ID_FILE_CLOSE );
  file_menu->insertSeparator();
  file_menu->insertItem(Icon("fileprint.xpm"), i18n("&Print"), ID_FILE_PRINT );
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("C&lose Widow"), ID_FILE_CLOSE_WINDOW);
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
  QPopupMenu *edit_menu = new QPopupMenu();
  edit_menu->insertItem(Icon("editcut.xpm"), i18n("Cu&t"), ID_EDIT_CUT );
  edit_menu->insertItem(Icon("editcopy.xpm"), i18n("&Copy"), ID_EDIT_COPY );
  edit_menu->insertItem(Icon("editpaste.xpm"), i18n("&Paste"), ID_EDIT_PASTE );
 
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
  view_menu->setItemChecked(ID_VIEW_STATUSBAR, bViewStatusbar);

  ///////////////////////////////////////////////////////////////////
  // EDIT YOUR APPLICATION SPECIFIC MENUENTRIES HERE
  

  ///////////////////////////////////////////////////////////////////
  // menuBar entry help_menu
  QPopupMenu *help_menu ;///orzel = new QPopupMenu();
  help_menu = kapp->getHelpMenu(true, i18n(IDS_APP_ABOUT));


  ///////////////////////////////////////////////////////////////////
  // MENUBAR CONFIGURATION
  // set menu_bar the current menuBar and the position due to config file
  menu_bar = menuBar();
  menu_bar->insertItem(i18n("&File"), file_menu);
  menu_bar->insertItem(i18n("&Edit"), edit_menu);
  menu_bar->insertItem(i18n("&View"), view_menu);

  ///////////////////////////////////////////////////////////////////
  // INSERT YOUR APPLICATION SPECIFIC MENUENTRIES HERE


  menu_bar->insertSeparator(); 
  menu_bar->insertItem(i18n("&Help"), help_menu);

  ///////////////////////////////////////////////////////////////////
  // CONNECT THE SUBMENU SLOTS WITH SIGNALS

  CONNECT_CMD(file_menu);
  CONNECT_CMD(edit_menu);
  CONNECT_CMD(view_menu);

  //connect(file_menu_recent, SIGNAL(activated(int)),SLOT(slotFileRecent(int)));  

  ///////////////////////////////////////////////////////////////////
  // INSERT YOUR APPLICATION SPECIFIC CONNECTS HERE



}
void BosonApp::initToolBars()
{

  ///////////////////////////////////////////////////////////////////
  // TOOLBAR
  // set tool_bar_0 the current toolBar and the position due to config file
  tool_bar_0 = toolBar(0);
  tool_bar_0->insertButton(Icon("filenew.xpm"), ID_FILE_NEW, true, i18n("New File"));
  tool_bar_0->insertButton(Icon("fileopen.xpm"), ID_FILE_OPEN, true, i18n("Open File"));
  tool_bar_0->insertButton(Icon("filefloppy.xpm"), ID_FILE_SAVE, true, i18n("Save File"));
  tool_bar_0->insertButton(Icon("editcopy.xpm"), ID_EDIT_COPY, true, i18n("Copy"));
  tool_bar_0->insertButton(Icon("editpaste.xpm"), ID_EDIT_PASTE, true, i18n("Paste"));
  tool_bar_0->insertButton(Icon("editcut.xpm"), ID_EDIT_CUT, true, i18n("Cut"));
  tool_bar_0->insertButton(Icon("fileprint.xpm"), ID_FILE_PRINT, true, i18n("Print"));
  tool_bar_0->insertSeparator();
  tool_bar_0->insertButton(Icon("help.xpm"), ID_HELP, SIGNAL(pressed()), kapp, SLOT(appHelpActivated()), true, i18n("Help"));

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

void BosonApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  //STATUSBAR
  //set status_bar the current statusBar
  status_bar = statusBar(); 
  status_bar->insertItem(IDS_DEFAULT, ID_STATUS_MSG );

}

void BosonApp::initView()
{ 

/* the physical map is created when a game is created */
	phys = new physMap(200,200);

/* a mainView is each window containing : field, mini, order...
   this one is the first one, other can pop up as well */

	mainView *mainview = new mainView(phys, this, "main_view_0");
	setView(mainview);

	this->resize(700,600);
}



void BosonApp::resizeEvent(QResizeEvent *evt)
{
    KTMainWindow::resizeEvent(evt);

    rMainGeom= this->geometry();
    //map->resize(rMainGeom.width(),rMainGeom.height());
    updateRects();
}

void BosonApp::addRecent(const char *filename)
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

void BosonApp::dlgModified()
{
  int qret=KMsgBox::yesNoCancel(this, i18n("Warning"),		
	    i18n("The current file has been modified. \nSave Changes ?"));     
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

bool BosonApp::queryExit()
{
  int exit=KMsgBox::yesNo(this, i18n("Exit"), i18n("Really Quit ?"));

  if(exit==1)
    return true;
  else
    return false;
}

void BosonApp::saveOptions()
{
  KConfig *config = kapp->getConfig();

  config->setGroup("FILES");
  config->writeEntry("RecentList", *recentList);

  config->setGroup("APPEARANCE");
  config->writeEntry("MainGeometry",rMainGeom);
  config->writeEntry("ShowToolbar_0",tool_bar_0->isVisible());
  config->writeEntry("ShowStatusbar",status_bar->isVisible());
  config->writeEntry("MenuBarPos", (int)menu_bar->menuBarPos());
  config->writeEntry("ToolBar_0_Pos", (int)tool_bar_0->barPos());

}

void BosonApp::readOptions()
{
  ///////////////////////////////////////////////////////////////////
  // read the config file entries
  KConfig *config = kapp->getConfig();

  config->setGroup("FILES");

  config->readListEntry("RecentList", *recentList);

 
  config->setGroup("APPEARANCE");
  rMainGeom = config->readRectEntry("MainGeometry",&QRect(0,0,500,450));
  bViewToolbar_0 = config->readBoolEntry("ShowToolbar_0", true);
  bViewStatusbar = config->readBoolEntry("ShowStatusbar", true);
  menu_bar_pos = (KMenuBar::menuPosition)config->readNumEntry("MenuBarPos", KMenuBar::Top); 
  tool_bar_0_pos = (KToolBar::BarPosition)config->readNumEntry("ToolBar_0_Pos", KToolBar::Top);

}


/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void BosonApp::slotAppExit()
{ 

  ///////////////////////////////////////////////////////////////////
  // exits the Application
  if(this->queryExit())
    {
      saveOptions();
      KTMainWindow::deleteAll();
      kapp->quit();
    }
  else return;
}

void BosonApp::slotViewToolBar_0()
{
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  bViewToolbar_0=!bViewToolbar_0;
  menu_bar->setItemChecked(ID_VIEW_TOOLBAR_0, bViewToolbar_0);
  enableToolBar(KToolBar::Toggle,0);

}

void BosonApp::slotViewStatusBar()
{

  ///////////////////////////////////////////////////////////////////
  //turns Statusbar on or off
  bViewStatusbar=!bViewStatusbar;
  menu_bar->setItemChecked(ID_VIEW_STATUSBAR, bViewStatusbar);
  enableStatusBar();
}


void BosonApp::slotStatusMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  status_bar->clear();
  status_bar->changeItem(text, ID_STATUS_MSG );
}


void BosonApp::slotStatusHelpMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message of whole statusbar temporary (text, msec)
  status_bar->message(text, 2000);
}




///////////////////////////////////////////////////////////////////
// THESE MACROS CONSTRUCT THE SWITCH FOR THE SIGNAL commandCallback(int)


BEGIN_CMD(BosonApp)

  ON_CMD(ID_APP_EXIT,                 slotAppExit(),            i18n(""))


  

  ON_CMD(ID_VIEW_TOOLBAR_0,           slotViewToolBar_0(),      i18n(""))
  ON_CMD(ID_VIEW_STATUSBAR,           slotViewStatusBar(),      i18n(""))
END_CMD()


///////////////////////////////////////////////////////////////////
// THESE MACROS CONSTRUCT THE SWITCH FOR THE SIGNAL statusCallback(int)


BEGIN_STATUS_MSG(BosonApp)
  ON_STATUS_MSG(ID_FILE_NEW_WINDOW,  i18n("Opens a new application window"))

  ON_STATUS_MSG(ID_FILE_NEW,         i18n("Creates a new document"))
  ON_STATUS_MSG(ID_FILE_OPEN,        i18n("Opens an existing document"))
  ON_STATUS_MSG(ID_FILE_RECENT,      i18n("Opens a recent document"))

  ON_STATUS_MSG(ID_FILE_SAVE,        i18n("Save the actual document"))
  ON_STATUS_MSG(ID_FILE_SAVE_AS,     i18n("Save the document as..."))
  ON_STATUS_MSG(ID_FILE_CLOSE,       i18n("Closes the actual file"))

  ON_STATUS_MSG(ID_FILE_PRINT,       i18n("Prints the current document"))

  ON_STATUS_MSG(ID_FILE_CLOSE_WINDOW,i18n("Closes the current window"))

  ON_STATUS_MSG(ID_APP_EXIT,         i18n("Exits the program"))  


  ON_STATUS_MSG(ID_EDIT_CUT,         i18n("Cuts the selected section and puts it to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_COPY,        i18n("Copys the selected section to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_PASTE,       i18n("Pastes the clipboard contents to actual position"))
  ON_STATUS_MSG(ID_EDIT_SELECT_ALL,  i18n("Selects the whole document contents"))


  ON_STATUS_MSG(ID_VIEW_TOOLBAR_0,   i18n("Enables / disables the actual Toolbar"))
  ON_STATUS_MSG(ID_VIEW_STATUSBAR,   i18n("Enables / disables the Statusbar"))
  ON_STATUS_MSG(ID_VIEW_OPTIONS,     i18n("Set program options"))

END_STATUS_MSG()
