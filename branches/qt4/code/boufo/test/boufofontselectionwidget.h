/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!

#ifndef BOUFOFONTSELECTIONWIDGET_H
#define BOUFOFONTSELECTIONWIDGET_H

#include "boufofontselectionwidgetbase.h"

class BoUfoFontSelectionWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoFontSelectionWidget : public BoUfoFontSelectionWidgetBase
{
	Q_OBJECT
public:
	BoUfoFontSelectionWidget(BoUfoManager* manager);
	~BoUfoFontSelectionWidget();

	const BoUfoFontInfo& fontInfo() const;

signals:
	void signalFontSelected(const BoUfoFontInfo&);

protected slots:
	virtual void slotFontChanged();
	virtual void slotStyleChanged();
	virtual void slotSizeChanged();
	virtual void slotSizeChangedCombo();

	virtual void slotOk();
	virtual void slotApply();

protected:
	void updateFont();
	void setSupportedStyles(int supportedStyles);
	int selectedStyles() const;
	void setSelectedStyles(int styles);

private:
	BoUfoFontSelectionWidgetPrivate* d;
};

#endif
