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

#include "bopuiaction.h"
#include "bopuiaction.moc"

#include <bodebug.h>
#include "bopumenubar.h"
#include "../bosonglwidget.h"

#include <kshortcut.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <qdict.h>
#include <qdom.h>
#include <qfile.h>
#include <qvaluelist.h>

#include <stdlib.h>
#include <string.h>

#define PU_USE_NONE
#include <plib/pu.h>

// AB: in contrast to KDE, we allow only (exactly) one MenuBar. No more, no
// less.
// AB: we try to use the kpartgui.dtd as far as possible, have a look at it for
//     possible tags. but note that we don't support a lot of them
//
// The root tag of a *ui.rc is always a <kpartgui> tag.
// <MenuBar>,<ToolBar> and <Menu> tags are container tags, they can contain
//   further tags.
// Relevant to us are mainly <Action> tags, a container tag withou <Action> tags
// is useless.
//
// Some of the unsupported tags: <TearOffHandle>, <ActionList>
class BoPUIXMLBuilder
{
public:
	BoPUIXMLBuilder(BoPUIActionCollection* c)
	{
		mActionCollection = c;
	}
	~BoPUIXMLBuilder()
	{
	}
	bool reset()
	{
		QFile file(locate("config", "ui/ui_standards.rc"));
		if (!file.open(IO_ReadOnly)) {
			boError() << k_funcinfo << "could not open ui_standards.rc" << endl;
			return false;
		}
		return mDoc.setContent(&file);
	}
	bool mergeFile(const QString& fileName)
	{
		QFile file(fileName);
		if (!file.open(IO_ReadOnly)) {
			boError() << k_funcinfo << "could not open " << fileName << endl;
			return false;
		}
		QDomDocument doc;
		if (!doc.setContent(&file)) {
			boError() << k_funcinfo << "parse error in " << fileName << endl;
			return false;
		}
		QDomElement baseRoot = mDoc.documentElement();
		QDomElement addRoot = doc.documentElement();
		if (!mergeMenuBars(baseRoot, addRoot)) {
			boError() << k_funcinfo << "menubar merging failed" << endl;
			return false;
		}
		// TODO: ToolBar tags

		return true;
	}

	bool cleanDoc()
	{
		QDomElement root = mDoc.documentElement();
		if (root.isNull()) {
			return false;
		}

		// clean up things we don't need
		if (!removeMergeLocal(root)) {
			return false;
		}

		if (!cleanElements(root)) {
			return false;
		}
		return true;
	}
	bool makeGUI(BoPUIMenuBar* bar)
	{
		QDomElement root = mDoc.documentElement();
		if (root.isNull()) {
			return false;
		}

		bool ret = makePUIMenuBar(bar);
		if (!ret) {
			boError() << k_funcinfo << "unable to create PUI MenuBar" << endl;
		}
		return ret;
	}

protected:
	bool mergeMenuBars(QDomElement& baseRoot, QDomElement& addRoot)
	{
		if (baseRoot.elementsByTagName("MenuBar").count() > 1) {
			boError() << k_funcinfo << "more than 1 MenuBar tag found in the base XML doc. not supported." << endl;
			return false;
		}
		if (addRoot.elementsByTagName("MenuBar").count() > 1) {
			boError() << k_funcinfo << "more than 1 MenuBar tag found in the new XML doc. not supported." << endl;
			return false;
		}
		QDomElement baseMenuBar = baseRoot.namedItem("MenuBar").toElement();
		QDomElement addMenuBar = addRoot.namedItem("MenuBar").toElement();
		if (baseMenuBar.isNull()) {
			boError() << k_funcinfo << "no MenuBar in base" << endl;
			return false;
		}

		QMap<QString, QDomElement> baseMenus;
		QMap<QString, QDomElement> addMenus;
		getChildTags(QString("Menu"), baseMenuBar, addMenuBar, baseMenus, addMenus);

#warning TODO actions outside of a Menu
		// TODO: Action tags that don't belong to a <Menu>
		QMap<QString, QDomElement>::Iterator it;
		for (it = addMenus.begin(); it != addMenus.end(); ++it) {
			QString name = it.key();
			QDomElement addMenu = it.data();
			if (baseMenus.contains(name)) {
				boDebug() << "merging menus " << name << endl;
				QDomElement baseMenu = baseMenus[name];
				if (!mergeMenu(baseMenu, addMenu)) {
					return false;
				}
			} else {
				boDebug() << "adding " << it.key() << endl;
				boWarning() << k_funcinfo << "custom menus ignore MergeLocal tags" << endl;
#warning FIXME
				// FIXME: append the child before a MergeLocal
				// tag!!
				baseMenuBar.appendChild(addMenu);
			}
		}
		return true;
	}

