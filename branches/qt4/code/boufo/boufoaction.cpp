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

#include <kshortcut.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kglobal.h>
#include <kaboutdata.h>

#include <q3dict.h>
#include <q3intdict.h>
#include <qdom.h>
#include <qfile.h>
#include <q3valuelist.h>
#include <qpointer.h>
//Added by qt3to4:
#include <Q3PtrList>

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
		QFile file(KStandardDirs::locate("config", "ui/ui_standards.rc"));
		if (!file.open(QIODevice::ReadOnly)) {
			boError() << k_funcinfo << "could not open ui_standards.rc" << endl;
			return false;
		}
		return mDoc.setContent(&file);
	}
	bool mergeFile(const QString& fileName)
	{
		if (fileName.isEmpty()) {
			boError() << k_funcinfo << "empty filename specified";
			return false;
		}
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly)) {
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
	bool makeUfoMenuBar(BoUfoMenuBar* bar, const QString& name);
	bool makeUfoToolBar(BoUfoToolBar* bar, const QString& name);

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

		if (!mergeBars(baseMenuBar, addMenuBar)) {
			boError() << k_funcinfo << "failed merging MenuBars" << endl;
			return false;
		}
		for (QDomNode n = addRoot.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if (e.isNull()) {
				continue;
			}
			if (e.tagName() != "ToolBar") {
				continue;
			}
			QDomElement addToolBar = e;
			QDomElement baseToolBar;
			QString name = e.attribute("name");
			for (QDomNode n2 = baseRoot.firstChild(); !n2.isNull() && baseToolBar.isNull(); n2 = n2.nextSibling()) {
				QDomElement e2 = n2.toElement();
				if (e2.isNull()) {
					continue;
				}
				if (e2.tagName() != "ToolBar") {
					continue;
				}
				if (e2.attribute("name") == name) {
					baseToolBar = e2;
				}
			}
			if (!baseToolBar.isNull()) {
				if (!mergeBars(baseToolBar, addToolBar)) {
					boError() << k_funcinfo << "failed merging ToolBars with name " << name << endl;
					return false;
				}
			} else {
				baseMenuBar.parentNode().appendChild(addToolBar);
			}
		}
		return true;
	}

	bool mergeBars(QDomElement& baseMenuBar, QDomElement& addMenuBar)
	{
		QMap<QString, QDomElement> baseTopItems;
		QMap<QString, QDomElement> addTopItems;
		QStringList tags;
		tags.append("Menu");
		tags.append("Action");
		getChildTags(tags, baseMenuBar, addMenuBar, baseTopItems, addTopItems);

		QMap<QString, QDomElement>::Iterator it;

		// merge all items that exist in both, addTopItems and
		// baseTopItems
		QStringList processedTopItems;
		for (it = addTopItems.begin(); it != addTopItems.end(); ++it) {
			QString name = it.key();
			QDomElement addItem = it.value();
			if (baseTopItems.contains(name)) {
				QDomElement baseItem = baseTopItems[name];
				if (baseItem.tagName() != "Menu") {
//					boWarning() << k_funcinfo << name << " is not a menu. ignoring." << endl;
					continue;
				}
				if (baseItem.tagName() != addItem.tagName()) {
					boError() << k_funcinfo << name << " has different tagnames" << endl;
					continue;
				}
				if (!mergeMenu(baseItem, addItem)) {
					return false;
				}
				processedTopItems.append(name);
			}
		}
		// remove all processed items from addTopItems
		for (QStringList::iterator it = processedTopItems.begin(); it != processedTopItems.end(); ++it) {
			addTopItems.remove(*it);
		}
		processedTopItems.clear();

		// merge all items that exist in addTopItems only
		// -> place them right before the corresponding MergeLocal tag
		// but first create a element that contains all addTopItems.
		QDomElement addItemsElement = addMenuBar.ownerDocument().createElement("Dummy");
		for (it = addTopItems.begin(); it != addTopItems.end(); ++it) {
			addItemsElement.appendChild(it.value());
		}
#if 0
		for (QDomNode n = baseMenuBar.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if (e.isNull()) {
				continue;
			}
			if (e.tagName() == "MergeLocal") {
				QString mergeName = e.attribute("name");
				mergeMenuItems(baseMenuBar, e, addItemsElement, mergeName);
			}
		}
#else
		mergeMenu(baseMenuBar, addItemsElement);
#endif
		return true;
	}

	void mergeMenuItems(QDomElement& baseMenu, QDomElement& mergeTag, QDomElement& addMenu, const QString& mergeName)
	{
		QDomElement it = addMenu.firstChild().toElement();
		while (!it.isNull()) {
			QDomElement child = it;
			// we change it to the next sibling now,
			// so that we can remove child from this
			// doc
			it = it.nextSibling().toElement();

			if (child.tagName() != QString("Action") && child.tagName() != QString("Menu")) {
				continue;
			}
			if (child.attribute("append").isEmpty() != mergeName.isEmpty()) {
				continue;
			}
			if (!mergeName.isEmpty()) {
				if (child.attribute("append") != mergeName) {
					continue;
				}
			}
			baseMenu.insertBefore(child.cloneNode(), mergeTag);
			addMenu.removeChild(child);
		}
	}

	void getChildTags(const QStringList& tagNames, QDomElement& baseRoot, QDomElement& addRoot, QMap<QString, QDomElement>& baseMenus, QMap<QString, QDomElement>& addMenus)
	{
		QDomNode node = baseRoot.firstChild();
		while (!node.isNull()) {
			QDomElement e = node.toElement();
			node = node.nextSibling();
			if (e.isNull()) {// not an element
				continue;
			}
			if (!tagNames.contains(e.tagName())) {
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
			if (!tagNames.contains(e.tagName())) {
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
				element.removeChild(e);
			}
		}
		return true;
	}

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
		QString mergeName = e.attribute("name");
		mergeMenuItems(baseMenu, e, addMenu, mergeName);
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


bool BoUfoXMLBuilder::makeUfoMenuBar(BoUfoMenuBar* bar, const QString& name)
{
 if (!bar) {
	BO_NULL_ERROR(bar);
	return false;
 }
 Q_UNUSED(name); // atm we support one menubar only
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
 bar->createMenuBar();

 return true;
}

bool BoUfoXMLBuilder::makeUfoToolBar(BoUfoToolBar* bar, const QString& name)
{
 if (!bar) {
	BO_NULL_ERROR(bar);
	return false;
 }
 Q_UNUSED(name); // atm we support one toolbar only
 QDomElement root = mDoc.documentElement();

 // TODO: honor the "name" attribute of Toolbar tags, atm we support only one
 // ToolBar
 QDomElement toolBar = root.namedItem("ToolBar").toElement();
 if (toolBar.isNull()) {
	// null toolbar is 100% valid
	return true;
 }

 bool ret = makeUfoMenu(toolBar, bar);
 if (!ret) {
	boError() << k_funcinfo << "cold not make ToolBar" << endl;
	return false;
 }

 // creation of data structures completed.
 // create actual toolbar
 bar->createToolBar();

 return true;
}

bool BoUfoXMLBuilder::makeUfoMenu(const QDomElement& parentElement, BoUfoMenuBarMenu* ufoMenu)
{
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


class BoUfoActionDeleter : public ufo::UBoUfoWidgetDeleter
{
public:
	BoUfoActionDeleter(BoUfoAction* action, ufo::UWidget* widget)
	{
		mAction = action;
		mWidget = widget;
	}
	~BoUfoActionDeleter()
	{
		doDestructionAction(mAction, mWidget);
	}

	static void doDestructionAction(BoUfoAction* action, ufo::UWidget* widget)
	{
		action->removeWidget(widget, false);
	}

private:
	BoUfoAction* mAction;
	ufo::UWidget* mWidget;
};

class BoUfoToolBarPushButton : public BoUfoPushButton
{
public:
	BoUfoToolBarPushButton(const QString& text)
		: BoUfoPushButton(text)
	{
		mAction = 0;
	}

	void setBoUfoAction(BoUfoAction* action)
	{
		mAction = action;
	}

	~BoUfoToolBarPushButton()
	{
		BO_CHECK_NULL_RET(mAction);
		// AB this does NOT delete the widget.
		BoUfoActionDeleter::doDestructionAction(mAction, ufoWidget());
	}

private:
	BoUfoAction* mAction;
};

class BoUfoToolBarCheckBox : public BoUfoCheckBox
{
public:
	BoUfoToolBarCheckBox(const QString& text)
		: BoUfoCheckBox(text)
	{
		mAction = 0;
	}

	void setBoUfoAction(BoUfoAction* action)
	{
		mAction = action;
	}

	~BoUfoToolBarCheckBox()
	{
		BO_CHECK_NULL_RET(mAction);
		// AB this does NOT delete the widget.
		BoUfoActionDeleter::doDestructionAction(mAction, ufoWidget());
	}

private:
	BoUfoAction* mAction;
};

class BoUfoActionPrivate
{
public:
	BoUfoActionPrivate()
	{
#if 0
		mAccel = 0;
#endif
	}
	KShortcut mDefaultShortcut;
	QString mText;
	KShortcut mShortcut;

	Q3PtrList<ufo::UWidget> mWidgets;

#warning TODO: port to Qt4
#if 0
	KAccel* mAccel;
#endif
};

BoUfoAction::BoUfoAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const QString& name)
	: QObject(parent)
{
 setObjectName(name);
 init(parent, text, cut, receiver, slot);
}

BoUfoAction::~BoUfoAction()
{
 unplug();

 if (mParentCollection) {
	mParentCollection->remove(this, false);
 }
#if 0
 if (d->mAccel) {
	d->mAccel->remove(name());
 }
#endif
 delete d;
}

void BoUfoAction::init(BoUfoActionCollection* parent, const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot)
{
 d = new BoUfoActionPrivate;
 mIsEnabled = true;
 mParentCollection = parent;
 d->mDefaultShortcut = cut;
 d->mText = text;

 if (mParentCollection) {
	mParentCollection->insert(this);
 }

 if (receiver && slot) {
	connect(this, SIGNAL(signalActivated()), receiver, slot);
 }

 d->mShortcut = cut;
 initShortcut();
}

void BoUfoAction::initShortcut()
{
#warning TODO: port to Qt4
#if 0
 if (objectName() == "unnamed") {
	return;
 }
 if (!mParentCollection) {
	boDebug() << k_funcinfo << "won't initialize shortcut for action without parent collection" << endl;
	return;
 }
 if (!mParentCollection->kaccel()) {
	boDebug() << k_funcinfo << "parent collection has no accel object. delay shortcut initialization until setAccelWidget() was called." << endl;
	return;
 }
 insertToKAccel(mParentCollection->kaccel());
#endif
}

void BoUfoAction::insertToKAccel(KAccel* accel)
{
#warning TODO: port to Qt4
#if 0
 if (d->mAccel) {
	boError() << k_funcinfo << "d->mAccel not NULL" << endl;
	return;
 }
 d->mAccel = accel;
 d->mAccel->insert(name(), d->mText, QString::null, d->mShortcut,
		this, SLOT(slotActivated()),
		true, isEnabled());
#endif
}

void BoUfoAction::setEnabled(bool e)
{
 if (mIsEnabled == e) {
	return;
 }
 mIsEnabled = e;
#warning TODO: port to Qt4
#if 0
 if (d->mAccel) {
	d->mAccel->setEnabled(name(), e);
 }
#endif
 Q3PtrListIterator<ufo::UWidget> it(d->mWidgets);
 while (it.current()) {
	it.current()->setEnabled(e);
	++it;
 }

 emit signalEnabled(e);
}

void BoUfoAction::setText(const QString& text)
{
 if (d->mText == text) {
	return;
 }
 d->mText = text;
 Q3PtrListIterator<ufo::UWidget> it(d->mWidgets);
 while (it.current()) {
	ufo::UButton* b = dynamic_cast<ufo::UButton*>(it.current());
	if (b) {
		// a UMenuItem is a UButton, therefore this covers
		// * UMenuItem
		// * UMenu (is a UMenuItem)
		// * UCheckBoxMenuItem (is a UMenuItem)
		QByteArray tmp = d->mText.toAscii();
		b->setText(std::string(tmp.constData(), tmp.length()));
	} else {
		boWarning() << k_funcinfo << "unknown class of widget not handled yet" << endl;
	}
	++it;
 }
}

const QString& BoUfoAction::text() const
{
 return d->mText;
}

void BoUfoAction::slotActivated()
{
 emit signalActivated();
}

void BoUfoAction::slotHighlighted()
{
}

void BoUfoAction::addWidget(ufo::UWidget* w)
{
 d->mWidgets.removeRef(w);
 d->mWidgets.append(w);
}

void BoUfoAction::removeWidget(ufo::UWidget* w, bool del)
{
 if (!del) {
	d->mWidgets.removeRef(w);
 } else {
	if (w) {
		if (w->getParent()) {
			// deletes w. its deleters d'tor will call removeWidget(w, false)
			w->getParent()->remove(w);
		}
	}
 }
}

const Q3PtrList<ufo::UWidget>& BoUfoAction::widgets() const
{
 return d->mWidgets;
}

void BoUfoAction::uslotActivated(ufo::UActionEvent*)
{
 slotActivated();
}

void BoUfoAction::uslotHighlighted(ufo::UActionEvent*)
{
 slotHighlighted();
}


void BoUfoAction::plug(ufo::UWidget* w)
{
 ufo::UMenuBar* menuBar = dynamic_cast<ufo::UMenuBar*>(w);
 ufo::UMenu* menu = dynamic_cast<ufo::UMenu*>(w);
 if (menuBar || menu) {
	QByteArray tmp = text().toAscii();
	ufo::UMenuItem* menuItem = new ufo::UMenuItem(std::string(tmp.constData(), tmp.length()));
	menuItem->setFont(w->getFont());
	BoUfoActionDeleter* deleter = new BoUfoActionDeleter(this, menuItem);
	menuItem->setBoUfoWidgetDeleter(deleter);

	menuItem->sigActivated().connect(slot(*this, &BoUfoAction::uslotActivated));
	menuItem->sigHighlighted().connect(slot(*this, &BoUfoAction::uslotHighlighted));

	w->add(menuItem);
	addWidget(menuItem);
 } else {
	BoUfoToolBarPushButton* button = new BoUfoToolBarPushButton(text());
	button->ufoWidget()->setFont(w->getFont());
	button->setBoUfoAction(this);
	connect(button, SIGNAL(signalClicked()),
			this, SLOT(slotActivated()));
	connect(button, SIGNAL(signalHighlighted()),
			this, SLOT(slotHighlighted()));

	w->add(button->ufoWidget());
	addWidget(button->ufoWidget());
 }
}

void BoUfoAction::unplug()
{
 Q3PtrList<ufo::UWidget> widgets = d->mWidgets;
 Q3PtrListIterator<ufo::UWidget> it(widgets);
 while (it.current()) {
	removeWidget(it.current(), true);
	++it;
 }
}

BoUfoToggleAction::BoUfoToggleAction(const QString& text, const KShortcut& cut, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const QString& name)
	: BoUfoAction(text, cut, receiver, slot, parent, name)
{
 mChecked = false;
}

BoUfoToggleAction::~BoUfoToggleAction()
{
}

void BoUfoToggleAction::plug(ufo::UWidget* w)
{
 ufo::UMenuBar* menuBar = dynamic_cast<ufo::UMenuBar*>(w);
 ufo::UMenu* menu = dynamic_cast<ufo::UMenu*>(w);
 if (menuBar || menu) {
	QByteArray tmp = text().toAscii();
	ufo::UCheckBoxMenuItem* menuItem = new ufo::UCheckBoxMenuItem(std::string(tmp.constData(), tmp.length()));
	menuItem->setFont(w->getFont());
	BoUfoActionDeleter* deleter = new BoUfoActionDeleter(this, menuItem);
	menuItem->setBoUfoWidgetDeleter(deleter);

	menuItem->setSelected(isChecked());
	menuItem->sigActivated().connect(slot(*((BoUfoAction*)this), &BoUfoAction::uslotActivated));
	menuItem->sigHighlighted().connect(slot(*((BoUfoAction*)this), &BoUfoAction::uslotHighlighted));

	w->add(menuItem);
	addWidget(menuItem);
 } else {
	BoUfoToolBarCheckBox* check = new BoUfoToolBarCheckBox(text());
	check->ufoWidget()->setFont(w->getFont());
	check->setBoUfoAction(this);
	check->setChecked(isChecked());
	connect(check, SIGNAL(signalActivated()),
			this, SLOT(slotActivated()));
	connect(check, SIGNAL(signalHighlighted()),
			this, SLOT(slotHighlighted()));

	w->add(check->ufoWidget());
	addWidget(check->ufoWidget());
 }
}

void BoUfoToggleAction::slotActivated()
{
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;

 setChecked(!mChecked);

 recursive = false;
 BoUfoAction::slotActivated();
 emit signalToggled(isChecked());
}

void BoUfoToggleAction::setChecked(bool c)
{
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;
 mChecked = c;
 Q3PtrList<ufo::UWidget> list = widgets();
 Q3PtrListIterator<ufo::UWidget> it(list);
 while (it.current()) {
	ufo::UCheckBoxMenuItem* checkBoxItem = dynamic_cast<ufo::UCheckBoxMenuItem*>(it.current());
	ufo::UCheckBox* checkBox = dynamic_cast<ufo::UCheckBox*>(it.current());
	if (checkBoxItem) {
		if (checkBoxItem->isSelected() != isChecked()) {
			checkBoxItem->setSelected(isChecked());
		}
	} else if (checkBox) {
		if (checkBox->isSelected() != isChecked()) {
			checkBox->setSelected(isChecked());
		}
	} else {
		boError() << k_funcinfo << "widget is not a checkbox" << endl;
	}
	++it;
 }
 recursive = false;
}



class BoUfoActionMenuPrivate
{
public:
	BoUfoActionMenuPrivate()
	{
	}
	Q3PtrList<BoUfoAction> mActions;
	Q3IntDict<BoUfoAction> mId2Action;
};


// AB: KActionMenu maintains a single internal popup menu that is added to every
// widget the action is plugged to.
// however we cannot do this, as UMenu::add() takes ownership of the widgets.
// so whenever this action is plugged to a menu/toolbar/... we need to create a
// new UMenu.
BoUfoActionMenu::BoUfoActionMenu(const QString& text, BoUfoActionCollection* parent, const QString& name)
	: BoUfoAction(text, KShortcut(), 0, 0, parent, name)
{
 d = new BoUfoActionMenuPrivate;
}

BoUfoActionMenu::~BoUfoActionMenu()
{
 delete d;
}

void BoUfoActionMenu::plug(ufo::UWidget* w)
{
 ufo::UMenuBar* menuBar = dynamic_cast<ufo::UMenuBar*>(w);
 ufo::UMenu* menu = dynamic_cast<ufo::UMenu*>(w);
 if (menuBar || menu) {
	QByteArray tmp = text().toAscii();
	ufo::UMenu* m = new ufo::UMenu(std::string(tmp.constData(), tmp.length()));
	m->setFont(w->getFont());
	BoUfoActionDeleter* deleter = new BoUfoActionDeleter(this, m);
	m->setBoUfoWidgetDeleter(deleter);

	w->add(m);
	addWidget(m);
 } else {
	// toolbar
	// TODO: toolbar
	boDebug() << k_funcinfo << "submenus not yet supported for toolbars" << endl;
 }
 redoMenus();
}

void BoUfoActionMenu::insert(BoUfoAction* a, int id, int index) // TODO: index
{
 if (index < 0) {
	index = d->mActions.count();
 }
 if (id >= 0 && d->mId2Action[id]) {
	boError() << k_funcinfo << "Id " << id << " already in use" << endl;
	id = -1;
 }
 if (id < 0) {
	id = 0;
	while (d->mId2Action[id]) {
		id++;
	}
 }
 connect(a, SIGNAL(signalActivated()), this, SLOT(slotMenuItemActivated()));
 d->mActions.insert(index, a);
 d->mId2Action.insert(id, a);
 redoMenus();
}

void BoUfoActionMenu::insertItem(const QString& name, int id, int index)
{
 BoUfoAction* a = new BoUfoAction(name, KShortcut(), 0, 0, 0, 0);
 insert(a, id, index);
}

void BoUfoActionMenu::remove(BoUfoAction* a)
{
 int id = itemId(a);
 if (id >= 0) {
	d->mId2Action.remove(id);
 }
 d->mActions.removeRef(a);
 redoMenus();
}

BoUfoAction* BoUfoActionMenu::item(int id) const
{
 return d->mId2Action[id];
}

int BoUfoActionMenu::itemId(BoUfoAction* a) const
{
 Q3IntDictIterator<BoUfoAction> it(d->mId2Action);
 while (it.current()) {
	if (it.current() == a) {
		return it.currentKey();
	}
	++it;
 }
 return -1;
}

void BoUfoActionMenu::clear()
{
 Q3PtrList<BoUfoAction> list = d->mActions;
 Q3PtrListIterator<BoUfoAction> it(list);
 while (it.current()) {
	remove(it.current());
	++it;
 }
}

void BoUfoActionMenu::slotMenuItemActivated()
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->inherits("BoUfoAction")) {
	boError() << k_funcinfo << "sender() is not a BoUfoAction" << endl;
	return;
 }
 BoUfoAction* a = (BoUfoAction*)sender();
 Q3PtrListIterator<BoUfoAction> it(actions());
 while (it.current()) {
	if (it.current() == a) {
		int id = itemId(a);
		if (id < 0) {
			boError() << k_funcinfo << "unknown item " << a << endl;
			return;
		}
		setCurrentItem(id);
		emit signalActivated(id);
		return;
	}
	++it;
 }
 boError() << k_funcinfo << "could not find item" << endl;
}


