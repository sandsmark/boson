/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef KGAMESPECIESDEBUG_H
#define KGAMESPECIESDEBUG_H

#include <qwidget.h>

class Boson;
class SpeciesTheme;

class SpeciesViewPrivate;
class SpeciesView : public QWidget
{
	Q_OBJECT
public:
	SpeciesView(QWidget* parent = 0);
	~SpeciesView();

	void setSpecies(SpeciesTheme* species);
	void update();

private:
	SpeciesViewPrivate* d;
};

class KGameSpeciesDebugPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameSpeciesDebug : public QWidget
{
	Q_OBJECT
public:
	KGameSpeciesDebug(QWidget* parent);
	~KGameSpeciesDebug();

	void loadSpecies();

protected slots:
	void slotChangeSpecies(int);

private:
	KGameSpeciesDebugPrivate* d;
};

#endif
