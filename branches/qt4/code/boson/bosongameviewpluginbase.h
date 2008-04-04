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

#ifndef BOSONGAMEVIEWPLUGINBASE_H
#define BOSONGAMEVIEWPLUGINBASE_H

#include <qobject.h>

class BoGLMatrices;
class BoUfoWidget;
class BosonCanvas;
class PlayerIO;
class BoSelection;

class BosonGameViewPluginBasePrivate;
class BosonGameViewPluginBase : public QObject
{
	Q_OBJECT
public:
	BosonGameViewPluginBase();
	virtual ~BosonGameViewPluginBase();

	virtual void init();

	BoUfoWidget* ufoWidget() const
	{
		return mUfoWidget;
	}

	virtual void quitGame() { }

	virtual void setGameMode(bool game) = 0;

	void setGameGLMatrices(const BoGLMatrices* m) { mGameGLMatrices = m; }
	const BoGLMatrices* gameGLMatrices() const { return mGameGLMatrices; }

	virtual void setCanvas(const BosonCanvas* c) { mCanvas = c; }
	const BosonCanvas* canvas() const { return mCanvas; }

	virtual void setLocalPlayerIO(PlayerIO* io) { mPlayerIO = io; }
	PlayerIO* playerIO() const { return mPlayerIO; }

	BoSelection* selection() const
	{
		return mSelection;
	}

	/**
	 * Called right before the widget is being painted.
	 *
	 * This can be used e.g. to hide certain widgets or to update some text.
	 **/
	virtual void updateBeforePaint() { }

protected slots:
	virtual void slotSelectionChanged(BoSelection*);

protected:
	virtual BoUfoWidget* createUfoWidget() const = 0;

private:
	BosonGameViewPluginBasePrivate* d;
	const BoGLMatrices* mGameGLMatrices;
	BoUfoWidget* mUfoWidget;
	const BosonCanvas* mCanvas;
	PlayerIO* mPlayerIO;
	BoSelection* mSelection;
};

#endif
