/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "newgamedialog.h"

#include <klocale.h>
#include <kgame/kgame.h>

#include "newgamedialog.moc"


NewGameDialog::NewGameDialog(KGame* g, KPlayer* owner, QWidget* parent)
		: KGameDialog(g, owner, i18n("New Game"), parent, false)
{
 mGame = g;
}

NewGameDialog::~NewGameDialog()
{
}

void NewGameDialog::slotOk()
{
 slotApply();
 if (mGame && mGame->isAdmin() && !mGame->isRunning()) {
	emit signalStartGame();
 }
 KGameDialog::accept();
}
