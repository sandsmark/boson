/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOUFOIMAGE_H
#define BOUFOIMAGE_H

#include <qobject.h>
//Added by qt3to4:
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <Q3ValueList>
#include <QKeyEvent>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
template<class T1, class T2> class QMap;
template<class T1> class Q3ValueList;
class QDomElement;

class BoUfoActionCollection;
class BoUfoMenuBar;
class BoUfoWidget;
class BoUfoInternalFrame;
class BoUfoLayeredPane;

namespace ufo {
	class UXToolkit;
	class UXDisplay;
	class UXContext;
	class URootPane;
	class UObject;
	class ULayoutManager;
	class UDimension;
	class UDrawable;
	class UImage;
	class UImageIO;
	class UPoint;
	class UFontInfo;
	class UFont;

	class UWidget;
	class UButton;
	class UCheckBox;
	class URadioButton;
	class UTextEdit;
	class UAbstractSlider;
	class USlider;
	class UListBox;
	class ULineEdit;
	class ULabel;
	class UComboBox;
	class UInternalFrame;
	class UBoProgress;
	class ULayeredPane;
	class UWidgetUI;
	class UButtonGroup;


	class UActionEvent;
	class UMouseEvent;
	class UMouseWheelEvent;
	class UWidgetEvent;
	class UKeyEvent;
	class UFocusEvent;
};


/**
 * Use @ref BoUfoImage instead.
 **/
class BoUfoImageIO
{
public:
	BoUfoImageIO();
	BoUfoImageIO(const QPixmap&);
	BoUfoImageIO(const QImage&);
	~BoUfoImageIO();

	void setPixmap(const QPixmap& p);
	void setImage(const QImage& img);

	ufo::UImageIO* imageIO() const
	{
		return mImageIO;
	}

private:
	void init();

private:
	ufo::UImageIO* mImageIO;
};


/**
 * Frontend to @ref ufo::UImage. Just create an object of this class with the
 * desired @ref QPixmap or @ref QImage and then use @ref image for your ufo
 * object.
 *
 * An object of this class references the @ref ufo::UImage object it holds and
 * unreferences it when it is destroyed. Therefore you can use the @ref image in
 * as many ufo object as you want.
 * Note that when the object of this class is destroyed, the @ref image is only
 * deleted when no ufo object holds a reference anymore, so you can delete this
 * object at any time.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoImage
{
public:
	BoUfoImage(); // NULL image
	BoUfoImage(const QPixmap&); // slowest
	BoUfoImage(const QImage&);
	BoUfoImage(const BoUfoImage&); // fastest (shallow copy)
	~BoUfoImage();

	/**
	 * Makes a shallow copy of @p img, i.e. only the image pointer is copied
	 * and reference()d.
	 **/
	BoUfoImage& operator=(const BoUfoImage& img);

	bool isNull() const
	{
		return (image() == 0);
	}

	unsigned int width() const;
	unsigned int height() const;

	void load(const QPixmap&);
	void load(const QImage&);
	void load(const BoUfoImage&);

	ufo::UImage* image() const
	{
		return mImage;
	}

	void paint();
	void paint(const QPoint& pos);
	void paint(const QPoint& pos, const QSize& size);
	void paint(const QRect& rect);

protected:
	void set(BoUfoImageIO*);
	void set(ufo::UImage*);

private:
	void init();

private:
	ufo::UImage* mImage;
};

#endif
