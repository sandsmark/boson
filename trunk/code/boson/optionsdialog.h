/*
    This file is part of the Boson game
    Copyright (C) 2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <kdialogbase.h>

#include "global.h"

class BosonConfig;
class OptionsWidget;
class Player;
class Boson;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class OptionsDialog : public KDialogBase
{
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent, bool modal = false);
	~OptionsDialog();

	void setGame(Boson* boson);
	void setPlayer(Player* localPlayer);

	void setCursor(CursorMode mode);

signals:
	void signalUpdateIntervalChanged(unsigned int);

	void signalCursorChanged(int index, const QString& cursorDir);

	void signalCmdBackgroundChanged(const QString& file);
	void signalMiniMapScaleChanged(double);

public slots:
	void slotLoad();

protected slots:
	virtual void slotOk();
	virtual void slotApply();
	virtual void slotDefault();


protected:
	void initGeneralPage();
	void initCursorPage();
	void initScrollingPage();
	void initSoundsPage();
	void initOpenGLPage();
	void initChatPage();

	void addOptions(OptionsWidget* o);

private:
	class OptionsDialogPrivate;
	OptionsDialogPrivate* d;

};

#endif