void BoUfoActionMenu::redoMenus()
{
 Q3PtrList<ufo::UWidget> list = widgets();
// boDebug() << k_funcinfo << "widgets=" << list.count() << " actions=" << d->mActions.count() << endl;
 Q3PtrListIterator<ufo::UWidget> it(list);
 while (it.current()) {
	ufo::UMenu* menu = dynamic_cast<ufo::UMenu*>(it.current());
	++it;
	if (menu) {
		// AB: this is dangerous. it is correct only, if all child
		// widgets are actually menu items.
		menu->getPopupMenu()->removeAll();

		Q3PtrListIterator<BoUfoAction> it(d->mActions);
		while (it.current()) {
			it.current()->plug(menu);
			++it;
		}
	}
 }
}

const Q3PtrList<BoUfoAction>& BoUfoActionMenu::actions() const
{
 return d->mActions;
}

class BoUfoSelectActionPrivate
{
public:
	BoUfoSelectActionPrivate()
	{
	}
};

BoUfoSelectAction::BoUfoSelectAction(const QString& text, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const QString& name)
	: BoUfoActionMenu(text, parent, name)
{
 d = new BoUfoSelectActionPrivate;

 if (receiver && slot) {
	connect(this, SIGNAL(signalActivated(int)), receiver, slot);
 }
}

