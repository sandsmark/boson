/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>
#include <ufo/gl/ugl_image.hpp>
#include <ufo/ux/ux.hpp>
#include <ufo/events/umousewheelevent.hpp>
#include <ufo/events/ukeysym.hpp>
#include <ufo/widgets/uslider.hpp>
#include <ufo/ui/uuidefs.hpp>
#include "ufoext/ubogridlayout.h"

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufodebugwidget.h"
#include "boufodebugwidget.moc"

#include "boufoimage.h"
#include "boufodrawable.h"
#include "boufomanager.h"
#include "boufowidget.h"

#include <bodebug.h>

#include <klocale.h>

#include <qlistview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qobjectlist.h>
#include <qcheckbox.h>
#include <qscrollview.h>

#include <math.h>

class BoUfoDebugWidgetPrivate
{
public:
	BoUfoDebugWidgetPrivate()
	{
		mToolkitLabel = 0;
		mDisplayLabel = 0;
		mContextLabel = 0;
		mListView = 0;
		mDetailedWidgetView = 0;

		mManager = 0;
	}

	QLabel* mToolkitLabel;
	QLabel* mDisplayLabel;
	QLabel* mContextLabel;
	QListView* mListView;
	BoUfoDebugSingleWidget* mDetailedWidgetView;

	BoUfoManager* mManager;

	QMap<ufo::UWidget*, BoUfoWidget*> mUfoWidget2BoUfoWidget;
	QMap<QListViewItem*, ufo::UWidget*> mItem2Widget;
};

BoUfoDebugWidget::BoUfoDebugWidget(QWidget* parent)
	: QWidget(parent)
{
 d = new BoUfoDebugWidgetPrivate;

 QVBoxLayout* layout = new QVBoxLayout(this);

 d->mToolkitLabel = new QLabel(this);
 layout->addWidget(d->mToolkitLabel);

 d->mDisplayLabel = new QLabel(this);
 layout->addWidget(d->mDisplayLabel);

 d->mContextLabel = new QLabel(this);
 layout->addWidget(d->mContextLabel);

 QSplitter* vSplitter = new QSplitter(Vertical, this);
 layout->addWidget(vSplitter);

 d->mListView = new QListView(vSplitter);
 d->mListView->setRootIsDecorated(true);
 d->mListView->setAllColumnsShowFocus(true);
 connect(d->mListView, SIGNAL(currentChanged(QListViewItem*)),
		this, SLOT(slotWidgetChanged(QListViewItem*)));
 QScrollView* scroll = new QScrollView(vSplitter);
 scroll->setResizePolicy(QScrollView::AutoOneFit);
 d->mDetailedWidgetView = new BoUfoDebugSingleWidget(scroll);
 scroll->addChild(d->mDetailedWidgetView);

 d->mListView->addColumn(i18n("Ufo Widget"));
 d->mListView->addColumn(i18n("BoUfoWidget"));
 d->mListView->addColumn(i18n("UWidget class"));
 d->mListView->addColumn(i18n("BoUfoWidget class"));
 d->mListView->addColumn(i18n("layout class"));
 d->mListView->addColumn(i18n("rootX"));
 d->mListView->addColumn(i18n("rootY"));
 d->mListView->addColumn(i18n("w"));
 d->mListView->addColumn(i18n("h"));
}

BoUfoDebugWidget::~BoUfoDebugWidget()
{
 delete d;
}

void BoUfoDebugWidget::setBoUfoManager(BoUfoManager* manager)
{
 d->mManager = manager;
 d->mToolkitLabel->setText(i18n("Ufo toolkit: 0x%1").
		arg(QString::number((unsigned long int)manager->toolkit(), 16)));
 d->mDisplayLabel->setText(i18n("Ufo display: 0x%1").
		arg(QString::number((unsigned long int)manager->display(), 16)));
 d->mContextLabel->setText(i18n("Ufo context: 0x%1").
		arg(QString::number((unsigned long int)manager->context(), 16)));

 d->mListView->clear();
 d->mDetailedWidgetView->setWidget(0, 0);

 d->mItem2Widget.clear();
 d->mUfoWidget2BoUfoWidget.clear();
 const QObjectList* objectList = QObject::objectTrees();
 for (QObjectListIterator it(*objectList); it.current(); ++it) {
	if (!it.current()->inherits("BoUfoWidget")) {
		continue;
	}
	BoUfoWidget* w = (BoUfoWidget*)it.current();
	if (!w->ufoWidget()) {
		boError() << k_funcinfo << "BoUfoWidget has NULL ufo widget" << endl;
		continue;
	}
	if (d->mUfoWidget2BoUfoWidget.contains(w->ufoWidget())) {
		boError() << k_funcinfo << "ufo widget already in map" << endl;
		continue;
	}
	d->mUfoWidget2BoUfoWidget.insert(w->ufoWidget(), w);
 }

 BO_CHECK_NULL_RET(d->mManager->rootPane());
 addWidget(d->mManager->rootPane(), new QListViewItem(d->mListView));
}


