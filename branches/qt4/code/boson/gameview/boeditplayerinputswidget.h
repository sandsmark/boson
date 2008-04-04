/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOEDITPLAYERINPUTSWIDGET_H
#define BOEDITPLAYERINPUTSWIDGET_H

#include <qwidget.h>

class Player;

class BoEditPlayerInputsWidgetPrivate;
class BoEditPlayerInputsWidget : public QWidget
{
	Q_OBJECT
public:
	BoEditPlayerInputsWidget(QWidget* parent);
	~BoEditPlayerInputsWidget();

	void setPlayer(Player* p);

signals:
	void signalAddedLocalPlayerInput();
	void signalAddMenuInput();

protected slots:
	void slotAddIO();
	void slotRemoveIO();

protected:
	QString rttiString(int rtti) const;

private:
	BoEditPlayerInputsWidgetPrivate* d;
};

#endif

