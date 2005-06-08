/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOUFOSTARTEDITORWIDGET_H
#define BOUFOSTARTEDITORWIDGET_H

#include "boufostarteditorwidgetbase.h"

class BosonStartupNetwork;
class BosonPlayField;
class Player;
class KPlayer;
class KGamePropertyBase;

class BoUfoStartEditorWidgetPrivate;
class BoUfoStartEditorWidget : public BoUfoStartEditorWidgetBase
{
	Q_OBJECT
public:
	BoUfoStartEditorWidget(BosonStartupNetwork* interface);
	~BoUfoStartEditorWidget();

public slots:
	virtual void slotStartClicked();

signals:
	void signalCancelled();

protected:
	/**
	 * Create a new map, according to the settings in the widget (such as
	 * width/height, groundtype, ...)
	 **/
	QByteArray createNewMap();

protected slots: // implementations for the .ui slots
	// these slots describe actions that the local player has executed in
	// his widget. nearly all must be transferred over network before the
	// actual action is performed!
	virtual void slotPlayFieldChanged(int, int);
	virtual void slotGroundThemeChanged(int);
	virtual void slotMaxPlayersChanged(float);
	virtual void slotMaxPlayersChanged(int);
	virtual void slotWidthChanged(float);
	virtual void slotWidthChanged(int);
	virtual void slotHeightChanged(float);
	virtual void slotHeightChanged(int);
	virtual void slotNewMapToggled(bool);
	virtual void slotCreateNewToggled(bool checked);
	virtual void slotEditExistingToggled(bool checked);
	virtual void slotCancel();

private slots:
	void slotNetStart();

	void slotNetPlayFieldChanged(BosonPlayField* field);

protected:
	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }
private:
	void initPlayFields();
	void initGroundThemes();

private:
	void initKGame();

private:
	BoUfoStartEditorWidgetPrivate* d;
	BosonStartupNetwork* mNetworkInterface;
};

#endif