BoUfoSelectAction::~BoUfoSelectAction()
{
 delete d;
}

void BoUfoSelectAction::clear()
{
 Q3PtrList<BoUfoAction> list = actions();
 BoUfoActionMenu::clear();
 Q3PtrListIterator<BoUfoAction> it(list);
 while (it.current()) {
	delete it.current();
 }
}

void BoUfoSelectAction::setItems(const QStringList& list)
{
 clear();
 QStringList::ConstIterator it = list.begin();
 int i = 0;
 for (; it != list.end(); ++it) {
	BoUfoToggleAction* a = new BoUfoToggleAction(*it, KShortcut(), 0, 0, 0, 0);
	insert(a, i, i);
	i++;
 }
}

void BoUfoSelectAction::setCurrentItem(int id)
{
 Q3PtrList<BoUfoAction> list = actions();
 Q3PtrListIterator<BoUfoAction> it(list);
 for (int i = 0; it.current(); ++it, i++) {
	if (!it.current()->inherits("BoUfoToggleAction")) {
		boWarning() << k_funcinfo << "oops - not a BoUfoToggleAction" << endl;
		continue;
	}
	BoUfoToggleAction* t = (BoUfoToggleAction*)it.current();
	if (itemId(it.current()) == id) {
		if (!t->isChecked()) {
			t->setChecked(true);
		}
	} else {
		if (t->isChecked()) {
			t->setChecked(false);
		}
	}
 }
}