void BoUfoDebugWidget::addWidget(ufo::UWidget* ufoWidget, QListViewItem* item)
{
 BoUfoWidget* boufoWidget = d->mUfoWidget2BoUfoWidget[ufoWidget];
 d->mItem2Widget.insert(item, ufoWidget);
 const ufo::UClassInfo* classInfo = ufoWidget->getClassInfo();
 item->setOpen(true);
 item->setText(0, QString("0x") + QString::number((unsigned long int)ufoWidget, 16));
 QString boufoWidgetText = QString("0x") + QString::number((unsigned long int)boufoWidget, 16);
 if (boufoWidget) {
	if (QString(boufoWidget->name()) != QString("unnamed")) {
		boufoWidgetText = i18n("%1 (%2)").arg(boufoWidgetText).arg(boufoWidget->name());
	}
 }
 item->setText(1, boufoWidgetText);
 item->setText(2, classInfo->getClassName().c_str());
 if (boufoWidget) {
	item->setText(3, boufoWidget->className());
 } else {
	item->setText(3, i18n("(null)"));
 }
 if (ufoWidget->getLayout()) {
	item->setText(4, ufoWidget->getLayout()->getClassInfo()->getClassName().c_str());
 } else {
	item->setText(4, i18n("(null"));
 }
 item->setText(5, QString::number(ufoWidget->getRootLocation().x));
 item->setText(6, QString::number(ufoWidget->getRootLocation().y));
 item->setText(7, QString::number(ufoWidget->getWidth()));
 item->setText(8, QString::number(ufoWidget->getHeight()));


 std::vector<ufo::UWidget*> children = ufoWidget->getWidgets();
 for (std::vector<ufo::UWidget*>::iterator it = children.begin(); it != children.end(); ++it) {
	addWidget(*it, new QListViewItem(item));
 }
}

void BoUfoDebugWidget::slotWidgetChanged(QListViewItem* item)
{
 ufo::UWidget* w = 0;
 if (item) {
	w = d->mItem2Widget[item];
 }
 BoUfoWidget* bw = 0;
 if (w) {
	bw = d->mUfoWidget2BoUfoWidget[w];
 }
 d->mDetailedWidgetView->setWidget(w, bw);
}





class BoUfoDebugSingleWidgetPrivate
{
public:
	BoUfoDebugSingleWidgetPrivate()
	{
		mUfoWidget = 0;
		mBoUfoWidget = 0;
		mUfoString = 0;
		mUfoName = 0;

		mUfoClass = 0;
		mBoUfoClass = 0;
		mLayout = 0;
		mRootBounds = 0;
		mBounds = 0;
		mVisible = 0;
		mEnabled = 0;

		mLayoutHint = 0;
		mStretch = 0;
		mChildrenStretch = 0;
		mPreferredSize = 0;
	}
	QLabel* mUfoWidget;
	QLabel* mBoUfoWidget;
	QLabel* mUfoString;
	QLabel* mUfoName;

	QLabel* mUfoClass;
	QLabel* mBoUfoClass;
	QLabel* mLayout;
	QLabel* mRootBounds;
	QLabel* mBounds;
	QCheckBox* mVisible;
	QCheckBox* mEnabled;

	QLabel* mLayoutHint;
	QLabel* mStretch;
	QLabel* mChildrenStretch;
	QLabel* mPreferredSize;
};