	void getChildTags(const QString& tagName, QDomElement& baseRoot, QDomElement& addRoot, QMap<QString, QDomElement>& baseMenus, QMap<QString, QDomElement>& addMenus)
	{
		QDomNode node = baseRoot.firstChild();
		while (!node.isNull()) {
			QDomElement e = node.toElement();
			node = node.nextSibling();
			if (e.isNull()) {// not an element
				continue;
			}
			if (e.tagName() != tagName) {
				continue;
			}
			QString name = e.attribute("name");
			baseMenus.insert(name, e);
		}

		node = addRoot.firstChild();
		while (!node.isNull()) {
			QDomElement e = node.toElement();
			node = node.nextSibling();
			if (e.isNull()) {// not an element
				continue;
			}
			if (e.tagName() != tagName) {
				continue;
			}
			QString name = e.attribute("name");
			addMenus.insert(name, e);
		}
	}

	// can be used for ToolBar tags, too
	bool mergeMenu(QDomElement& baseMenu, QDomElement& addMenu);

	// removed <Action> tags that are not implemented in the
	// mActionCollection
	// also removed <ActionList> and <TearOffHandle> tags, as we don't support them
	bool cleanElements(QDomElement& element);

	/**
	 * Remove MergeLocal tags from the element. Also removes DefineGroup
	 * tags (which are similar to MergeLocal) and Merge (which are not used
	 * by us) tags.
	 **/
	bool removeMergeLocal(QDomElement& element)
	{
		if (element.isNull()) {
			return false;
		}
		QDomElement eIt = element.firstChild().toElement();
		while (!eIt.isNull()) {
			QDomElement e = eIt;
			eIt = eIt.nextSibling().toElement();

			QString tag = e.tagName();
			if (tag == QString("Menu") || tag == QString("MenuBar") || tag == QString("ToolBar")) {
				bool ret = removeMergeLocal(e);
				if (!ret) {
					return false;
				}
			} else if (tag == QString("Merge") || tag == QString("MergeLocal") || tag == QString("DefineGroup")) {
				// AB: Merge is totally unused by us
				// AB: Merge is totally unused by us
				element.removeChild(e);
			}
		}
		return true;
	}

	bool makePUIMenuBar(BoPUIMenuBar* bar);
	bool makePUIMenu(const QDomElement& menuBar, BoPUIMenuBarMenu* puiMenu);

private:
	QDomDocument mDoc;
	BoPUIActionCollection* mActionCollection;
};