int BoUfoSelectAction::currentItem() const
{
 Q3PtrList<BoUfoAction> list = actions();
 Q3PtrListIterator<BoUfoAction> it(list);
 for (int i = 0; it.current(); ++it, i++) {
	if (!it.current()->inherits("BoUfoToggleAction")) {
		boWarning() << k_funcinfo << "oops - not a BoUfoToggleAction" << endl;
		continue;
	}
	BoUfoToggleAction* t = (BoUfoToggleAction*)it.current();
	if (t->isChecked()) {
		return itemId(t);
	}
 }
 return -1;
}


class BoUfoActionCollectionPrivate
{
public:
	BoUfoActionCollectionPrivate()
	{
		mParentCollection = 0;

#if 0
		mAccel = 0;
		mAccelWatchWidget = 0;
#endif
	}
	BoUfoActionCollection* mParentCollection;
	Q3PtrList<BoUfoActionCollection> mChildCollections;

	Q3Dict<BoUfoAction> mActionDict;
	QStringList mGUIFiles; // ui.rc files

#if 0
	KAccel* mAccel;
	QPointer<QWidget> mAccelWatchWidget;
#endif
};

BoUfoActionCollection::BoUfoActionCollection(QObject* parent, const QString& name)
	: QObject(parent)
{
 setObjectName(name);
 init();
}

