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

#ifndef BOUFOCOLORCHOOSER_H
#define BOUFOCOLORCHOOSER_H

#include "../boufo/boufo.h"

class QColor;
template<class T> class QValueVector;
template<class T> class QValueList;

class BoUfoColorChooserPrivate;
class BoUfoColorChooser : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoColorChooser();
	~BoUfoColorChooser();

	void showButtons(int count);
	unsigned int buttonCount() const;

	/**
	 * Note that this does not influence whether a button is taken!
	 * The taken flag applies to a button, not to a color.
	 **/
	void setColors(const QValueVector<QColor>& colors);
	void setColors(const QValueList<QColor>& colors);

	void setTaken(int index, bool taken);
	/**
	 * @overload
	 **/
	void setTaken(const QColor& color, bool taken);
	bool isTaken(int index) const;
	void setAllTaken(bool taken);

	int indexOfColor(const QColor&) const;
	QColor colorOfIndex(int i) const;

	void highlightColor(int i);
	void highlightColor(const QColor&);
	void unhighlightColor(int i);
	void unhighlightColor(const QColor&);

signals:
	void signalColorSelected(int index);
	void signalColorSelected(const QColor& color);

protected:
	void applyColors();

protected slots:
	void slotButtonClicked(int i);
	void slotMouseButtonReleased(QMouseEvent* e);

private:
	BoUfoColorChooserPrivate* d;
};

#endif

