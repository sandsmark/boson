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
#ifndef BOUFOACTION_H
#define BOUFOACTION_H

#include <qobject.h>

class BoUfoActionCollection;
class BosonGLWidget;
class KShortcut;
class KAccel;
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
 *
 * Note that in contrast to the classes derived of @ref BoUfoWidget you can
 * delete objects of this class yourself.
 * @autor Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoAction : public QObject
{
	Q_OBJECT
public:
	BoUfoAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name);
	~BoUfoAction();

	void setText(const QString& text);
	const QString& text() const;

	void setEnabled(bool e);
	bool isEnabled() const
	{
		return mIsEnabled;
	}

	virtual void plug(ufo::UWidget*);
	virtual void unplug();

	/**
	 * @internal
	 **/
	void setParentCollection(BoUfoActionCollection* parent)
	{
		mParentCollection = parent;
	}
	void insertToKAccel(KAccel* accel);

public slots:
	/**
	 * Just emit @ref signalActivated
	 **/
	virtual void slotActivated();

	virtual void slotHighlighted();

protected:
	void addWidget(ufo::UWidget*);
	void removeWidget(ufo::UWidget*, bool del = true);
	const QPtrList<ufo::UWidget>& widgets() const;

signals:
	void signalActivated();
	void signalEnabled(bool);

private:
	void init(BoUfoActionCollection* parent, const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot);
	void initShortcut();

public: // g++ 3.4 complains if we make this protected
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	BoUfoActionPrivate* d;
	BoUfoActionCollection* mParentCollection;
	bool mIsEnabled;

	friend class BoUfoActionDeleter; // needs to access removeWidget()
};

class BoUfoToggleAction : public BoUfoAction
{
	Q_OBJECT
public:
	BoUfoToggleAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name);
	~BoUfoToggleAction();

	virtual void plug(ufo::UWidget*);

	virtual void setChecked(bool);
	bool isChecked() const
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
	void insert(BoUfoAction* a, int id = -1, int index = -1);
	void remove(BoUfoAction* a);
	void insertItem(const QString&, int id = -1, int index = -1);

	BoUfoAction* item(int id) const;
	int itemId(BoUfoAction*) const;

	virtual void clear();

	/**
	 * Does nothing by default. Reimplemented by @ref BoUfoSelectAction
	 **/
	virtual void setCurrentItem(int id) { Q_UNUSED(id); }

protected:
	void redoMenus();
	const QPtrList<BoUfoAction>& actions() const;

signals:
	void signalActivated(int id);

protected slots:
	void slotMenuItemActivated();

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
	virtual void setCurrentItem(int id);

	/**
	 * @return The Id of the current item. See @ref setCurrentItem
	 **/
	int currentItem() const;

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
 * // ... create your BoUfoAction object with actionCollection as parent
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
	BoUfoActionCollection(BoUfoActionCollection* parentCollection, QObject* parent, const char* name);
	~BoUfoActionCollection();

	void insert(BoUfoAction* action);
	void remove(BoUfoAction* action, bool deleteIt = true);
	bool hasAction(const QString& name) const;
	BoUfoAction* action(const QString& name) const;

	void clearActions();

	void setActionEnabled(const QString& name, bool);

	void setGUIFiles(const QStringList& fileList);
	const QStringList& guiFiles() const;

	/**
	 * Create a GUI according to the current @ref guiFiles. If this
	 * actioncollection is a child of another actioncollection, the
	 * implementation of the parent actioncollection is called.
	 **/
	bool createGUI();

	/**
	 * @overload
	 * This method calls @ref setGUIFiles with the specified files
	 **/
	bool createGUI(const QStringList& guiFiles);
	bool createGUI(const QString& guiFile);

	void setAccelWidget(QWidget* widget);
	KAccel* kaccel() const;

	static void initActionCollection(BoUfoManager* m);

protected:
	void registerChildCollection(BoUfoActionCollection* c);
	void unregisterChildCollection(BoUfoActionCollection* c);

private:
	void init();

private:
	BoUfoActionCollectionPrivate* d;
};


// AB: heavily based on KStdAction/KStdGameAction
class BoUfoStdAction
{
public:
	enum StdAction {
		// File menu
		FileNew = 1, FileOpen, FileSave, FileSaveAs,
		FileClose, FileQuit,

		// Game menu
		GameNew, GameLoad, GameSave, GameSaveAs,
		GameEnd, GamePause, GameQuit,

		// Edit Menu
		EditUndo, EditRedo,

		// Settings menu
		ShowMenubar, ShowToolbar, ShowStatusbar,
		KeyBindings, Preferences,
		
		FullScreen,
		ActionNone
	};

	static BoUfoAction* create(StdAction id,
			const QObject* receiver, const char* slot,
			BoUfoActionCollection* parent, const char* name);

	/**
	 * @return The name for @p id as it appears in the ui.rc file. E.g.
	 * "game_quit" for GameQuit.
	 **/
	static const char* name(StdAction id);

	/**
	 * @return The label that is used by default for @p id.
	 **/
	static const char* label(StdAction id);


	static BoUfoAction* fileNew(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* fileOpen(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* fileSave(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* fileSaveAs(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* fileClose(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* fileQuit(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);

	static BoUfoAction* gameNew(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* gameLoad(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* gameSave(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* gameSaveAs(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* gameEnd(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoToggleAction* gamePause(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* gameQuit(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* editUndo(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* editRedo(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoToggleAction* showMenubar(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoToggleAction* showToolbar(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoToggleAction* showStatusbar(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoToggleAction* fullScreen(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* keyBindings(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
	static BoUfoAction* preferences(const QObject* receiver = 0, const char* slot = 0,
			BoUfoActionCollection* parent = 0, const char* name = 0);
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
 * can create a libufo submenu structure (see @ref createMenuBarUfoSubMenu), but this
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

	void createUfoMenuBarSubMenu(ufo::UWidget*);
	void createUfoToolBarSubMenu(ufo::UWidget*);

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

	void setVisible(bool v);

	void createMenuBar();

	static void initMenuBar(BoUfoManager* m);

	ufo::UMenuBar* ufoMenuBar() const;

protected:
	void clearUfoMenuBar();

private:
	BoUfoMenuBarPrivate* d;
};

class BoUfoToolBarPrivate;
/**
 * You should never need this class, it is used internally.
 **/
class BoUfoToolBar : public BoUfoMenuBarMenu
{
	Q_OBJECT
public:
	BoUfoToolBar(BoUfoManager* parent, const char* name = 0);
	~BoUfoToolBar();

	void setVisible(bool v);

	void createToolBar();

	static void initToolBar(BoUfoManager* m);

//	ufo::UMenuBar* ufoToolBar() const;

protected:
	void clearUfoToolBar();

private:
	BoUfoToolBarPrivate* d;
};


#endif

