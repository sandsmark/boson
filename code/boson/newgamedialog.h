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
#ifndef __NEWGAMEDIALOG_H__
#define __NEWGAMEDIALOG_H__

#include <kgame/kgamedialog.h>

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class NewGameDialog : public KGameDialog
{
	Q_OBJECT
public:
	NewGameDialog(KGame*, KPlayer* owner, QWidget* parent);
	virtual ~NewGameDialog();

public slots:
	virtual void slotOk();

signals:
	void signalStartGame();

private:
	KGame* mGame;
};

#endif