BoUfoDebugSingleWidget::BoUfoDebugSingleWidget(QWidget* parent)
	: QWidget(parent)
{
 d = new BoUfoDebugSingleWidgetPrivate();
 QGridLayout* grid = new QGridLayout(this, -1, 2);
 QVBoxLayout* vlayout1 = new QVBoxLayout(grid);
 QVBoxLayout* vlayout2 = new QVBoxLayout(grid);

 d->mUfoWidget = new QLabel(this);
 vlayout1->addWidget(d->mUfoWidget);
 d->mBoUfoWidget = new QLabel(this);
 vlayout1->addWidget(d->mBoUfoWidget);
 d->mUfoString = new QLabel(this);
 vlayout1->addWidget(d->mUfoString);
 d->mUfoName = new QLabel(this);
 vlayout1->addWidget(d->mUfoName);

 d->mUfoClass = new QLabel(this);
 vlayout1->addWidget(d->mUfoClass);
 d->mBoUfoClass = new QLabel(this);
 vlayout1->addWidget(d->mBoUfoClass);
 d->mLayout = new QLabel(this);
 vlayout1->addWidget(d->mLayout);
 d->mRootBounds = new QLabel(this);
 vlayout1->addWidget(d->mRootBounds);
 d->mBounds = new QLabel(this);
 vlayout1->addWidget(d->mBounds);
 d->mVisible = new QCheckBox(this);
 d->mVisible->setText(i18n("Visible"));
 d->mVisible->setEnabled(false);
 vlayout1->addWidget(d->mVisible);
 d->mEnabled= new QCheckBox(this);
 d->mEnabled->setText(i18n("Enabled"));
 d->mEnabled->setEnabled(false);
 vlayout1->addWidget(d->mEnabled);


 d->mLayoutHint= new QLabel(this);
 vlayout2->addWidget(d->mLayoutHint);
 d->mStretch = new QLabel(this);
 vlayout2->addWidget(d->mStretch);
 d->mChildrenStretch = new QLabel(this);
 vlayout2->addWidget(d->mChildrenStretch);
 d->mPreferredSize = new QLabel(this);
 vlayout2->addWidget(d->mPreferredSize);
}

BoUfoDebugSingleWidget::~BoUfoDebugSingleWidget()
{
 delete d;
}

void BoUfoDebugSingleWidget::setWidget(ufo::UWidget* u, BoUfoWidget* w)
{
 if (!u) {
	hide();
	return;
 }
 show();
 d->mUfoWidget->setText(i18n("Selected Ufo Widget: 0x%1").
		arg(QString::number((unsigned long int)u, 16)));
 d->mBoUfoWidget->setText(i18n("BoUfo Widget: 0x%1%2").
		arg(QString::number((unsigned long int)w, 16)).
		arg(w ? (QString(" (%1)").arg(w->name())) : QString("")));
 d->mUfoString->setText(i18n("Ufo widget string: %1").
		arg(u->toString().c_str()));
 d->mUfoName->setText(i18n("Ufo name: %1").
		arg(u->getName().c_str()));

 const ufo::UClassInfo* classInfo = u->getClassInfo();
 d->mUfoClass->setText(i18n("Ufo class: %1").arg(classInfo->getClassName().c_str()));
 QString boufoClass = i18n("(null)");
 if (w) {
	boufoClass = w->className();
 }
 d->mBoUfoClass->setText(i18n("BoUfo class: %1").arg(boufoClass));
 QString layoutClass = i18n("(null)");
 if (u->getLayout()) {
	 ufo::ULayoutManager* l = u->getLayout();
	const ufo::UClassInfo* i = l->getClassInfo();
	BO_CHECK_NULL_RET(i);
	layoutClass = i->getClassName().c_str();
 }
 d->mLayout->setText(i18n("Layout class: %1").arg(layoutClass));
 d->mRootBounds->setText(i18n("Absolute bounds: x=%1 y=%2 w=%3 h=%4").
	arg(u->getRootLocation().x).
	arg(u->getRootLocation().y).
	arg(u->getWidth()).
	arg(u->getHeight()));
 d->mBounds->setText(i18n("Relative (to parent) bounds: x=%1 y=%2 w=%3 h=%4").
	arg(u->getX()).
	arg(u->getY()).
	arg(u->getWidth()).
	arg(u->getHeight()));
 d->mVisible->setChecked(u->isVisible());
 d->mEnabled->setChecked(u->isEnabled());

 d->mLayoutHint->setText(i18n("Layout Hint: %1").arg(u->getString("layout").c_str()));
 d->mStretch->setText(i18n("Stretch: %1").arg(u->getString("stretch").c_str()));
 ufo::UDimension preferredSize = u->getPreferredSize();
 d->mPreferredSize->setText(i18n("Preferred Size: w=%1 h=%2").arg(preferredSize.w).arg(preferredSize.h));

 int stretchCount = 0;
 for (unsigned int i = 0; i < u->getWidgetCount(); i++) {
	const ufo::UWidget* w = u->getWidget(i);
	if (!w->isVisible()) {
		continue;
	}
	ufo::UObject* o = w->get("stretch");
	if (o) {
		int s = ufo::UInteger::toInt(o->toString());
		if (s > 0) {
			stretchCount += s;
		}
		if (s < 0) {
			boError() << k_funcinfo << s << endl;
		}
	}
 }
 d->mChildrenStretch->setText(i18n("Children stretch count: %1").arg(stretchCount));
}

