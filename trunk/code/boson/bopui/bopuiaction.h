/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOPUIACTION_H
#define BOPUIACTION_H

#include <qobject.h>

class BoPUIActionCollection;
class BosonGLWidget;
class KShortcut;
class puMenuBar;

template<class T> class QValueList;

class BoPUIActionPrivate;
// this is coded to be similar to KAction
class BoPUIAction : public QObject
{
	Q_OBJECT
public:
	BoPUIAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoPUIActionCollection* parent, const char* name);
	~BoPUIAction();

	const QString& text() const;

public slots:
	/**
	 * Just emit @ref signalActivated
	 **/
	void slotActivated();

signals:
	void signalActivated();

private:
	void init(BoPUIActionCollection* parent, const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot);

private:
	BoPUIActionPrivate* d;
	BoPUIActionCollection* mParentCollection;
};

class BoPUIActionCollectionPrivate;
class BoPUIActionCollection : public QObject
{
	Q_OBJECT
public:
	BoPUIActionCollection(QObject* parent, const char* name);
	~BoPUIActionCollection();

	void insert(BoPUIAction* action);
	bool hasAction(const QString& name) const;
	BoPUIAction* action(const QString& name) const;

	bool createGUI(const QString& file);

	static void initActionCollection(BosonGLWidget* w);

private:
	BoPUIActionCollectionPrivate* d;
};

// a menu item
class BoPUIMenuBarItem : public QObject
{
	Q_OBJECT
public:
	BoPUIMenuBarItem(const QString& text, QObject* parent, const char* name = 0);
	~BoPUIMenuBarItem();

	const QString& text() const
	{
		return mText;
	}

	/**
	 * @return A char* pointer containing the @ref text in latin1
	 * representation. This pointer is persistent, i.e. it is valid as long
	 * as this BoPUIMenuBarItem object is alive.
	 **/
	const char* puiText() const
	{
		return mPUIText;
	}

	/**
	 * Called by the plib callback
	 **/
	void activate();

signals:
	void signalActivated();

private:
	QString mText;
	char* mPUIText;
};


class BoPUIMenuBarMenuPrivate;
class BoPUIMenuBarMenu : public BoPUIMenuBarItem
{
	Q_OBJECT
public:
	BoPUIMenuBarMenu(const QString& text, QObject* parent, const char* name = 0);
	~BoPUIMenuBarMenu();

	void addMenuItem(const QString& text, const QObject* receiver, const char* slot);
	void addSeparator();
	void addSubMenu(BoPUIMenuBarMenu* menu);

	/**
	 * @return The number of items in this menu. Not recursively, i.e.
	 * sub-menus are not counted.
	 **/
	unsigned int itemCount() const;

	void createPUISubMenu();

protected:
	QValueList<BoPUIMenuBarItem*>& items() const;

private:
	void clearPUI();

private:
	BoPUIMenuBarMenuPrivate* d;
};

class BoPUIMenuBarPrivate;
class BoPUIMenuBar : public BoPUIMenuBarMenu
{
	Q_OBJECT
public:
	BoPUIMenuBar(BoPUIActionCollection*, QObject* parent, const char* name = 0);
	~BoPUIMenuBar();

	void createMenu();

	static void initMenuBar(BosonGLWidget* w);

	puMenuBar* puiMenuBar() const;

protected:
	void clearPUI();

private:
	BoPUIMenuBarPrivate* d;
};


#endif

