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

#include "bosonstartwidgetbase.h"
#include <qwidget.h>
#include <qmap.h>

class QVBoxLayout;
class QGridLayout;
class QVGroupBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class KIntNumInput;

class Boson;
class Player;
class KPlayer;
class KGamePropertyBase;
class TopWidget;
class BosonPlayField;

class BosonStartEditorWidget : public BosonStartWidgetBase
{
	Q_OBJECT
public:
	BosonStartEditorWidget(QWidget* parent);
	~BosonStartEditorWidget();

public slots:
	void slotStart();

signals:
	void signalSetLocalPlayer(Player*);

protected slots:
	virtual void slotSendPlayFieldChanged(int index);

protected:
	virtual void setCurrentPlayField(BosonPlayField* field);
	virtual void sendNewGame();

	void initNewMap();

private:
	void initKGame();
	void initPlayFields();
	void initTileSets();
	void initSpecies();

private:
	QVBoxLayout* mTopLayout;

	QComboBox* mMapCombo;
	QVGroupBox* mMapBox; // information (size, tileset, ...) about the map
	QComboBox* mTileSetCombo;
	KIntNumInput* mMapWidth;
	KIntNumInput* mMapHeight;
	KIntNumInput* mMaxPlayers;

	QPushButton* mCancelButton;
	QPushButton* mStartGameButton;
};

#endif