bool BoPUIXMLBuilder::mergeMenu(QDomElement& baseMenu, QDomElement& addMenu)
{
 if (baseMenu.isNull() || addMenu.isNull()) {
	boError() << k_funcinfo << endl;
	return false;
 }
 QDomElement e = baseMenu.firstChild().toElement();
 while (!e.isNull()) {
	QString tag = e.tagName();
	if (tag == QString("Action")) {
		QString name = e.attribute("name");
		if (!mActionCollection->hasAction(name)) {
			QDomElement old = e;
			e = e.nextSibling().toElement();
			baseMenu.removeChild(old);
			continue;
		}
	} else if (tag == QString("MergeLocal")) {
		QString name = e.attribute("name");
		QDomElement it = addMenu.firstChild().toElement();
		while (!it.isNull()) {
			QDomElement child = it;
			// we change it to the next sibling now,
			// so that we can remove child from this
			// doc
			it = it.nextSibling().toElement();
			if (child.tagName() != QString("Action")) {
				continue;
			}
			if (child.attribute("append").isEmpty() != name.isEmpty()) {
				continue;
			}
			if (!name.isEmpty()) {
				if (child.attribute("append") != name) {
					continue;
				}
			}
			baseMenu.insertBefore(child.cloneNode(), e);
			addMenu.removeChild(child);
		}
	}
	e = e.nextSibling().toElement();
 }
 return true;
}

bool BoPUIXMLBuilder::cleanElements(QDomElement& element)
{
 if (element.isNull()) {
	return false;
 }
 int separator = -1; // no previous element
 QDomElement eIt = element.firstChild().toElement();
 while (!eIt.isNull()) {
	QDomElement e = eIt;
	eIt = eIt.nextSibling().toElement();

	QString tag = e.tagName();
	if (tag == QString("Action")) {
		QString name = e.attribute("name");
		if (!mActionCollection->hasAction(name)) {
			element.removeChild(e);
		}
	} else if (tag == QString("ActionList")) {
		// we don't support <ActionList>
		element.removeChild(e);
	} else if (tag == QString("TearOffHandle")) {
		// we don't support <TearOffHandle>
		element.removeChild(e);
	} else if (tag == QString("Menu")) {
		bool ret = cleanElements(e);
		if (!ret) {
			return false;
		}

		if (e.elementsByTagName("Action").count() == 0) {
			// the <Menu> tag is useless.
			element.removeChild(e);
		}
	} else if (tag == QString("MenuBar") || tag == QString("ToolBar")) {
		bool ret = cleanElements(e);
		if (!ret) {
			return false;
		}
	} else if (tag == QString("Separator")) {
		if (separator == -1 || separator == 1) {
			element.removeChild(e);
		}
	}


	if (tag == QString("Separator")) {
		if (e.parentNode() == element) {
			// previous element is now a separator
			separator = 1;
		}
	} else {
		if (tag != QString("text")) {
			if (e.parentNode() == element) {
				// previous element is not a separator anymore
				separator = 0;
			}
		}
	}
 }
 return true;
}


bool BoPUIXMLBuilder::makePUIMenuBar(BoPUIMenuBar* bar)
{
 QDomElement root = mDoc.documentElement();
 QDomElement menuBar = root.namedItem("MenuBar").toElement();
 if (menuBar.isNull()) {
	boError() << k_funcinfo << "no MenuBar" << endl;
	return false;
 }

 bool ret = makePUIMenu(menuBar, bar);
 if (!ret) {
	boError() << k_funcinfo << "cold not make menu" << endl;
	return false;
 }

 // creation of data structures completed.
 // create actual plib menubar
 bar->createMenu();

 return true;
}

bool BoPUIXMLBuilder::makePUIMenu(const QDomElement& parentElement, BoPUIMenuBarMenu* puiMenu)
{
 // note: plib does not support
 // - menu items that are directly in a menubar (without a
 //   submenu)
 // - menus inside menus
 //   (this is very important to us)

 if (parentElement.isNull()) {
	return false;
 }

 QDomElement element = parentElement.firstChild().toElement();
 while (!element.isNull()) {
	if (element.tagName() == QString("text")) {
		// nothing to do.
	} else if (element.tagName() == QString("Action")) {
		QString name = element.attribute("name");
		BoPUIAction* a = mActionCollection->action(name);
		if (!a) {
			boError() << k_funcinfo << "did not find action " << name << endl;
		} else {
			puiMenu->addMenuItem(a->text(), a, SLOT(slotActivated()));
		}
	} else if (element.tagName() == QString("Separator")) {
		puiMenu->addSeparator();
	} else if (element.tagName() == QString("Menu")) {
		QString menuName;
		QStringList list;
		QDomElement menuText = element.namedItem("text").toElement();
		if (menuText.isNull()) {
			boError() << k_funcinfo << "no text element found for menu" << endl;
			menuName = i18n("Unknown");
		} else {
			menuName = menuText.text();
		}

		BoPUIMenuBarMenu* puiSubMenu = new BoPUIMenuBarMenu(menuName, puiMenu);
		bool ret = makePUIMenu(element, puiSubMenu);
		if (!ret) {
			boError() << k_funcinfo << "creation of submenu failed" << endl;
			delete puiSubMenu;
			return false;
		}
		puiMenu->addSubMenu(puiSubMenu);
	} else {
		boWarning() << k_funcinfo << "unrecognized tag " << element.tagName() << endl;
	}
	element = element.nextSibling().toElement();
 }
 return true;
}


