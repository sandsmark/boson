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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOINTERNALFRAME_H
#define BOUFOINTERNALFRAME_H

#include "boufowidget.h"

/**
 * Note: in libufo @ref ufo::UInternalFrame objects have to be added using @ref
 * ufo::URootPane::addFrame, so they are special cases. You cannot add them to
 * other widgets. Therefore you have to add a BoUfoInternalFrame object using
 * @ref BoUfoManager::addFrame. This is done automatically by the constructor.
 **/
class BoUfoInternalFrame : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoInternalFrame(BoUfoManager* manager, const QString& title = QString::null);
	~BoUfoInternalFrame();

	void setBounds(int x, int y, int w, int h);
	void setTitle(const QString& title);
	QString title() const;
	void setResizable(bool r);
	bool resizable() const;

	/**
	 * See @ref ufo::UInternalFrame::pack. This adjusts the size of the
	 * widget so that it fits to its contents.
	 **/
	void adjustSize();

	/**
	 * @reimplemented
	 **/
	virtual void setLayoutClass(LayoutClass c)
	{
		contentWidget()->setLayoutClass(c);
	}

	/**
	 * @reimplemented
	 **/
	virtual void addWidget(BoUfoWidget* w)
	{
		contentWidget()->addWidget(w);
	}

	/**
	 * @reimplemented
	 **/
	virtual void addSpacing(int spacing)
	{
		contentWidget()->addSpacing(spacing);
	}

	ufo::UInternalFrame* frame() const
	{
		return (ufo::UInternalFrame*)ufoWidget();
	}
	ufo::URootPane* rootPane() const
	{
		return mRootPane;
	}

	/**
	 * You should prefer @ref contentWidget instead, which returns the same
	 * widget.
	 **/
	ufo::UWidget* contentPane() const
	{
		return mContentPane;
	}

	/**
	 * @return A BoUfo wrapper around @ref contentPane. Use this instead of
	 * @ref contentPane.
	 **/
	BoUfoWidget* contentWidget() const
	{
		return mContentWidget;
	}


private:
	void init();

private:
	ufo::URootPane* mRootPane;
	ufo::UWidget* mContentPane;
	BoUfoWidget* mContentWidget;
};


#endif
