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
#ifndef BOUFOACTION_H
#define BOUFOACTION_H

#include <qobject.h>

class BoUfoActionCollection;
class BosonGLWidget;
class KShortcut;
class BoUfoManager;

template<class T> class QValueList;
template<class T> class QPtrList;

namespace ufo {
	class UMenuBar;
	class UMenuItem;
	class UWidget;
	class UActionEvent;
};

class BoUfoActionPrivate;
/**
 * @short A simple @ref KAction like class that for libufo
 * @autor Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoAction : public QObject
{
	Q_OBJECT
public:
	BoUfoAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name);
	~BoUfoAction();

	const QString& text() const;

	virtual void plug(ufo::UWidget*);

public slots:
	/**
	 * Just emit @ref signalActivated
	 **/
	virtual void slotActivated();

protected:
	void addWidget(ufo::UWidget*);
	void removeWidget(ufo::UWidget*);
	const QPtrList<ufo::UWidget>& widgets() const;

signals:
	void signalActivated();

private:
	void init(BoUfoActionCollection* parent, const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot);

protected:
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);
	void uslotWidgetRemoved(ufo::UWidget*);

private:
	BoUfoActionPrivate* d;
	BoUfoActionCollection* mParentCollection;
};

class BoUfoToggleAction : public BoUfoAction
{
	Q_OBJECT
public:
	BoUfoToggleAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name);
	~BoUfoToggleAction();

	virtual void plug(ufo::UWidget*);

	void setChecked(bool);
	bool checked() const
	{
		return mChecked;
	}

public slots:
	virtual void slotActivated();

signals:
	void signalToggled(bool);

private:
	bool mChecked;
};

class BoUfoActionMenuPrivate;
class BoUfoActionMenu : public BoUfoAction
{
	Q_OBJECT
public:
	BoUfoActionMenu(const QString& text, BoUfoActionCollection* parent, const char* name);
	~BoUfoActionMenu();

	virtual void plug(ufo::UWidget*);

	// does NOT take ownership
//	void insert(BoUfoAction* a, int index); // TODO
	void insert(BoUfoAction* a);
	void remove(BoUfoAction* a);

	virtual void clear();

protected:
	void redoMenus();
	const QPtrList<BoUfoAction>& actions() const;

private:
	BoUfoActionMenuPrivate* d;
};

class BoUfoSelectActionPrivate;
class BoUfoSelectAction : public BoUfoActionMenu
{
	Q_OBJECT
public:
	BoUfoSelectAction(const QString& text, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name);
	~BoUfoSelectAction();

	virtual void clear();

	void setItems(const QStringList&);
	void setCurrentItem(int);

protected slots:
	void slotItemActivated();

signals:
	void signalActivated(int);

private:
	BoUfoSelectActionPrivate* d;
};

class BoUfoActionCollectionPrivate;
/**
 * An action collection, similar to @ref KActionCollection, but it provides only
 * the most basic things.
 *
 * In most applications you will just do
 * <pre>
 * BoUfoManager* ufoManager;
 * // ...
 * BoUfoActionCollection::initActionCollection(ufoManager);
 * BoUfoActionCollection* actionCollection = ufoManager->actionCollection();
 * // ... create your BoAction object with actionCollection as parent
 * actionCollection->createGUI("myui.rc");
 * </pre>
 *
 * Everything else is done internally. You should need the actionCollection
 * object for your @ref BoUfoAction objects only.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoActionCollection : public QObject
{
	Q_OBJECT
public:
	BoUfoActionCollection(QObject* parent, const char* name);
	~BoUfoActionCollection();

	void insert(BoUfoAction* action);
	bool hasAction(const QString& name) const;
	BoUfoAction* action(const QString& name) const;

	bool createGUI(const QString& file);

	static void initActionCollection(BoUfoManager* m);

private:
	BoUfoActionCollectionPrivate* d;
};

/**
 * @internal
 *
 * This represents a menu item from the xml file (the *ui.rc one). This class
 * does not store any ufo objects.
 **/
class BoUfoMenuBarItem : public QObject
{
	Q_OBJECT
public:
	BoUfoMenuBarItem(BoUfoAction* a, const QString& text, QObject* parent, const char* name = 0);
	~BoUfoMenuBarItem();

	const QString& text() const
	{
		return mText;
	}

	BoUfoAction* action() const
	{
		return mAction;
	}

signals:
	void signalActivated();

private:
	QString mText;
	BoUfoAction* mAction;
};


class BoUfoMenuBarMenuPrivate;
/**
 * @internal
 *
 * This represents a menu from the xml file (the *ui.rc one). This class
 * can create a libufo submenu structure (see @ref createUfoSubMenu), but this
 * class does not store any libufo objects. Once they were created, they exist
 * in the libufo data structures only.
 *
 * Also this class is NOT deleted when the libufo objects are deleted. However
 * it is deleted when the Qt parent is deleted.
 **/
class BoUfoMenuBarMenu : public BoUfoMenuBarItem
{
	Q_OBJECT
public:
	BoUfoMenuBarMenu(const QString& text, QObject* parent, const char* name = 0);
	~BoUfoMenuBarMenu();

	void addMenuItem(const QString& text, BoUfoAction* receiver, const char* slot);
	void addSeparator();
	void addSubMenu(BoUfoMenuBarMenu* menu);

	/**
	 * @return The number of items in this menu. Not recursively, i.e.
	 * sub-menus are not counted.
	 **/
	unsigned int itemCount() const;

	void createUfoSubMenu(ufo::UWidget*);

protected:
	const QValueList<BoUfoMenuBarItem*>& items() const;

private:
	BoUfoMenuBarMenuPrivate* d;
};

class BoUfoMenuBarPrivate;
/**
 * You should never need this class, it is used internally.
 **/
class BoUfoMenuBar : public BoUfoMenuBarMenu
{
	Q_OBJECT
public:
	BoUfoMenuBar(BoUfoManager* parent, const char* name = 0);
	~BoUfoMenuBar();

	void createMenu();

	static void initMenuBar(BoUfoManager* m);

	ufo::UMenuBar* ufoMenuBar() const;

protected:
	void clearUfo();

private:
	BoUfoMenuBarPrivate* d;
};


#endif