class BoPUIActionPrivate
{
public:
	BoPUIActionPrivate()
	{
	}
	KShortcut mDefaultShortcut;
	QString mText;
	KShortcut mShortcut;
};

BoPUIAction::BoPUIAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoPUIActionCollection* parent, const char* name)
	: QObject(parent, name)
{
 init(parent, text, cut, receiver, slot);
}

BoPUIAction::~BoPUIAction()
{
 delete d;
}

void BoPUIAction::init(BoPUIActionCollection* parent, const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot)
{
 d = new BoPUIActionPrivate;
 mParentCollection = parent;
 d->mDefaultShortcut = cut;
 d->mText = text;

 if (!mParentCollection) {
	BO_NULL_ERROR(mParentCollection);
 } else {
	mParentCollection->insert(this);
 }

 if (receiver && slot) {
	connect(this, SIGNAL(signalActivated()), receiver, slot);
 }

 d->mShortcut = cut;
 // TODO: insert shortcut to actioncollection
 // -> also connect accel
 // --> see KAction::initShortcut()
}

const QString& BoPUIAction::text() const
{
 return d->mText;
}

void BoPUIAction::slotActivated()
{
 emit signalActivated();
}

class BoPUIActionCollectionPrivate
{
public:
	BoPUIActionCollectionPrivate()
	{
	}
	QDict<BoPUIAction> mActionDict;
};

BoPUIActionCollection::BoPUIActionCollection(QObject* parent, const char* name)
	: QObject(parent, name)
{
 d = new BoPUIActionCollectionPrivate;
 d->mActionDict.setAutoDelete(true);
}

BoPUIActionCollection::~BoPUIActionCollection()
{
 d->mActionDict.clear();
 delete d;
}

void BoPUIActionCollection::insert(BoPUIAction* action)
{
 char unnamed[100];
 const char* name = action->name();
 if (qstrcmp(name, "unnamed") == 0) {
	sprintf(unnamed, "unnamed-%p", (void*)action);
	name = unnamed;
 }
 if (d->mActionDict[name]) {
	boError() << k_funcinfo << "already an action with name " << name << " inserted" << endl;
	return;
 }
 d->mActionDict.insert(name, action);
}

BoPUIAction* BoPUIActionCollection::action(const QString& name) const
{
 return d->mActionDict[name];
}

bool BoPUIActionCollection::hasAction(const QString& name) const
{
 if (!action(name)) {
	return false;
 }
 return true;
}

bool BoPUIActionCollection::createGUI(const QString& file)
{
 if (!parent()) {
	BO_NULL_ERROR(parent());
	return false;
 }
 if (!parent()->inherits("BosonGLWidget")) {
	boError() << k_funcinfo << "parent() is not a BosonGLWidget" << endl;
	return false;
 }
 BosonGLWidget* w = (BosonGLWidget*)parent();
 puSetWindow(w->winId());
 BoPUIMenuBar::initMenuBar(w);
 if (!w->menuBar()) {
	BO_NULL_ERROR(w->menuBar());
	return false;
 }
 BoPUIXMLBuilder builder(this);
 if (!builder.reset()) {
	boError() << k_funcinfo << "builder reset failed" << endl;
	return false;
 }
 if (!builder.mergeFile(file)) {
	boError() << k_funcinfo << "merging of file " << file << " failed" << endl;
	return false;
 }
 if (!builder.cleanDoc()) {
	boError() << k_funcinfo << "failed cleaning the xml doc" << endl;
	return false;
 }
 return builder.makeGUI(w->menuBar());
}

