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

#include <ufo/widgets/umenubar.hpp>
#include <ufo/widgets/umenu.hpp>
#include <ufo/widgets/urootpane.hpp>
#include <ufo/signals/uobjectslot.hpp>
#include <ufo/signals/usignal.hpp>
#include <ufo/ufo.hpp>

#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufoaction.h"
#include "boufoaction.moc"

#include "boufo.h"
#include <bodebug.h>
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
class BoUfoXMLBuilder
{
public:
	BoUfoXMLBuilder(BoUfoActionCollection* c)
	{
		mActionCollection = c;
	}
	~BoUfoXMLBuilder()
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
	bool makeGUI(BoUfoMenuBar* bar)
	{
		QDomElement root = mDoc.documentElement();
		if (root.isNull()) {
			return false;
		}

		bool ret = makeUfoMenuBar(bar);
		if (!ret) {
			boError() << k_funcinfo << "unable to create Ufo MenuBar" << endl;
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

	bool makeUfoMenuBar(BoUfoMenuBar* bar);
	bool makeUfoMenu(const QDomElement& menuBar, BoUfoMenuBarMenu* ufoMenu);

private:
	QDomDocument mDoc;
	BoUfoActionCollection* mActionCollection;
};

bool BoUfoXMLBuilder::mergeMenu(QDomElement& baseMenu, QDomElement& addMenu)
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

bool BoUfoXMLBuilder::cleanElements(QDomElement& element)
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


bool BoUfoXMLBuilder::makeUfoMenuBar(BoUfoMenuBar* bar)
{
 QDomElement root = mDoc.documentElement();
 QDomElement menuBar = root.namedItem("MenuBar").toElement();
 if (menuBar.isNull()) {
	boError() << k_funcinfo << "no MenuBar" << endl;
	return false;
 }

 bool ret = makeUfoMenu(menuBar, bar);
 if (!ret) {
	boError() << k_funcinfo << "cold not make menu" << endl;
	return false;
 }

 // creation of data structures completed.
 // create actual ufo menubar
 bar->createMenu();

 return true;
}

bool BoUfoXMLBuilder::makeUfoMenu(const QDomElement& parentElement, BoUfoMenuBarMenu* ufoMenu)
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
		BoUfoAction* a = mActionCollection->action(name);
		if (!a) {
			boError() << k_funcinfo << "did not find action " << name << endl;
		} else {
			ufoMenu->addMenuItem(a->text(), a, SLOT(slotActivated()));
		}
	} else if (element.tagName() == QString("Separator")) {
		ufoMenu->addSeparator();
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

		BoUfoMenuBarMenu* ufoSubMenu = new BoUfoMenuBarMenu(menuName, ufoMenu);
		bool ret = makeUfoMenu(element, ufoSubMenu);
		if (!ret) {
			boError() << k_funcinfo << "creation of submenu failed" << endl;
			delete ufoSubMenu;
			return false;
		}
		ufoMenu->addSubMenu(ufoSubMenu);
	} else {
		boWarning() << k_funcinfo << "unrecognized tag " << element.tagName() << endl;
	}
	element = element.nextSibling().toElement();
 }
 return true;
}


class BoUfoActionPrivate
{
public:
	BoUfoActionPrivate()
	{
	}
	KShortcut mDefaultShortcut;
	QString mText;
	KShortcut mShortcut;
};

BoUfoAction::BoUfoAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name)
	: QObject(parent, name)
{
 init(parent, text, cut, receiver, slot);
}

BoUfoAction::~BoUfoAction()
{
 delete d;
}

void BoUfoAction::init(BoUfoActionCollection* parent, const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot)
{
 d = new BoUfoActionPrivate;
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

const QString& BoUfoAction::text() const
{
 return d->mText;
}

void BoUfoAction::slotActivated()
{
 emit signalActivated();
}

BoUfoToggleAction::BoUfoToggleAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const char* name)
	: BoUfoAction(text, cut, receiver, slot, parent, name)
{
 mChecked = false;
}

BoUfoToggleAction::~BoUfoToggleAction()
{
}

void BoUfoToggleAction::slotActivated()
{
 BoUfoAction::slotActivated();

 mChecked = !mChecked;
 emit signalToggled(checked());
}

void BoUfoToggleAction::setChecked(bool c)
{
 mChecked = c;
 emit signalInternalToggle(c);
}

class BoUfoActionCollectionPrivate
{
public:
	BoUfoActionCollectionPrivate()
	{
	}
	QDict<BoUfoAction> mActionDict;
};

