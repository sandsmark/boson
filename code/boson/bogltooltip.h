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
class BoToolTipCreator;

class BoToolTipCreatorFactoryPrivate;
/**
 * This class is used on construction of @ref BoGLToolTips. It just creates an
 * object of @ref BoToolTipCreator.
 *
 * When you subclass @ref BoToolTipCreator you need to
 *
 * @li register your new class using @ref registerTipCreator
 * @li return a new object of your class in @ref tipCreator
 *
 * Your subclass of @ref BoToolTipCreator will automatically show up in any
 * configuration widget.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoToolTipCreatorFactory
{
public:
	BoToolTipCreatorFactory();
	~BoToolTipCreatorFactory();

	/**
	 * @return A list of all type IDs that have been specified to @ref
	 * registerTipCreator. Use @ref tipCreatorName to get a name for the
	 * type.
	 **/
	QValueList<int> availableTipCreators() const;

	/**
	 * @return A name for the @ref BoToolTipCreator @p type. This name can
	 * be used e.g. in a @ref QComboBox.
	 **/
	QString tipCreatorName(int type) const;

	/**
	 * @return A @ref BoToolTipCreator object of the class that is assigned to @p type.
	 **/
	BoToolTipCreator* tipCreator(int type) const;

protected:
	/**
	 * @param name A short name for your @ref BoToolTipCreator class, as it should
	 * show up in a configuration widget (e.g. a @ref QComboBox)
	 **/
	void registerTipCreator(int type, const QString& name);

private:
	BoToolTipCreatorFactoryPrivate* d;
};


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
	 * Set which kind of tooltips will be created. See @ref
	 * BoToolTipCreatorFactory and @ref BoToolTipCreator for more
	 * information.
	 **/
	void setToolTipCreator(int type);

	/**
	 * Render the tooltip at the cursor position @p cursorX and @p cursor Y.
	 * An offset is added to that cursor position and the "best" direction
	 * (left to the cursor / right to the cursor, ...) is chosen.
	 * @param viewportMatrix The current viewport matrix as it is used by
	 * the @ref BosonBigDisplayBase widget that was specified in the
	 * construcotr.
	 * @param font The desired font for the tooltip
	 **/
	void renderToolTip(int cursorX, int cursorY, const int* viewportMatrix, BosonGLFont* font);

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

	/**
	 * Ensure that @p item is <em>not</em> currently shown. This should
	 * happen before the item is deleted.
	 **/
	void unsetItem(BosonItem* item);

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
	BoToolTipCreator* mCreator;
	bool mShowTip;
	int mUpdatePeriod;
};

#endif
