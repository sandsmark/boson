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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef EDITORRANDOMMAPWIDGET_H
#define EDITORRANDOMMAPWIDGET_H

#include "boufo.h"

class BosonCanvas;
class PlayerIO;
class bofixed;
template<class T> class QValueList;
template<class T1, class T2> class QPair;

class MyMap;

class EditorRandomMapWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorRandomMapWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	EditorRandomMapWidget();
	~EditorRandomMapWidget();

	void setCanvas(const BosonCanvas* c)
	{
		mCanvas = c;
	}
	const BosonCanvas* canvas() const
	{
		return mCanvas;
	}
	void setLocalPlayerIO(PlayerIO* io)
	{
		mLocalPlayerIO = io;
	}
	PlayerIO* localPlayerIO() const
	{
		return mLocalPlayerIO;
	}

protected slots:
	void slotCreateTerrain();
	void slotCreateMountains();

	void slotUpdateHeightProbabilityLabels();
	void slotUpdateMountainProbabilityLabels();
	void slotTerrainCreationChanged(BoUfoRadioButton*);
	void slotMountainCreationChanged(BoUfoRadioButton*);

protected:
	void createHeightsSimple(MyMap&);
	void createHeightsDiamondSquare(MyMap&);
	void createMountainSimple(MyMap& map, const QPoint& start);
	void createMountainParticleDeposition(MyMap& map, const QPoint& start);
	void createMountainDiamondSquare(MyMap& map, const QPoint& start);

private:
	void initTerrainCreationGUI(BoUfoWidget* parent);
	void initMountainCreationGUI(BoUfoWidget* parent);

private:
	EditorRandomMapWidgetPrivate* d;
	const BosonCanvas* mCanvas;
	PlayerIO* mLocalPlayerIO;
};


#endif

