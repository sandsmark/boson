/***************************************************************************
                          boeditor.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#include <qlineedit.h>

#include <klocale.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>


#include "common/boconfig.h"
#include "common/log.h"

#include "boeditor.h"
#include "editorTopLevel.h"
#include "editorCanvas.h"
#include "newDlg.h"
#include "visual.h"

/*
 * visual/visual.h
 */
visualCanvas		*vcanvas = 0;
speciesTheme		*species[BOSON_MAX_PLAYERS] = {0l, 0l};
uint			nb_player;
QString			*dataPath;

extern editorCanvas	*ecanvas;

#define UNKNOWN_NAME  "___orzel_unknown_name___" 	// will anybody ever save a file with this name ?

BoEditorApp::BoEditorApp()
{

	/* logfile handling */
	logfile = fopen(BOSON_LOGFILE_EDITOR, "a+b");
	if (!logfile) {
		logfile = stderr;
		logf(LOG_ERROR, "Can't open logfile, using stderr");
	}
	logf(LOG_INFO, "========= New Log File ==============");

	filename = QString::null;
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
	(void) new KAction(
		i18n("New &Window"), 0,
		this, SLOT(slot_newWindow()),
		&m_actions, "new_window");

	/* standard actions */
	ADD_ACTION(openNew);
	ADD_ACTION(open);
	ADD_ACTION(close);
	ADD_ACTION(openRecent);
	ADD_ACTION(save);
	ADD_ACTION(saveAs);
	ADD_ACTION(quit);
}
#undef ADD_ACTION

/*
 *
 * ACTIONS-related slots
 *
 */

void BoEditorApp::slot_newWindow()
{
	editorTopLevel *btl = new editorTopLevel(this);
	btl->show();
	if (!filename.isNull()) btl->setCaption(filename);
	topLevels.append(btl);
}

void BoEditorApp::slot_openNew()
{
	if (!filename.isNull() && !slot_close()) return;

	newDlg	*newdlg = new newDlg(0, "New Scenario"); // 0 -> centered
	if (newdlg->exec() != QDialog::Accepted) {
		delete newdlg;
		return;
	}

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
	if (!ecanvas->New(g, newdlg->scb_width->value(), newdlg->scb_height->value(), newdlg->qle_name->text() ) ) {
		delete newdlg;
  		KMessageBox::sorry(0, i18n("Creation of new scenario failed :-((("), i18n("Creaton failed") );
		return;
	}

	delete newdlg;

	filename = UNKNOWN_NAME;

	ecanvas->update();
}

void BoEditorApp::slot_open()
{
	if (!filename.isNull() && !slot_close()) return;

	QString name = KFileDialog::getOpenFileName( "", "*.bpf" , 0 );
	if ( name.isEmpty() ) return;

	do_open(name);
}

void BoEditorApp::do_open(QString name)
{
	// internal management
	boAssert( QString::null == filename);	// should not be called if filame already set
	boAssert( QString::null != name);	// shoud not be called with a null arg
	filename = name;
	
	// actually read the file
	boAssert(ecanvas);
	if (!ecanvas->Load(filename)) {
		// it failed : turning back to nothing
		logf(LOG_ERROR, "haven't been abled to open %s", name.latin1() );
		QString msg = i18n("Can't open the file %1").arg(name);
		KMessageBox::sorry(0, msg, i18n("Can't open file") );
		filename = QString::null;
		return;
	}

	// set captions on TopLevel windows
	for ( editorTopLevel *btl=topLevels.first(); btl != 0; btl=topLevels.next() )
		btl->setCaption(name);
}

bool BoEditorApp::slot_close()
{
	/* already closed ? */
	if (filename.isNull()) return true;
	
	/* modified ? */
	if (ecanvas->isModified())
		switch (KMessageBox::warningYesNoCancel(0, // 0 means app-level messagebox
				"The current file has been modified\n"
				"Do you want to save it ?")) {
			case KMessageBox::Yes:
				if (!slot_save()) return false;
				break;
			case KMessageBox::No:
				/* discar changes */
				break;
			default:
			case KMessageBox::Cancel:
				return false;
		}

	/* actually close the file */
//	delete ecanvas; ecanvas = 0l;
//XXX	handle windows here
//	delete mainWidget; mainWidget = 0l;
	filename = QString::null;
	return true;
}

void BoEditorApp::slot_openRecent()
{
}

bool BoEditorApp::slot_save()
{
	if (filename == UNKNOWN_NAME)
		return slot_saveAs();

	/* actual saving */
	if (ecanvas->Save(filename))
			return true;

	KMessageBox::sorry(0, i18n("Saving of the file failed"), i18n("Save failed") );

	return false;
}

bool BoEditorApp::slot_saveAs()
{
	QString name;
	name = KFileDialog::getSaveFileName( "", "*bpf", 0l );
	if ( name.isEmpty() ) return false;

	/* already exists */
	QFileInfo info( name );
	QString msg = i18n("A document with this name already exists.\n" "Do you want to overwrite it?" );
	if( info.exists()  && KMessageBox::Yes != KMessageBox::warningYesNo( 0, msg, i18n("Save As") ))
		return false;

	/* it's ok */
	filename = name;
	boAssert(filename != QString::null); // recursion with ::slot_save
	return slot_save();	// save it
}

void BoEditorApp::slot_quit()
{
//	logf(LOG_INFO, "slot_quit called");
	if (slot_close()) quit() ;
}

