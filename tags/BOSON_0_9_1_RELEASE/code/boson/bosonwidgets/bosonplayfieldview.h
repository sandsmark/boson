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

#ifndef BOSONPLAYFIELDVIEW_H
#define BOSONPLAYFIELDVIEW_H

#include <klistview.h>

class BosonPlayFieldView;

class BosonPlayFieldItem : public QListViewItem
{
public:
	enum RTTI {
		PlayFieldItem = 2000,
		PlayFieldNewMapItem = 2001
	};
public:
	BosonPlayFieldItem(const QString& playFieldIdentifier, BosonPlayFieldView* parent);
	BosonPlayFieldItem(const QString& playFieldIdentifier, BosonPlayFieldItem* parent);
	~BosonPlayFieldItem();

	/**
	 * Currently equal to setText(0, text);
	 **/
	void setPlayFieldName(const QString& text);

	virtual int rtti() const { return PlayFieldItem; }

	const QString& playFieldIdentifier() const { return mPlayFieldIdentifier; }

private:
	void init(const QString& playFieldIdentifier);

private:
	QString mPlayFieldIdentifier;
};

/**
 * This is a specialized version of a @ref BosonPlayFieldItem, which uses
 * QString::null as playfield identifier. The playfield name is i18n("New Map").
 *
 * The @ref BosonPlayFieldView should display a BosonPlayFieldNewMapItem as
 * first item in the view.
 **/
class BosonPlayFieldNewMapItem : public BosonPlayFieldItem
{
public:
	BosonPlayFieldNewMapItem(BosonPlayFieldView* parent);
	BosonPlayFieldNewMapItem(BosonPlayFieldItem* parent);
	~BosonPlayFieldNewMapItem();

	virtual int rtti() const { return PlayFieldNewMapItem; }

private:
	void init();
};

class BosonPlayFieldViewPrivate;

class BosonPlayFieldView : public KListView
{
	Q_OBJECT
public:
	BosonPlayFieldView(QWidget* parent = 0, const char* name = 0);
	~BosonPlayFieldView();

private:
	BosonPlayFieldViewPrivate* d;
};

#endif