BoUfoActionCollection::BoUfoActionCollection(QObject* parent, const char* name)
	: QObject(parent, name)
{
 d = new BoUfoActionCollectionPrivate;
 d->mActionDict.setAutoDelete(true);
}

BoUfoActionCollection::~BoUfoActionCollection()
{
 d->mActionDict.clear();
 delete d;
}

void BoUfoActionCollection::insert(BoUfoAction* action)
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

BoUfoAction* BoUfoActionCollection::action(const QString& name) const
{
 return d->mActionDict[name];
}

bool BoUfoActionCollection::hasAction(const QString& name) const
{
 if (!action(name)) {
	return false;
 }
 return true;
}

bool BoUfoActionCollection::createGUI(const QString& file)
{
 if (!parent()) {
	BO_NULL_ERROR(parent());
	return false;
 }
 if (!parent()->inherits("BoUfoManager")) {
	boError() << k_funcinfo << "parent() is not a BoUfoManager" << endl;
	return false;
 }
 BoUfoManager* m = (BoUfoManager*)parent();
 BoUfoMenuBar::initMenuBar(m);
 if (!m->menuBar()) {
	BO_NULL_ERROR(m->menuBar());
	return false;
 }
 BoUfoXMLBuilder builder(this);
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
 return builder.makeGUI(m->menuBar());
}

void BoUfoActionCollection::initActionCollection(BoUfoManager* m)
{
 BO_CHECK_NULL_RET(m);
 if (m->actionCollection()) {
	return;
 }
 BoUfoActionCollection* c = new BoUfoActionCollection(m, "actioncollection");
 m->setActionCollection(c);
}


class BoUfoMenuBarPrivate
{
public:
	BoUfoMenuBarPrivate()
	{
		mUfoManager = 0;
		mActionCollection = 0;

		mBar = 0;
	}

	BoUfoManager* mUfoManager;
	BoUfoActionCollection* mActionCollection;

	ufo::UMenuBar* mBar;
};

BoUfoMenuBar::BoUfoMenuBar(BoUfoManager* parent, const char* name)
	: BoUfoMenuBarMenu(QString::null, parent, name)
{
 d = new BoUfoMenuBarPrivate;
 d->mUfoManager = parent;
 d->mActionCollection = parent->actionCollection();
}

BoUfoMenuBar::~BoUfoMenuBar()
{
 clearUfo();
 delete d;
}

ufo::UMenuBar* BoUfoMenuBar::ufoMenuBar() const
{
 return d->mBar;
}

void BoUfoMenuBar::clearUfo()
{
 if (d->mBar && d->mUfoManager) {
	d->mUfoManager->setMenuBar(0); // deletes the ufo menubar
	d->mBar = 0;
 }
}

void BoUfoMenuBar::createMenu()
{
 BO_CHECK_NULL_RET(d->mUfoManager);
 BO_CHECK_NULL_RET(d->mUfoManager->rootPane());
 d->mUfoManager->makeContextCurrent();
 clearUfo();
 d->mBar = new ufo::UMenuBar();
 d->mUfoManager->rootPane()->setMenuBar(d->mBar);

 boDebug() << k_funcinfo << "creating submenus" << endl;
 createUfoSubMenu(d->mBar);
}

void BoUfoMenuBar::initMenuBar(BoUfoManager* m)
{
 BO_CHECK_NULL_RET(m);
 BO_CHECK_NULL_RET(m->actionCollection());
 if (m->menuBar()) {
	return;
 }
 BoUfoMenuBar* bar = new BoUfoMenuBar(m, "menubar");
 m->setMenuBar(bar);
}



class BoUfoMenuBarMenuPrivate
{
public:
	BoUfoMenuBarMenuPrivate()
	{
	}
	QValueList<BoUfoMenuBarItem*> mItems;
};

BoUfoMenuBarMenu::BoUfoMenuBarMenu(const QString& text, QObject* parent, const char* name)
	: BoUfoMenuBarItem(0, text, parent, name)
{
 d = new BoUfoMenuBarMenuPrivate;
}

BoUfoMenuBarMenu::~BoUfoMenuBarMenu()
{
 QValueList<BoUfoMenuBarItem*>::Iterator it;
 for (it = d->mItems.begin(); it != d->mItems.end(); ++it) {
	delete *it;
 }
 d->mItems.clear();
 delete d;
}

void BoUfoMenuBarMenu::addSeparator()
{
 addMenuItem(i18n("--------"), 0, 0);
}

