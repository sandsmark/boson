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
#ifndef __KGAMEDIALOGBOSONCONFIG_H__
#define __KGAMEDIALOGBOSONCONFIG_H__

#include <kgame/kgamedialogconfig.h>

class KGameDialogBosonConfigPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialogBosonConfig : public KGameDialogGeneralConfig
{
	Q_OBJECT
public:
	KGameDialogBosonConfig(QWidget* parent = 0);
	virtual ~KGameDialogBosonConfig();

signals:
	void signalStartGame();
	void signalMapChanged(const QString& fileName);
	void signalScenarioChanged(const QString& fileName);
	void signalSpeciesChanged(const QString& species);

public slots:
	void slotMapChanged(int index);
	void slotMapChanged(const QString& mapIdentifier);
	void slotScenarioChanged(const QString& scenarioIdentifier);

protected slots:
	void slotStartGame();
	void slotScenarioChanged(int index);
	void slotSpeciesChanged(int index);

private:
	KGameDialogBosonConfigPrivate* d;
};
#endif