BoUfoActionCollection::BoUfoActionCollection(BoUfoActionCollection* parentCollection, QObject* parent, const QString& name)
	: QObject(parent)
{
 setObjectName(name);
 init();
 d->mParentCollection = parentCollection;
 if (d->mParentCollection) {
	d->mParentCollection->registerChildCollection(this);
 }
}

BoUfoActionCollection::~BoUfoActionCollection()
{
 Q3PtrList<BoUfoActionCollection> children = d->mChildCollections;
 Q3PtrListIterator<BoUfoActionCollection> it(children);
 while (it.current()) {
	it.current()->clearActions();
	++it;
 }
 if (d->mParentCollection) {
	d->mParentCollection->unregisterChildCollection(this);
 }
 clearActions();
 d->mActionDict.setAutoDelete(true);
 d->mActionDict.clear();
#if 0
 delete d->mAccel;
#endif
 delete d;
}

void BoUfoActionCollection::init()
{
 d = new BoUfoActionCollectionPrivate;
#if 0
 d->mAccel = 0;
#endif
 d->mActionDict.setAutoDelete(true);
 d->mParentCollection = 0;
}

void BoUfoActionCollection::registerChildCollection(BoUfoActionCollection* c)
{
 BO_CHECK_NULL_RET(c);
 d->mChildCollections.append(c);
#if 0
 c->setAccelWidget(d->mAccelWatchWidget);
#endif
}

void BoUfoActionCollection::unregisterChildCollection(BoUfoActionCollection* c)
{
 BO_CHECK_NULL_RET(c);
 d->mChildCollections.removeRef(c);
}

KAccel* BoUfoActionCollection::kaccel() const
{
#if 0
 return d->mAccel;
#else
 return 0;
#endif
}

void BoUfoActionCollection::setAccelWidget(QWidget* widget)
{
#if 0
 if (d->mAccel) {
	boWarning() << "accel object already constructed. deleting." << endl;
	delete d->mAccel;
	d->mAccel = 0;
 }
 d->mAccelWatchWidget = widget;
 d->mAccel = new KAccel(d->mAccelWatchWidget, this, "BoUfoActionCollection-Accel");
 for (Q3DictIterator<BoUfoAction> it(d->mActionDict); it.current(); ++it) {
	it.current()->insertToKAccel(d->mAccel);
 }
 for (Q3PtrListIterator<BoUfoActionCollection> it(d->mChildCollections); it.current(); ++it) {
	it.current()->setAccelWidget(widget);
 }
#endif
}

void BoUfoActionCollection::insert(BoUfoAction* action)
{
 QString name = action->objectName();
 if (name == "unnamed") {
	char unnamed[100];
	sprintf(unnamed, "unnamed-%p", (void*)action);
	name = unnamed;
 }
 if (d->mActionDict[name]) {
	boError() << k_funcinfo << "already an action with name " << name << " inserted" << endl;
	return;
 }
 d->mActionDict.insert(name, action);
 if (d->mParentCollection) {
	d->mParentCollection->insert(action);
 }
}

void BoUfoActionCollection::remove(BoUfoAction* action, bool deleteIt)
{
 if (d->mActionDict.isEmpty()) {
	return;
 }
 QString name = action->objectName();
 if (name == "unnamed") {
	char unnamed[100];
	sprintf(unnamed, "unnamed-%p", (void*)action);
	name = unnamed;
 }
 if (!d->mActionDict[name]) {
	return;
 }
 if (d->mParentCollection) {
	d->mParentCollection->remove(action, false);
 }
 BoUfoAction* a = d->mActionDict.take(name);
 if (deleteIt) {
	delete a;
 }
}

void BoUfoActionCollection::clearActions()
{
 if (parent() && parent()->inherits("BoUfoManager")) {
	BoUfoManager* m = (BoUfoManager*)parent();
	m->setMenuBarData(0);
	m->setToolBarData(0);
 }
 if (d->mParentCollection) {
	Q3DictIterator<BoUfoAction> it(d->mActionDict);
	while (it.current()) {
		d->mParentCollection->remove(it.current(), false);
		++it;
	}
 }
 d->mActionDict.setAutoDelete(true);
 d->mActionDict.clear();
}

BoUfoAction* BoUfoActionCollection::action(const QString& name) const
{
 BoUfoAction* a = d->mActionDict[name];
 if (a) {
	return a;
 }
 Q3PtrListIterator<BoUfoActionCollection> it(d->mChildCollections);
 while (it.current()) {
	BoUfoAction* a = it.current()->action(name);
	if (a) {
		return a;
	}
	++it;
 }
 return 0;
}

bool BoUfoActionCollection::hasAction(const QString& name) const
{
 if (!action(name)) {
	return false;
 }
 return true;
}

void BoUfoActionCollection::setActionEnabled(const QString& name, bool enabled)
{
 BoUfoAction* a = action(name);
 if (!a) {
	boError() << k_funcinfo << "no action " << name << endl;
	return;
 }
 a->setEnabled(enabled);
}

void BoUfoActionCollection::setGUIFiles(const QStringList& fileList)
{
 d->mGUIFiles = fileList;
}

const QStringList& BoUfoActionCollection::guiFiles() const
{
 return d->mGUIFiles;
}

bool BoUfoActionCollection::createGUI(const QString& file)
{
 QStringList list;
 list.append(file);
 return createGUI(list);
}

bool BoUfoActionCollection::createGUI(const QStringList& fileList)
{
 setGUIFiles(fileList);
 return createGUI();
}

