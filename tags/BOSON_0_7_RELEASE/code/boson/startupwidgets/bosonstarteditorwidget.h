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

class BosonStartEditorWidget : public QWidget
{
	Q_OBJECT
public:
	BosonStartEditorWidget(TopWidget* top, QWidget* parent);
	~BosonStartEditorWidget();

public slots:
	void slotStart();
	/**
	 * Called when user clicks on "Cancel" button
	 * Cancels game starting and closes (hides) this widget 
	 **/
	void slotCancel();

signals:
	void signalStartGame();
	void signalCancelled();

protected slots:
	void slotMapChanged(const QString& mapIdentifier);
	void slotMyMapChanged(int index);

protected:
	void sendNewGame();

	void initNewMap();

private:
	void initKGame();
	void initPlayer();
	void initMaps();
	void initTileSets();
	void initSpecies();
	Player* player() const; // FIXME: do NOT use here! we should add dummy AI players for *all* players (even local)
	BosonPlayField* playField() const;

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

	QMap<int, QString> mMapIndex2Identifier;

	TopWidget* mTop;
};

#endif
