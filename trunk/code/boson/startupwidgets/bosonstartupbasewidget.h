/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONSTARTUPBASEWIDGET_H
#define BOSONSTARTUPBASEWIDGET_H

#include <qwidget.h>

class QLabel;

class BosonStartupBaseWidget : public QWidget
{ 
	Q_OBJECT
public:
	BosonStartupBaseWidget(QWidget* parent, const char* name = 0);
	~BosonStartupBaseWidget();

	/**
	 * Needs to be called once all widgets of the derived class are
	 * constructed. TODO: maybe modify the style() instead and ensure that
	 * if style changes it is also modified. Would be a <em>lot</em>
	 * cleaner than this hack
	 **/
	void initBackgroundOrigin();

	/**
	 * @return The content widget - use this as parent for all widgets of
	 * the derived class!
	 **/
	QWidget* plainWidget() const
	{
		return mPlainWidget;
	}

protected:

private:
	QWidget* mPlainWidget;
	QLabel* mLogo;
};

#endif