void BoPUIActionCollection::initActionCollection(BosonGLWidget* w)
{
 BO_CHECK_NULL_RET(w);
 if (w->actionCollection()) {
	return;
 }
 BoPUIActionCollection* c = new BoPUIActionCollection(w, "actioncollection");
 w->setActionCollection(c);
}


void bo_pui_menu_item_callback(puObject* object)
{
 BO_CHECK_NULL_RET(object);
 BO_CHECK_NULL_RET(object->getUserData());
 BoPUIMenuBarItem* item = (BoPUIMenuBarItem*)object->getUserData();
 item->activate();
}

class BoPUIMenuBarPrivate
{
public:
	BoPUIMenuBarPrivate()
	{
		mActionCollection = 0;

		mBar = 0;
	}

	BoPUIActionCollection* mActionCollection;

	bopuMenuBar* mBar;
};

BoPUIMenuBar::BoPUIMenuBar(BoPUIActionCollection* c, QObject* parent, const char* name)
	: BoPUIMenuBarMenu(QString::null, parent, name)
{
 d = new BoPUIMenuBarPrivate;
 d->mActionCollection = c;
}

BoPUIMenuBar::~BoPUIMenuBar()
{
 clearPUI();
 delete d;
}

bopuMenuBar* BoPUIMenuBar::puiMenuBar() const
{
 return d->mBar;
}

void BoPUIMenuBar::clearPUI()
{
 if (d->mBar) {
	puDeleteObject(d->mBar);
	d->mBar = 0;
 }
}

void BoPUIMenuBar::createMenu()
{
 BO_CHECK_NULL_RET(parent());
 if (!parent()->inherits("BosonGLWidget")) {
	boError() << k_funcinfo << "parent() is not a BosonGLWidget" << endl;
	return;
 }
 BosonGLWidget* w = (BosonGLWidget*)parent();
 puSetWindow(w->winId());
 clearPUI();
 d->mBar = new bopuMenuBar();

 QValueList<BoPUIMenuBarItem*>::Iterator it;
 int i = 0;
 for (it = items().begin(); it != items().end(); ++it, i++) {
	if ((*it)->isA("BoPUIMenuBarMenu")) {
		BoPUIMenuBarMenu* menu = (BoPUIMenuBarMenu*)*it;
		menu->createPUISubMenu();
	} else if ((*it)->isA("BoPUIMenuBarItem")) {
		boWarning() << k_funcinfo << "menu items outside of a submenu are not supported by plib. won't add " << (*it)->text() << endl;
	}
 }
 d->mBar->close();
}

void BoPUIMenuBar::initMenuBar(BosonGLWidget* w)
{
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(w->actionCollection());
 if (w->menuBar()) {
	return;
 }
 BoPUIMenuBar* bar = new BoPUIMenuBar(w->actionCollection(), w, "menubar");
 w->setMenuBar(bar);
}



class BoPUIMenuBarMenuPrivate
{
public:
	BoPUIMenuBarMenuPrivate()
	{
	}
	QValueList<BoPUIMenuBarItem*> mItems;
};

BoPUIMenuBarMenu::BoPUIMenuBarMenu(const QString& text, QObject* parent, const char* name)
	: BoPUIMenuBarItem(text, parent, name)
{
 d = new BoPUIMenuBarMenuPrivate;
}