bool BoUfoActionCollection::createGUI()
{
 if (d->mParentCollection) {
	return d->mParentCollection->createGUI();
 }
 if (!parent()) {
	BO_NULL_ERROR(parent());
	return false;
 }
 if (!parent()->inherits("BoUfoManager")) {
	boError() << k_funcinfo << "parent() is not a BoUfoManager" << endl;
	return false;
 }
 BoUfoManager* m = (BoUfoManager*)parent();
 m->setMenuBarData(0);
 m->setToolBarData(0);
 BoUfoMenuBar::initMenuBar(m);
 BoUfoToolBar::initToolBar(m);
 if (!m->menuBarData()) {
	BO_NULL_ERROR(m->menuBarData());
	return false;
 }
 if (!m->toolBarData()) {
	BO_NULL_ERROR(m->toolBarData());
	return false;
 }
 BoUfoXMLBuilder builder(this);
 if (!builder.reset()) {
	boError() << k_funcinfo << "builder reset failed" << endl;
	return false;
 }

 QStringList fileList = guiFiles();
 if (fileList.isEmpty()) {
	boError() << k_funcinfo << "no guiFiles() specified.";
	return false;
 }
 for (Q3PtrListIterator<BoUfoActionCollection> it(d->mChildCollections); it.current(); ++it) {
	QStringList l = it.current()->guiFiles();
	QStringList::iterator i;
	for (i = l.begin(); i != l.end(); ++i) {
		if (!fileList.contains(*i)) {
			fileList.append(*i);
		}
	}
 }
 QStringList::const_iterator it = fileList.begin();
 for (; it != fileList.end(); ++it) {
	if (!builder.mergeFile(*it)) {
		boError() << k_funcinfo << "merging of file " << *it << " failed" << endl;
		return false;
	}
 }
 if (!builder.cleanDoc()) {
	boError() << k_funcinfo << "failed cleaning the xml doc" << endl;
	return false;
 }
 bool ret = true;
 if (!builder.makeUfoMenuBar(m->menuBarData(), "MenuBar")) {
	boError() << k_funcinfo << "building menubar GUI failed" << endl;
	ret = false;
 }

 // TODO: honor the "name" attribute of Toolbar tags, atm we support only one
 // ToolBar
 if (!builder.makeUfoToolBar(m->toolBarData(), "ToolBar")) {
	boError() << k_funcinfo << "building toolbar GUI failed" << endl;
	ret = false;
 }
 return ret;
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

		mMenuBar = 0;
	}

	BoUfoManager* mUfoManager;
	BoUfoActionCollection* mActionCollection;

	ufo::UMenuBar* mMenuBar;
	bool mIsVisible;
};

BoUfoMenuBar::BoUfoMenuBar(BoUfoManager* parent, const QString& name)
	: BoUfoMenuBarMenu(QString::null, parent, name)
{
 d = new BoUfoMenuBarPrivate;
 d->mIsVisible = true;
 d->mUfoManager = parent;
 d->mActionCollection = parent->actionCollection();
}

BoUfoMenuBar::~BoUfoMenuBar()
{
 clearUfoMenuBar();
 delete d;
}

void BoUfoMenuBar::setVisible(bool v)
{
 d->mIsVisible = v;
 if (ufoMenuBar()) {
	ufoMenuBar()->setVisible(v);
 }
}

ufo::UMenuBar* BoUfoMenuBar::ufoMenuBar() const
{
 return d->mMenuBar;
}

void BoUfoMenuBar::clearUfoMenuBar()
{
 if (d->mMenuBar && d->mUfoManager) {
	d->mUfoManager->setMenuBarData(0); // deletes the ufo menubar
	d->mMenuBar = 0;
 }
}

void BoUfoMenuBar::createMenuBar()
{
 BO_CHECK_NULL_RET(d->mUfoManager);
 BO_CHECK_NULL_RET(d->mUfoManager->rootPane());
 d->mUfoManager->makeContextCurrent();
 clearUfoMenuBar();
 d->mMenuBar = new ufo::UMenuBar();
 d->mMenuBar->setVisible(d->mIsVisible);
 d->mMenuBar->setFont(d->mUfoManager->rootPane()->getFont());
 d->mUfoManager->rootPane()->setMenuBar(d->mMenuBar);
 if (d->mMenuBar->getParent()) {
	// AB: this is a hack.
	//     if the parent is ever changed in some way in libufo it'll stop
	//     working.
	//     actually libufo should make sure that the menubar is always on
	//     top, but it doesn't, so we ensure this manually here.
	d->mMenuBar->getParent()->setIndexOf(d->mMenuBar, 0);
 }

 createUfoMenuBarSubMenu(d->mMenuBar);
}

void BoUfoMenuBar::initMenuBar(BoUfoManager* m)
{
 BO_CHECK_NULL_RET(m);
 BO_CHECK_NULL_RET(m->actionCollection());
 if (m->menuBarData()) {
	return;
 }
 BoUfoMenuBar* bar = new BoUfoMenuBar(m, "menubar");
 m->setMenuBarData(bar);
}


class BoUfoToolBarPrivate
{
public:
	BoUfoToolBarPrivate()
	{
		mUfoManager = 0;
		mActionCollection = 0;
	}

	BoUfoManager* mUfoManager;
	BoUfoActionCollection* mActionCollection;

	bool mIsVisible;
};

BoUfoToolBar::BoUfoToolBar(BoUfoManager* parent, const QString& name)
	: BoUfoMenuBarMenu(QString::null, parent, name)
{
 d = new BoUfoToolBarPrivate;
 d->mIsVisible = true;
 d->mUfoManager = parent;
 d->mActionCollection = parent->actionCollection();
}

BoUfoToolBar::~BoUfoToolBar()
{
 clearUfoToolBar();
 delete d;
}

void BoUfoToolBar::setVisible(bool v)
{
 d->mIsVisible = v;
 if (d->mUfoManager && d->mUfoManager->toolBarContentWidget()) {
	d->mUfoManager->toolBarContentWidget()->setVisible(d->mIsVisible);
 }
}

void BoUfoToolBar::clearUfoToolBar()
{
 if (d->mUfoManager && d->mUfoManager->toolBarContentWidget()) {
	d->mUfoManager->toolBarContentWidget()->removeAllWidgets();
	d->mUfoManager->toolBarContentWidget()->setVisible(false);
 }
}

void BoUfoToolBar::createToolBar()
{
 BO_CHECK_NULL_RET(d->mUfoManager);
 BO_CHECK_NULL_RET(d->mUfoManager->rootPane());
 BO_CHECK_NULL_RET(d->mUfoManager->toolBarContentWidget());
 d->mUfoManager->makeContextCurrent();
 clearUfoToolBar();
 BO_CHECK_NULL_RET(d->mUfoManager->toolBarContentWidget());
 d->mUfoManager->toolBarContentWidget()->ufoWidget()->setFont(d->mUfoManager->rootPane()->getFont());
 d->mUfoManager->toolBarContentWidget()->setVisible(d->mIsVisible);

 createUfoToolBarSubMenu(d->mUfoManager->toolBarContentWidget()->ufoWidget());
}