void BoUfoMenuBarMenu::addMenuItem(const QString& text, BoUfoAction* a, const char* slot)
{
 BoUfoMenuBarItem* item = new BoUfoMenuBarItem(a, text, this);
 if (a && slot) {
	connect(item, SIGNAL(signalActivated()), a, slot);
 }
 d->mItems.append(item);
}

void BoUfoMenuBarMenu::addSubMenu(BoUfoMenuBarMenu* menu)
{
 d->mItems.append(menu);
}

unsigned int BoUfoMenuBarMenu::itemCount() const
{
 return d->mItems.count();
}

QValueList<BoUfoMenuBarItem*>& BoUfoMenuBarMenu::items() const
{
 return d->mItems;
}

void BoUfoMenuBarMenu::createUfoSubMenu(ufo::UWidget* parentWidget)
{
 BO_CHECK_NULL_RET(parentWidget);

 for (unsigned int i = 0; i < itemCount(); i++) {
	if (items()[i]->isA("BoUfoMenuBarMenu")) {
		BoUfoMenuBarMenu* m = (BoUfoMenuBarMenu*)items()[i];

		// TODO: we could provide an icon
		ufo::UMenu* menu = new ufo::UMenu(m->text().latin1());
		menu->sigActivated().connect(slot(*((BoUfoMenuBarItem*)m), &BoUfoMenuBarMenu::uslotActivated));
		menu->sigHighlighted().connect(slot(*((BoUfoMenuBarItem*)m), &BoUfoMenuBarItem::uslotHighlighted));

		parentWidget->add(menu);

		m->createUfoSubMenu(menu);
	} else {
		BoUfoMenuBarItem* item = items()[i];
		BoUfoAction* action = item->action();
		ufo::UMenuItem* menuItem = 0;
		if (!action) {
			// e.g. a separator
			menuItem = new ufo::UMenuItem(item->text().latin1());
		} else if (action->inherits("BoUfoToggleAction")) {
			BoUfoToggleAction* toggle = (BoUfoToggleAction*)action;
#if UFO_MAJOR_VERSION == 0 && UFO_MINOR_VERSION <= 7 && UFO_MICRO_VERSION <= 2
			// AB: libufo <= 0.7.2 had a broken UCheckBoxMenuItem
			// implementation.
			ufo::UMenuItem* c = new ufo::UMenuItem(item->text().latin1());
#else
			ufo::UCheckBoxMenuItem* c = new ufo::UCheckBoxMenuItem(item->text());
			c->setSelected(toggle->checked());
#endif
			menuItem = c;
		} else {
			menuItem = new ufo::UMenuItem(item->text().latin1());
		}
		item->setUfoItem(menuItem);
		menuItem->sigActivated().connect(slot(*item, &BoUfoMenuBarItem::uslotActivated));
		menuItem->sigHighlighted().connect(slot(*item, &BoUfoMenuBarItem::uslotHighlighted));

		parentWidget->add(menuItem);
	}
 }
}


BoUfoMenuBarItem::BoUfoMenuBarItem(BoUfoAction* action, const QString& text, QObject* parent, const char* name)
	: QObject(parent, name)
{
 mAction = action;
 mText = text;
 mUfoItem = 0;

 if (mAction && mAction->inherits("BoUfoToggleAction")) {
	connect(mAction, SIGNAL(signalInternalToggle(bool)),
			this, SLOT(slotInternalToggle(bool)));
 }
}

BoUfoMenuBarItem::~BoUfoMenuBarItem()
{
}

void BoUfoMenuBarItem::slotInternalToggle(bool checked)
{
 BO_CHECK_NULL_RET(ufoItem());
 ufo::UCheckBoxMenuItem* c = dynamic_cast<ufo::UCheckBoxMenuItem*>(ufoItem());
 BO_CHECK_NULL_RET(c);
 c->setSelected(checked);
}

void BoUfoMenuBarItem::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
}

void BoUfoMenuBarItem::uslotHighlighted(ufo::UActionEvent*)
{
}

void BoUfoMenuBarItem::uslotWidgetRemoved(ufo::UWidget*)
{
 mUfoItem = 0;
}

void BoUfoMenuBarItem::setUfoItem(ufo::UMenuItem* i)
{
 mUfoItem = i;
 // AB: this is required to avoid invalid pointers. I'd like to
 // use a sigWidgetDeleted() signal instead.
 i->sigWidgetRemoved().connect(slot(*this, &BoUfoMenuBarItem::uslotWidgetRemoved));

}

