/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOMATRIXWIDGET_H
#define BOMATRIXWIDGET_H

#include <qwidget.h>

#include <lib3ds/types.h>

class BoMatrix;

class BoMatrixWidgetPrivate;
/**
 * Display a matrix. We use a @ref QGrid here, in order to display it like this:
 * <pre>
 * 1 0  0   0
 * 0 10 0   0
 * 0 0  100 0
 * 0 0  0   1
 * </pre>
 * and not like this
 * <pre>
 * 1 0 0 0
 * 1 10 0 0
 * 1 0 0 100 0
 * 1 0 0 1
 * </pre>
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMatrixWidget : public QWidget
{
	Q_OBJECT
public:
	BoMatrixWidget(QWidget* parent = 0, const char* name = 0);
	~BoMatrixWidget();

	void setMatrix(const BoMatrix* m);
	void setMatrix(const BoMatrix& m);
	void setMatrix(Lib3dsMatrix m);
	void setIdentity();
	void clear();

	const BoMatrix& matrix() const;

	void mark(unsigned int i);
	void unmark(unsigned int i);

	void setPrecision(int prec = 6) { mPrecision = prec; }
	int precision() const { return mPrecision; }

	/**
	 * Compare the of this widget matrix with @p matrix and @ref mark all
	 * elements, that don't match.
	 *
	 * @param diff The maximal differences that is treated as "equal". A
	 * perfect match happens rarely with floating point numbers, so using
	 * 0.0f is usually a bad idea.
	 **/
	bool compareMatrices(const BoMatrix& matrix, float diff = 0.001f);

	/**
	 * @overload
	 * This will use the matrix of @p widget as 2nd matrix and @ref mark the
	 * elements in both widgets.
	 **/
	bool compareMatrices(BoMatrixWidget* widget, float diff = 0.001f);

private:
	BoMatrixWidgetPrivate* d;
	int mPrecision;
};


#endif
