/*
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!

#ifndef BOCOLORCHOOSER_H
#define BOCOLORCHOOSER_H

#include <bocolorchooserbase.h>

template<class T> class QValueList;
template<class T> class QPtrList;

class BoColorChooserPrivate;

class BoColorChooser : public BoColorChooserBase
{
	Q_OBJECT
public:
	/**
	 * @param colors Ignored, if you provide less colors than necessary (10)
	 **/
	BoColorChooser(const QValueList<QColor>& colors, QWidget* parent = 0, const char* name = 0);
	BoColorChooser(QWidget* parent = 0, const char* name = 0);
	~BoColorChooser();

	void setTaken(const QColor& color, bool taken = true);
	void setAllTaken(bool taken = true);

	/**
	 * @param colors A list of colors - there must be at least as many
	 * colors in this list as there are color-labels in this widget (i.e.
	 * 10).
	 **/
	void setColors(const QValueList<QColor>& colors);

protected:
	void applyColors();

	/**
	 * @return The color that is display if a certain color is already
	 * taken.
	 **/
	QColor takenColor() const;

private:
	void init();

private:
	BoColorChooserPrivate* d;
};

#endif
