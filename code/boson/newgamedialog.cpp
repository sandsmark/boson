/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
