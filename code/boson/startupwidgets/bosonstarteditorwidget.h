/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONSTARTEDITORWIDGET_H
#define BOSONSTARTEDITORWIDGET_H

#include <qwidget.h>

class BosonStartupNetwork;

class BosonStartEditorWidgetPrivate;
class BosonStartEditorWidget : public QWidget
{
	Q_OBJECT
public:
	BosonStartEditorWidget(BosonStartupNetwork* interface, QWidget* parent);
	~BosonStartEditorWidget();

public slots:
	virtual void slotStartGameClicked();

signals:
	/**
	 * Emitted when the player clicks on cancel. The widget should get
	 * hidden now. (back to welcome widget)
	 **/
	void signalCancelled();

protected:
	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }

private:
	void initKGame();

private:
	BosonStartEditorWidgetPrivate* d;
	BosonStartupNetwork* mNetworkInterface;
};

#endif
