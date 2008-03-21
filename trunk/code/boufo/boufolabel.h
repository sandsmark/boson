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
#ifndef BOUFOLABEL_H
#define BOUFOLABEL_H

#include "boufowidget.h"

class BoUfoLabel : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(QString iconFile READ iconFile WRITE setIconFile);
public:
	BoUfoLabel();
	BoUfoLabel(const QString& text);

	ufo::ULabel* label() const
	{
		return mLabel;
	}

	QString text() const;
	void setIcon(const BoUfoImage&);
	void setIcon(const BoUfoDrawable& drawable);

	/**
	 * Convenience method that calls @ref setIcon.
	 **/
	void setIconFile(const QString&);

	/**
	 * @return The filename used in @ref setIconFile. Note that if you set
	 * an icon through other methods (e.g. by using @ref setIcon directly)
	 * the returned value of this method is undefined.
	 **/
	QString iconFile() const;

	virtual void setOpaque(bool o);

	static void setDefaultForegroundColor(const QColor& color);
	static const QColor& defaultForegroundColor();

	/**
	 * See @ref BoUfoWidget::setVerticalAlignment
	 **/
	virtual void setVerticalAlignment(VerticalAlignment alignment);

	/**
	 * See @ref BoUfoWidget::setHorizontalAlignment
	 **/
	virtual void setHorizontalAlignment(HorizontalAlignment alignment);

public slots:
	void setText(const QString& text);

protected:
	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

private:
	void init();

private:
	ufo::ULabel* mLabel;
	QString mIconFile;

	static QColor mDefaultForegroundColor;
};

#endif
