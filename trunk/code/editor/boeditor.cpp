/***************************************************************************
                          boson.cpp  -  description                              
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

#include <klocale.h>
#include <kstdaction.h>

#include "common/boconfig.h"
#include "common/log.h"

#include "boeditor.h"
#include "visual.h"

/*
 * visual/visual.h
 */
visualCanvas		*vcanvas = 0;
speciesTheme		*species[BOSON_MAX_PLAYERS] = {0l, 0l};
int			nb_player;

BoEditorApp::BoEditorApp()
{

	/* logfile handling */
	logfile = fopen(BOSON_LOGFILE_CLIENT, "a+b");
	if (!logfile) {
		logfile = stderr;
		logf(LOG_ERROR, "Can't open logfile, using stderr");
	}
	logf(LOG_INFO, "========= New Log File ==============");

	/* application initialisation */
	init();

}

BoEditorApp::~BoEditorApp()
{
	/* logfile handling */
	logf(LOG_INFO, "Closing logfile.\n+++\n\n");
	if (logfile != stderr) fclose(logfile);
}

#define ADD_ACTION(name) KStdAction::##name(this, SLOT(slot_##name()), &m_actions);

void BoEditorApp::init()
{ 
	/* customs actions */
	puts("new_window...");
	(void) new KAction(
		i18n("&New Window"), 0,
		this, SLOT(slot_newWindow()),
		&m_actions, "new_window");
	puts("ok");

	/*ark one :
	(void) new KAction(
	   	i18n("New &Window"), 0,
		this, SLOT(file_newWindow()),
		actionCollection(), "new_window");
	*/

	/* standard actions */
	ADD_ACTION(openNew);
	ADD_ACTION(open);
	ADD_ACTION(openRecent);
	ADD_ACTION(save);
	ADD_ACTION(saveAs);
	ADD_ACTION(quit);
	// those should go to KTMW
//	ADD_ACTION(close);
//	ADD_ACTION(showToolbar);
	puts("kapp : end of init()");
}
#undef ADD_ACTION

/*
 *
 * ACTIONS-related slots
 *
 */

void BoEditorApp::slot_newWindow()
{
}

void BoEditorApp::slot_openNew()
{
}

void BoEditorApp::slot_open()
{
}

void BoEditorApp::slot_openRecent()
{
}

void BoEditorApp::slot_save()
{
}

void BoEditorApp::slot_saveAs()
{
}

void BoEditorApp::slot_quit()
{
}

