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
class BosonGLFont;

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

	/**
	 * Render the tooltip at the cursor position @p cursorX and @p cursor Y.
	 * An offset is added to that cursor position and the "best" direction
	 * (left to the cursor / right to the cursor, ...) is chosen.
	 * @param viewportMatrix The current viewport matrix as it is used by
	 * the @ref BosonBigDisplayBase widget that was specified in the
	 * construcotr.
	 * @param font The desired font for the tooltip
	 **/
	void renderToolTip(int cursorX, int cursorY, int* viewportMatrix, BosonGLFont* font);

	/**
	 * @return TRUE when the @ref currentTip should get displayed in this
	 * frame.
	 **/
	bool showTip() const { return mShowTip; }

	/**
	 * @return How many ms it takes for a tooltip to be shown, i.e. for how
	 * long the mouse must not be moved.
	 **/
	static int toolTipDelay();

	/**
	 * @return How often the tooltip data is updated. E.g. if this returns 5
	 * then every 5 ms the data is updated. See @ref slotUpdate
	 **/
	int updatePeriod() const { return mUpdatePeriod; }

	/**
	 * See @ref updatePeriod and @ref slotUpdate.
	 **/
	void setUpdatePeriod(int ms);

	virtual bool eventFilter(QObject* o, QEvent* e);

protected:
	void hideTip();

protected slots:
	/**
	 * Called when @ref toolTipDelay has expired (i.e. the mouse has not
	 * been moved for @ref toolTipDelay ms). This slot will check whether
	 * there is a relevant item below the cursor on the @ref
	 * BosonBigDisplayBase widget. If there is one it will update the
	 * tooltip (in case that item was not under the cursor when this slot
	 * had been called the last time).
	 *
	 * If a tooltip is meant to be displayed @ref showTip will be set to
	 * TRUE. Use @ref renderToolTip to render the tip.
	 *
	 * Also see @ref slotUpdate, which takes care that the displayed data
	 * remains current.
	 **/
	void slotTimeOut();

	/**
	 * This slot gets called every @ref updatePeriod ms. It will update the
	 * data about the currently displayed tooltip. I.e. if the tooltip is
	 * for a unit, the data about that unit is updated. Note that this slot
	 * does not check whether the unit is still under the cursor.
	 *
	 * This slot is used to display dynamic data, to keep it fast. You don't
	 * need to update the tip whenever it is rendered to display the current
	 * health (e.g.). An internal timer takes care of updating it regulary.
	 **/
	void slotUpdate();

private:
	BoGLToolTipPrivate* d;
	BosonBigDisplayBase* mView;
	bool mShowTip;
	int mUpdatePeriod;
};

#endif
