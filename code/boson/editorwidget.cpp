/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "editorwidget.h"

#include "defines.h"
#include "boson.h"
#include "bosonmap.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "bodebug.h"
#include "bpfdescriptiondialog.h"
#include "optionsdialog.h"
#include "boaction.h"
#include "botexmapimportdialog.h"
#include "bosongroundtheme.h"
#include "bosonbigdisplaybase.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>

#include <qptrlist.h>
#include <qvalidator.h>
#include <qimage.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "editorwidget.moc"

class EditorWidget::EditorWidgetPrivate
{
public:
	EditorWidgetPrivate()
	{
	}
};

EditorWidget::EditorWidget(QWidget* parent)
    : BosonWidgetBase(parent)
{
 d = new EditorWidgetPrivate;
}

EditorWidget::~EditorWidget()
{
 boDebug() << k_funcinfo << endl;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::initDisplayManager()
{
 BosonWidgetBase::initDisplayManager();
 BO_CHECK_NULL_RET(displayManager());
 BO_CHECK_NULL_RET(displayManager()->activeDisplay());
 BosonBigDisplayBase* display = displayManager()->activeDisplay();

 connect(displayManager(), SIGNAL(signalLockAction(bool)),
		this, SLOT(slotLockAction(bool)));
 connect(display, SIGNAL(signalEditorChangeLocalPlayer(Player*)),
		this, SIGNAL(signalChangeLocalPlayer(Player*)));
 connect(display, SIGNAL(signalEditorEditPlayerMinerals()),
		this, SLOT(slotEditPlayerMinerals()));
 connect(display, SIGNAL(signalEditorEditPlayerOil()),
		this, SLOT(slotEditPlayerOil()));
}

void EditorWidget::initMap()
{
 BosonWidgetBase::initMap();
 if (!boGame->playField() || !boGame->playField()->map()) {
	boError() << k_funcinfo << endl;
	return;
 }
 BosonMap* map = boGame->playField()->map();

 connect(boGame, SIGNAL(signalChangeTexMap(int,int,unsigned int,unsigned int*,unsigned char*)),
		map, SLOT(slotChangeTexMap(int,int,unsigned int,unsigned int*,unsigned char*)));
}

void EditorWidget::slotChangeCursor(int , const QString& )
{
 // editor mode
 changeCursor(new BosonKDECursor());
}

void EditorWidget::slotEditPlayerMinerals()
{
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->minerals());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Minerals for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	// cancel pressed
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setMinerals(v);
}

void EditorWidget::slotEditPlayerOil()
{
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->oil());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Oil for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setOil(v);
}

void EditorWidget::slotEditHeight(bool on)
{
 if (on) {
	BoSpecificAction action;
	action.setType(ActionChangeHeight);
	displayManager()->slotAction(action);
 } else {
	displayManager()->unlockAction();
 }
}

void EditorWidget::slotLockAction(bool locked)
{
 if (locked) {
	// we might display something in the cmdframe or in the toolbar or so.
	// the cursor will get updated anyway.
	return;
 }
}

