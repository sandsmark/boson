/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonstartupbasewidget.h"
#include "bosonstartupbasewidget.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qpixmap.h>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <stdlib.h>

//#include <klocale.h>
BosonStartupBaseWidget::BosonStartupBaseWidget(QWidget* parent, const char* name)
		: QWidget(parent, name)
{
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addSpacing(20); //FIXME hardcoded

 QPixmap backgroundPix(locate("data", "boson/pics/boson-startup-bg.png"));
 if (backgroundPix.isNull()) {
	KMessageBox::error(this, i18n("You do not have Boson data files installed!\n"
			"Please install data package of Boson and restart Boson."), i18n("Data files not found!"));
	exit(1);  // Evil, but if we'd call qApp->exit(1); then we would return to event loop
	return;
 }
 setPaletteBackgroundPixmap(backgroundPix);


 mLogo = new QLabel(this, "bosonlogo");
 mLogo->setBackgroundOrigin(WindowOrigin);
 QPixmap logoPix(locate("data", "boson/pics/boson-startup-logo.png"));
 mLogo->setPixmap(logoPix);
 topLayout->addWidget(mLogo, 0, AlignHCenter);
 topLayout->addSpacing(10); // FIXME hardcoded

 mPlainWidget = new QWidget(this, "plainwidget");
 QVBoxLayout* l = new QVBoxLayout(mPlainWidget);
 l->setAutoAdd(true);
 mPlainWidget->setBackgroundOrigin(WindowOrigin);
 mPlainWidget->setPaletteForegroundColor(Qt::white);
 topLayout->addWidget(mPlainWidget);
}

BosonStartupBaseWidget::~BosonStartupBaseWidget()
{
}

void BosonStartupBaseWidget::initBackgroundOrigin()
{
 // warning! hack!
 // we need to change the backgroundorigin of all child widgets to
 // WindowOrigin. is there a better way?
 // update: probably not, since we also need to change the grand-childs (thats
 // why we need *Window*Origin - ParentOrigin doesnt work with grandchilds).
 QObjectList* l = queryList("QWidget", 0, true, true);
 QObjectListIt it(*l);
 QWidget* w;
 while ((w = (QWidget*)it.current()) != 0) {
	w->setBackgroundOrigin(WindowOrigin);
	++it;
 }
 delete l;
 // (hack end)
}

