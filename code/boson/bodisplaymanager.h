/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BODISPLAYMANAGER_H
#define BODISPLAYMANAGER_H

#include <qwidget.h>

class BoBox;
class BosonBigDisplayBase;
class UnitBase;
class BosonCursor;
class Player;
class PlayerIO;
class Unit;
class BoSelection;
class BoSpecificAction;
class BoFontInfo;

class KPlayer;
template<class T> class QPtrList;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoDisplayManager : public QWidget
{
	Q_OBJECT
public:
	/**
	 **/
	BoDisplayManager(QWidget* parent);
	~BoDisplayManager();

	BosonBigDisplayBase* addInitialDisplay();
	BosonBigDisplayBase* activeDisplay() const;

public slots:

	/**
	 * Called by @ref Boson::signalAdvance.
	 *
	 * Note that it is <em>not</em> ensured, that @ref
	 * BosonCanvas::slotAdvance is called first. It might be possible that
	 * this slot gets called before @ref BosonCanvas::slotAdvance but the
	 * other way round might be possible as well.
	 *
	 * Also note that this should <em>not</em> be used for game logic parts
	 * that the network might depend on. Use it for OpenGL or similar
	 * operations (input/output on the local client) only.
	 **/
	void slotAdvance(unsigned int, bool);

	void slotAction(const BoSpecificAction&);

	void slotSetGrabMovie(bool grab);

protected:
	void grabMovieFrame();

private:
	class BoDisplayManagerPrivate;
	BoDisplayManagerPrivate* d;
};

#endif