BoPUIMenuBarMenu::~BoPUIMenuBarMenu()
{
 QValueList<BoPUIMenuBarItem*>::Iterator it;
 for (it = d->mItems.begin(); it != d->mItems.end(); ++it) {
	delete *it;
 }
 d->mItems.clear();
 clearPUI();
 delete d;
}

void BoPUIMenuBarMenu::clearPUI()
{
 // obsolete :-)
}

void BoPUIMenuBarMenu::addSeparator()
{
 addMenuItem(i18n("--------"), 0, 0);
}

void BoPUIMenuBarMenu::addMenuItem(const QString& text, const QObject* receiver, const char* slot)
{
 BoPUIMenuBarItem* item = new BoPUIMenuBarItem(text, this);
 if (receiver && slot) {
	connect(item, SIGNAL(signalActivated()), receiver, slot);
 }
 d->mItems.append(item);
}

void BoPUIMenuBarMenu::addSubMenu(BoPUIMenuBarMenu* menu)
{
 d->mItems.append(menu);
}

unsigned int BoPUIMenuBarMenu::itemCount() const
{
 return d->mItems.count();
}

QValueList<BoPUIMenuBarItem*>& BoPUIMenuBarMenu::items() const
{
 return d->mItems;
}

void BoPUIMenuBarMenu::createPUISubMenu()
{
 BO_CHECK_NULL_RET(parent());
 if (!parent()->inherits("BoPUIMenuBarMenu")) {
	boError() << k_funcinfo << "parent() is not a BoPUIMenuBarMenu" << endl;
	return;
 }
 puPopupMenu* currentPopupMenu = 0;
 BoPUIMenuBar* bar = 0;
 if (parent()->inherits("BoPUIMenuBar")) {
	bar = (BoPUIMenuBar*)parent();
 } else if (parent()->inherits("BoPUIMenuBarMenu")) {
	QObject* p = parent()->parent();
	while (!bar && p) {
		if (p->inherits("BoPUIMenuBar")) {
			bar = (BoPUIMenuBar*)p;
		} else if (p->inherits("BoPUIMenuBarMenu")) {
			p = p->parent();
		} else {
			boError() << k_funcinfo << "unexpected parent class" << endl;
			return;
		}
	}
	if (!bar) {
		boError() << k_funcinfo << "could not find the BoPUIMenuBar parent" << endl;
		return;
	}
	puGroup* g = puGetCurrGroup();
	BO_CHECK_NULL_RET(g);
	if (g->getType() & PUCLASS_POPUPMENU) {
		currentPopupMenu = (puPopupMenu*)g;
	}
 } else {
	boError() << k_funcinfo << "unexpected parent() class" << endl;
	return;
 }

 BO_CHECK_NULL_RET(bar->puiMenuBar());
 clearPUI();

 puPopupMenu* p = bar->puiMenuBar()->addSubMenu(currentPopupMenu, puiText());
 for (unsigned int i = 0; i < itemCount(); i++) {
	int index = itemCount() - 1 - i;
	if (items()[index]->isA("BoPUIMenuBarMenu")) {
		BoPUIMenuBarMenu* m = (BoPUIMenuBarMenu*)items()[index];
		m->createPUISubMenu();
	} else {
		bar->puiMenuBar()->addMenuItem(p,
				items()[index]->puiText(),
				bo_pui_menu_item_callback,
				(void*)items()[index]);
	}
 }
 bar->puiMenuBar()->closeSubMenu(p);
}


BoPUIMenuBarItem::BoPUIMenuBarItem(const QString& _text, QObject* parent, const char* name)
	: QObject(parent, name)
{
 mText = _text;
 QString text = _text;
 if (text.isNull()) {
	text = "";
 }
 text.remove("&"); // TODO: does plib support accels?
 mPUIText = new char[text.length() + 1];
 strncpy(mPUIText, text.latin1(), text.length() + 1);
}

BoPUIMenuBarItem::~BoPUIMenuBarItem()
{
 delete[] mPUIText;
}

void BoPUIMenuBarItem::activate()
{
 emit signalActivated();
}