void BoUfoToolBar::initToolBar(BoUfoManager* m)
{
 BO_CHECK_NULL_RET(m);
 BO_CHECK_NULL_RET(m->actionCollection());
 if (m->toolBarData()) {
	return;
 }
 BoUfoToolBar* bar = new BoUfoToolBar(m, "toolbar");
 m->setToolBarData(bar);
}





class BoUfoMenuBarMenuPrivate
{
public:
	BoUfoMenuBarMenuPrivate()
	{
	}
	Q3ValueList<BoUfoMenuBarItem*> mItems;
};

BoUfoMenuBarMenu::BoUfoMenuBarMenu(const QString& text, QObject* parent, const QString& name)
	: BoUfoMenuBarItem(0, text, parent, name)
{
 d = new BoUfoMenuBarMenuPrivate;
}

BoUfoMenuBarMenu::~BoUfoMenuBarMenu()
{
 Q3ValueList<BoUfoMenuBarItem*>::Iterator it;
 for (it = d->mItems.begin(); it != d->mItems.end(); ++it) {
	delete *it;
 }
 d->mItems.clear();
 delete d;
}

void BoUfoMenuBarMenu::addSeparator()
{
 // FIXME: UMenu has its own separator - we should use them!
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

const Q3ValueList<BoUfoMenuBarItem*>& BoUfoMenuBarMenu::items() const
{
 return d->mItems;
}

void BoUfoMenuBarMenu::createUfoMenuBarSubMenu(ufo::UWidget* parentWidget)
{
 BO_CHECK_NULL_RET(parentWidget);

 for (unsigned int i = 0; i < itemCount(); i++) {
	if (qobject_cast<BoUfoMenuBarMenu*>(items()[i])) {
		BoUfoMenuBarMenu* m = (BoUfoMenuBarMenu*)items()[i];

		// TODO: we could provide an icon
		QByteArray tmp = m->text().toAscii();
		ufo::UMenu* menu = new ufo::UMenu(std::string(tmp.constData(), tmp.length()));
		menu->setFont(parentWidget->getFont());
		parentWidget->add(menu);

		m->createUfoMenuBarSubMenu(menu);
	} else {
		BoUfoMenuBarItem* item = items()[i];
		BoUfoAction* action = item->action();
		if (!action) {
			// e.g. a separator
			ufo::UMenuItem* menuItem = 0;
			QByteArray tmp = item->text().toAscii();
			menuItem = new ufo::UMenuItem(std::string(tmp.constData(), tmp.length()));
			menuItem->setFont(parentWidget->getFont());
			parentWidget->add(menuItem);
		} else {
//			boDebug() << k_funcinfo << "plugging action " << action->name() << endl;
			action->plug(parentWidget);
		}
	}
 }
}

void BoUfoMenuBarMenu::createUfoToolBarSubMenu(ufo::UWidget* parentWidget)
{
 BO_CHECK_NULL_RET(parentWidget);

 for (unsigned int i = 0; i < itemCount(); i++) {
	if (qobject_cast<BoUfoMenuBarMenu*>(items()[i])) {
		BoUfoMenuBarMenu* m = (BoUfoMenuBarMenu*)items()[i];

		// TODO: we could provide an icon
#if 0
		ufo::UMenu* menu = new ufo::UMenu(m->text().latin1());
		menu->setFont(parentWidget->getFont());
		parentWidget->add(menu);

		m->createUfoToolBarSubMenu(menu);
#else
		boDebug() << k_funcinfo << "submenus not yet supported by toolbar (" << m->text() << ")" << endl;
		m->createUfoToolBarSubMenu(parentWidget);
#endif
	} else {
		BoUfoMenuBarItem* item = items()[i];
		BoUfoAction* action = item->action();
		if (!action) {
			// e.g. a separator
			ufo::UMenuItem* menuItem = 0;
			QByteArray tmp = item->text().toAscii();
			menuItem = new ufo::UMenuItem(std::string(tmp.constData(), tmp.length()));
			menuItem->setFont(parentWidget->getFont());
			parentWidget->add(menuItem);
		} else {
//			boDebug() << k_funcinfo << "plugging action " << action->name() << endl;
			action->plug(parentWidget);
		}
	}
 }
}


BoUfoMenuBarItem::BoUfoMenuBarItem(BoUfoAction* action, const QString& text, QObject* parent, const QString& name)
	: QObject(parent)
{
 setObjectName(name);
 mAction = action;
 mText = text;
}

BoUfoMenuBarItem::~BoUfoMenuBarItem()
{
}


struct BoUfoStdActionInfo
{
	BoUfoStdAction::StdAction id;
	KStandardShortcut::StandardShortcut globalAccel;
	int shortcut;
	QString name;
	const char* label;
	const char* whatsThis;
	const char* iconName;
};

const BoUfoStdActionInfo g_actionInfo[] = {
	// File menu
	{ BoUfoStdAction::FileNew, KStandardShortcut::New, 0, "file_new", I18N_NOOP("&New"), 0, "filenew"},
	{ BoUfoStdAction::FileOpen, KStandardShortcut::Open, 0, "file_open", I18N_NOOP("&Open..."), 0, "fileopen"},
	{ BoUfoStdAction::FileSave, KStandardShortcut::Save, 0, "file_save", I18N_NOOP("&Save"), 0, "filesave"},
	{ BoUfoStdAction::FileSaveAs, KStandardShortcut::AccelNone, 0, "file_save_as", I18N_NOOP("Save &As..."), 0, "filesaveas"},
	{ BoUfoStdAction::FileClose, KStandardShortcut::Close, 0, "file_close", I18N_NOOP("&Close"), 0, "fileclose"},
	{ BoUfoStdAction::FileQuit, KStandardShortcut::Quit, 0, "file_quit", I18N_NOOP("&Quit"), 0, "filequit"},

	// Game menu
	{ BoUfoStdAction::GameNew, KStandardShortcut::New, 0, "game_new", I18N_NOOP("&New"), 0, "filenew"},
	{ BoUfoStdAction::GameLoad, KStandardShortcut::Open, 0, "game_load", I18N_NOOP("&Load..."), 0, "fileopen"},
	{ BoUfoStdAction::GameSave, KStandardShortcut::Save, 0, "game_save", I18N_NOOP("&Save"), 0, "filesave"},
	{ BoUfoStdAction::GameSaveAs, KStandardShortcut::AccelNone, 0, "game_save_as", I18N_NOOP("Save &As..."), 0, "filesaveas"},
	{ BoUfoStdAction::GameEnd, KStandardShortcut::End, 0, "game_end", I18N_NOOP("&End Game"), 0, "fileclose"},
	{ BoUfoStdAction::GamePause, KStandardShortcut::AccelNone, Qt::Key_P, "game_pause", I18N_NOOP("Pa&use"), 0, "player_pause"},
	{ BoUfoStdAction::GameQuit, KStandardShortcut::Quit, 0, "game_quit", I18N_NOOP("&Quit"), 0, "exit"},

	// Edit menu
	{ BoUfoStdAction::EditUndo, KStandardShortcut::Undo, 0, "edit_undo", I18N_NOOP("&Undo"), 0, "undo"},
	{ BoUfoStdAction::EditRedo, KStandardShortcut::Redo, 0, "edit_redo", I18N_NOOP("Re&do"), 0, "redo"},

	// Settings menu
	{ BoUfoStdAction::ShowMenubar, KStandardShortcut::ShowMenubar, 0, "options_show_menubar", I18N_NOOP("Show &Menubar"), 0, "showmenu"},
	{ BoUfoStdAction::ShowToolbar, KStandardShortcut::AccelNone, 0, "options_show_toolbar", I18N_NOOP("Show &Toolbar"), 0, 0},
	{ BoUfoStdAction::ShowStatusbar, KStandardShortcut::AccelNone, 0, "options_show_statusbar", I18N_NOOP("Show St&atusbar"), 0, 0},
	{ BoUfoStdAction::KeyBindings, KStandardShortcut::AccelNone, 0, "options_configure_keybinding", I18N_NOOP("Configure S&hortcuts..."), 0, "configure_shortcuts"},
	{ BoUfoStdAction::Preferences, KStandardShortcut::AccelNone, 0, "options_configure", I18N_NOOP("&Configure %1..."), 0, "configure"},


	// TODO: In KDE > 3.3 there is a "fullscreen" action in
	// ui_standards.rc. Use it!
	// -> For that we need to make sure that an action that appears in two
	// different files will appear only once in the resulting file after
	// merging, otherwise the fullscreen menu item will appear twice for
	// some people (once from ui_standards.rc and once from our own *ui.rc
	// file) or not at all (if we remove it from our files, it won't appear
	// for people with KDE <= 3.3)
	{ BoUfoStdAction::FullScreen, KStandardShortcut::FullScreen, 0, "window_fullscreen", I18N_NOOP("F&ull Screen Mode"), 0, "window_fullscreen"},
};

const BoUfoStdActionInfo* infoPtr(BoUfoStdAction::StdAction id)
{
 for (unsigned int i = 0; g_actionInfo[i].id != BoUfoStdAction::ActionNone; i++) {
	if (g_actionInfo[i].id == id) {
		return &g_actionInfo[i];
	}
 }
 return 0;
}

BoUfoAction* BoUfoStdAction::create(StdAction id, const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const QString& name)
{
 BoUfoAction* action = 0;
 const BoUfoStdActionInfo* info = infoPtr(id);
 if (!info) {
	boError() << k_funcinfo << "no info about " << (int)id << endl;
	return 0;
 }
 QString label;
 KShortcut cut = (info->globalAccel == KStandardShortcut::AccelNone
		? KShortcut(info->shortcut)
		: KStandardShortcut::shortcut(info->globalAccel));
 QString n = (!name.isNull()) ? name : info->name;
 switch (id) {
	case Preferences:
	{
#warning TODO: port to Qt4
#if 0
		const KAboutData* aboutData = KGlobal::instance()->aboutData();
		QByteArray tmp = d->mText.toAscii();
		QString appName = (aboutData) ? aboutData->programName() : QString::fromLatin1(qApp->name());
		label = i18n(info->label, appName);
#endif
		break;
	}
	default:
		label = i18n(info->label);
		break;
 }
 switch (id) {
	case ShowMenubar:
	case ShowToolbar:
	case ShowStatusbar:
	case GamePause:
	case FullScreen:
		// TODO: iconName
		action = new BoUfoToggleAction(label, cut, receiver, slot, parent, n);
		break;
	default:
		// TODO: iconName
		action = new BoUfoAction(label, cut, receiver, slot, parent, n);
		break;
 }
 return action;
}

QString BoUfoStdAction::name(BoUfoStdAction::StdAction id)
{
 const BoUfoStdActionInfo* info = infoPtr(id);
 if (info) {
	return info->name;
 }
 return 0;
}

// AB: this may break i18n() !
// -> I18N_NOOP is used for the label. I don't know how to handle that correctly!
const char* BoUfoStdAction::label(BoUfoStdAction::StdAction id)
{
 const BoUfoStdActionInfo* info = infoPtr(id);
 if (info) {
	return info->label;
 }
 return 0;
}

#define ACTION(name1, name2, ret) \
	ret* BoUfoStdAction::name1(const QObject* receiver, const char* slot, BoUfoActionCollection* parent, const QString& name) \
	{ return (ret*)BoUfoStdAction::create(name2, receiver, slot, parent, name); }

ACTION(fileNew, FileNew, BoUfoAction)
ACTION(fileOpen, FileOpen, BoUfoAction)
ACTION(fileSave, FileSave, BoUfoAction)
ACTION(fileSaveAs, FileSaveAs, BoUfoAction)
ACTION(fileClose, FileClose, BoUfoAction)
ACTION(fileQuit, FileQuit, BoUfoAction)

ACTION(editUndo, EditUndo, BoUfoAction)
ACTION(editRedo, EditRedo, BoUfoAction)

ACTION(gameNew, GameNew, BoUfoAction)
ACTION(gameLoad, GameLoad, BoUfoAction)
ACTION(gameSave, GameSave, BoUfoAction)
ACTION(gameSaveAs, GameSaveAs, BoUfoAction)
ACTION(gameEnd, GameEnd, BoUfoAction)
ACTION(gamePause, GamePause, BoUfoToggleAction)
ACTION(gameQuit, GameQuit, BoUfoAction)

ACTION(showMenubar, ShowMenubar, BoUfoToggleAction)
ACTION(showToolbar, ShowToolbar, BoUfoToggleAction)
ACTION(showStatusbar, ShowStatusbar, BoUfoToggleAction)
ACTION(keyBindings, KeyBindings, BoUfoAction)
ACTION(preferences, Preferences, BoUfoAction)

ACTION(fullScreen, FullScreen, BoUfoToggleAction)

#undef ACTION

