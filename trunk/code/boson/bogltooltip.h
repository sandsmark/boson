/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOGLTOOLTIP_H
#define BOGLTOOLTIP_H

#include <qobject.h>

class BosonBigDisplayBase;
class BosonItem;
class QString;

class BoGLToolTipPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGLToolTip : public QObject
{
	Q_OBJECT
public:
	BoGLToolTip(BosonBigDisplayBase*);
	virtual ~BoGLToolTip();

	virtual bool eventFilter(QObject* o, QEvent* e);

	/**
	 * @return TRUE when the @ref currentTip should get displayed in this
	 * frame.
	 **/
	bool showTip() const { return mShowTip; }

	const QString& currentTip() const { return mCurrentTip; }

	/**
	 * Initialize the global/static tip manager. This is done automatically
	 * in the BoGLToolTip c'tor, so you usually don't need to call this.
	 **/
	static void initTipManager();

	static int toolTipDelay();

	/**
	 * Add a tip that gets displayed for every item which @ref
	 * BosonItem::rtti equals rtti. Note that you can override this tip if
	 * you add a tip providing the pointer of the @ref BosonItem
	 * @param rtti The @ref BosonItem::rtti this tip is displayed for
	 * @param tip Well... the tip!
	 **/
	static void add(int rtti, const QString& tip);

	/**
	 * Add a tip for a single item only. Note that this tip does
	 * <em>not</em> get removed once the unit is deleted! Use @ref remove in
	 * your destructor!
	 *
	 * This tip overrides any rtti-dependant tip.
	 * @param item The @ref BosonItem this tip is displayed for
	 * @param tip Well... the tip!
	 **/
	static void add(BosonItem* item, const QString& tip);
	static void remove(BosonItem* item);
	static void remove(int rtti);

	static void ignore(int rtti);
	static void ignore(BosonItem* item);
	static void unignore(int rtti);
	static void unignore(BosonItem* item);
	
protected:
	void hideTip();

protected slots:
	void slotTimeOut();

private:
	BoGLToolTipPrivate* d;
	BosonBigDisplayBase* mView;
	QString mCurrentTip;
	bool mShowTip;
};

#endif
