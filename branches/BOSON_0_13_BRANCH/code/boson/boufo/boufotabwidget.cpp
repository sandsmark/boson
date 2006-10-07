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

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufotabwidget.h"
#include "boufotabwidget.moc"

#include "boufopushbutton.h"
#include <bodebug.h>

#include <qintdict.h>

class BoUfoTabWidgetPrivate
{
public:
	BoUfoTabWidgetPrivate()
	{
		mButtonsWidget = 0;
		mTabLayoutWidget = 0;
	}
	BoUfoHBox* mButtonsWidget;
	BoUfoVBox* mTabLayoutWidget;

	QIntDict<BoUfoPushButton> mButtons;
	QIntDict<BoUfoWidget> mTabs; // ownership is NOT taken
	int mCurrentTab;
};


BoUfoTabWidget::BoUfoTabWidget() : BoUfoWidget()
{
 init();
}

BoUfoTabWidget::~BoUfoTabWidget()
{
 delete d;
}

void BoUfoTabWidget::init()
{
 d = new BoUfoTabWidgetPrivate;
 BoUfoVBox* topLayoutWidget = new BoUfoVBox();
 addWidget(topLayoutWidget);

 d->mButtonsWidget = new BoUfoHBox();
 d->mTabLayoutWidget = new BoUfoVBox();

 topLayoutWidget->addWidget(d->mButtonsWidget);
 topLayoutWidget->addWidget(d->mTabLayoutWidget);
 d->mCurrentTab = -1;
}

void BoUfoTabWidget::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 for (QIntDictIterator<BoUfoPushButton> it(d->mButtons); it.current(); ++it) {
	it.current()->setOpaque(o);
 }
 for (QIntDictIterator<BoUfoWidget> it(d->mTabs); it.current(); ++it) {
	it.current()->setOpaque(o);
 }
}

int BoUfoTabWidget::addTab(BoUfoWidget* widget, const QString& label)
{
 if (!widget) {
	BO_NULL_ERROR(widget);
	return -1;
 }
 int id = findId();
 if (id < 0) { // will never happen
	boError() << k_funcinfo << "invalid id" << endl;
	return -1;
 }

 BoUfoPushButton* button = new BoUfoPushButton(label);
 d->mButtons.insert(id, button);
 d->mTabs.insert(id, widget);
 connect(button, SIGNAL(signalClicked()), this, SLOT(slotButtonClicked()));

 d->mButtonsWidget->addWidget(button);
 d->mTabLayoutWidget->addWidget(widget);

 widget->hide();
 if (!currentTab()) {
	setCurrentTab(id);
 }
 return id;
}

void BoUfoTabWidget::removeTab(BoUfoWidget* widget)
{
 QIntDictIterator<BoUfoWidget> it(d->mTabs);
 int id = -1;
 while (it.current() && it.current() != widget) {
	++it;
 }
 if (it.current()) {
	id = it.currentKey();
 } else {
	boWarning() << k_funcinfo << "did not find tab" << endl;
	return;
 }

 BoUfoPushButton* b = d->mButtons[id];
 BoUfoWidget* tab = d->mTabs[id];
 d->mTabs.remove(id);
 d->mButtons.remove(id);

#warning TODO
 boWarning() << k_funcinfo << "removing tabs does not work correctly yet" << endl;
 // remove from mButtonsWidget and mTabLayoutWidget
}

void BoUfoTabWidget::slotButtonClicked()
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->inherits("BoUfoPushButton")) {
	boError() << k_funcinfo << "sender() is not a BoUfoPushButton" << endl;
	return;
 }
 BoUfoPushButton* senderButton = (BoUfoPushButton*)sender();
 QIntDictIterator<BoUfoPushButton> it(d->mButtons);
 while (it.current()) {
	if (it.current() == senderButton) {
		setCurrentTab(it.currentKey());
		return;
	}
	++it;
 }
}

void BoUfoTabWidget::setCurrentTab(int id)
{
 BoUfoWidget* w = currentTab();
 if (w) {
	w->hide();
 }
 d->mCurrentTab = id;
 w = currentTab();
 if (!w) {
	if (d->mTabs.count() > 0) {
#warning FIXME: we use ID, not index!
		setCurrentTab(0);
	}
	return;
 }
 w->show();
 invalidate();
}

BoUfoWidget* BoUfoTabWidget::currentTab() const
{
 return d->mTabs[d->mCurrentTab];
}

int BoUfoTabWidget::findId() const
{
 int i = 0;
 while (d->mTabs[i]) {
	i++;
 }
 return i;
}



