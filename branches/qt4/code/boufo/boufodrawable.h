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
#ifndef BOUFODRAWABLE_H
#define BOUFODRAWABLE_H


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
 * Frontend to @ref ufo::UDrawable. You can simply subclass this class and
 * totally ignore @ref ufo::UDrawable. If you require a @ref ufo::UDrawable
 * pointer, just use @ref drawable which provides a pointer to the internal
 * drawable (which calls methods in this class only).
 **/
class BoUfoDrawable
{
public:
	BoUfoDrawable();
	virtual ~BoUfoDrawable();

	ufo::UDrawable* drawable() const
	{
		return mDrawable;
	}

	virtual void render(int x, int y, int w, int h) = 0;

	virtual int drawableWidth() const = 0;
	virtual int drawableHeight() const = 0;

private:
	ufo::UDrawable* mDrawable;
};

#endif
