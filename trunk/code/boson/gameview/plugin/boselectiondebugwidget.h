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
#ifndef BOSELECTIONDEBUGWIDGET_H
#define BOSELECTIONDEBUGWIDGET_H

#include "boufo.h"

class BosonCanvas;
class PlayerIO;
class BoSelection;
class Unit;
class bofixed;
template<class T> class QValueList;
template<class T1, class T2> class QPair;

class BoSelectionDebugWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoSelectionDebugWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoSelectionDebugWidget();
	~BoSelectionDebugWidget();

	void setLocalPlayerIO(PlayerIO* io)
	{
		mLocalPlayerIO = io;
	}
	PlayerIO* localPlayerIO() const
	{
		return mLocalPlayerIO;
	}
	void setSelection(BoSelection* selection)
	{
		mSelection = selection;
	}
	BoSelection* selection() const
	{
		return mSelection;
	}

	void update();

private:
	BoSelectionDebugWidgetPrivate* d;
	PlayerIO* mLocalPlayerIO;
	BoSelection* mSelection;
};


class BoSelectionGroupDebugWidgetPrivate;
/**
 * @internal
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoSelectionGroupDebugWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoSelectionGroupDebugWidget();
	~BoSelectionGroupDebugWidget();

	void update(BoSelection*);

private:
	BoSelectionGroupDebugWidgetPrivate* d;
};


class BoUnitDebugWidgetPrivate;
/**
 * @internal
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUnitDebugWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUnitDebugWidget();
	~BoUnitDebugWidget();

	void update(Unit*);

private:
	BoUnitDebugWidgetPrivate* d;
};


class BoUnitXMLDebugWidgetPrivate;
/**
 * @internal
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUnitXMLDebugWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUnitXMLDebugWidget();
	~BoUnitXMLDebugWidget();

	void update(Unit*);

private:
	BoUnitXMLDebugWidgetPrivate* d;
};


#endif

