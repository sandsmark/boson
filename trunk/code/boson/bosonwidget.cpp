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

#include "bosonwidget.h"

#include "defines.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "bosonconfig.h"
#include "optionsdialog.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "bosonbigdisplaybase.h"
#include "global.h"
#include "bodebug.h"
#include "sound/bosonaudiointerface.h"

#include <kstdgameaction.h>
#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>

#include <qsignalmapper.h>
#include <qtimer.h>

#include "bosonwidget.moc"

BosonWidget::BosonWidget(QWidget* parent)
    : BosonWidgetBase(parent)
{
}

BosonWidget::~BosonWidget()
{
}

void BosonWidget::initDisplayManager()
{
 BosonWidgetBase::initDisplayManager();
 BO_CHECK_NULL_RET(displayManager());
 BosonBigDisplayBase* display = displayManager()->activeDisplay();
 BO_CHECK_NULL_RET(display);
 connect(display, SIGNAL(signalSaveGame()), this, SIGNAL(signalSaveGame()));
}

void BosonWidget::slotChangeCursor(int mode, const QString& cursorDir_)
{
 boDebug() << k_funcinfo << endl;
 if (!boGame->gameMode()) {
	// editor mode
	mode = CursorKDE;
 }
 BosonCursor* b;
 switch (mode) {
	case CursorOpenGL:
		b = new BosonOpenGLCursor;
		break;
	case CursorKDE:
	default:
		b = new BosonKDECursor;
		mode = CursorKDE; // in case we had an unknown/invalid mode
		break;
 }

 QString cursorDir = cursorDir_;
 if (cursorDir.isNull()) {
	cursorDir = BosonCursor::defaultTheme();
 }

 bool ok = true;
 if (!b->insertMode(CursorMove, cursorDir, QString::fromLatin1("move"))) {
	ok = false;
 }
 if (!b->insertMode(CursorAttack, cursorDir, QString::fromLatin1("attack"))) {
	ok = false;
 }
 if (!b->insertMode(CursorDefault, cursorDir, QString::fromLatin1("default"))) {
	ok = false;
 }
 if (!ok) {
	boError() << k_funcinfo << "Could not load cursor mode " << mode << " from " << cursorDir << endl;
	delete b;
	if (!cursor() && mode != CursorKDE) { // loading *never* fails for CursorKDE. we check here anyway.
		// load fallback cursor
		slotChangeCursor(CursorKDE, QString::null);
		return;
	}
	// continue to use the old cursor
	return;
 }
 changeCursor(b);

 boConfig->setCursorMode(mode);
 boConfig->setCursorDir(cursorDir);
}


