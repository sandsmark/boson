/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "optionsdialog.h"
#include "optionsdialog.moc"

#include "../../bomemory/bodummymemory.h"
#include "optionswidgets.h"
#include "opengloptions.h"
#include "bodebug.h"

#include <klocale.h>

#include <q3vbox.h>
#include <q3ptrlist.h>

class OptionsDialogPrivate
{
public:
	OptionsDialogPrivate()
	{
	}
	Q3PtrList<OptionsWidget> mOptionsWidgets;
};

OptionsDialog::OptionsDialog(QWidget* parent)
	: KPageDialog( parent)
{
 d = new OptionsDialogPrivate;
 setFaceType(KPageDialog::Tabbed);
 setWindowTitle(KDialog::makeStandardCaption(i18n("Boson Options")));
 setButtons(KDialog::Ok | KDialog::Apply | KDialog::Default);
 setDefaultButton(KDialog::Cancel);

 initGeneralPage();
 initCursorPage();
 initScrollingPage();
 initSoundsPage();
 initOpenGLPage();
 initWaterPage();
 initChatPage();
 initToolTipPage();
}

OptionsDialog::~OptionsDialog()
{
 boDebug(210) << k_funcinfo << endl;
 delete d;
}

void OptionsDialog::initGeneralPage()
{
 GeneralOptions* o = new GeneralOptions(0);
 addPage(o, i18n("&General"));
 addOptions(o);
}

void OptionsDialog::initCursorPage()
{
 CursorOptions* o = new CursorOptions(0);
 addPage(o, i18n("C&ursor"));
 connect(o, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SIGNAL(signalCursorChanged(int, const QString&)));
 addOptions(o);
}

void OptionsDialog::initScrollingPage()
{
 ScrollingOptions* o = new ScrollingOptions(0);
 addPage(o, i18n("&Scrolling"));
 addOptions(o);
}

void OptionsDialog::initSoundsPage()
{
 SoundOptions* o = new SoundOptions(0);
 addPage(o, i18n("S&ounds"));
 addOptions(o);
}

void OptionsDialog::initOpenGLPage()
{
 OpenGLOptions* o = new OpenGLOptions(0);
 addPage(o, i18n("&OpenGL"));
 connect(o, SIGNAL(signalFontChanged(const BoFontInfo&)),
		this, SIGNAL(signalFontChanged(const BoFontInfo&)));
 addOptions(o);
}

void OptionsDialog::initWaterPage()
{
 WaterOptions* o = new WaterOptions(0);
 addPage(o, i18n("&Water"));
 addOptions(o);
}

void OptionsDialog::initChatPage()
{
 ChatOptions* o = new ChatOptions(0);
 addPage(o, i18n("C&hat"));
 addOptions(o);
}

void OptionsDialog::initToolTipPage()
{
 ToolTipOptions* o = new ToolTipOptions(0);
 addPage(o, i18n("&Tool Tips"));
 addOptions(o);
}

void OptionsDialog::addOptions(OptionsWidget* o)
{
 d->mOptionsWidgets.append(o);
}

void OptionsDialog::slotLoad()
{
 Q3PtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->load();
 }
}

void OptionsDialog::slotDefault()
{
 Q3PtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->setDefaults();
 }
}

void OptionsDialog::slotOk()
{
 boDebug(210) << k_funcinfo << endl;
 slotApply();
 accept();
 boDebug(210) << k_funcinfo << "done" << endl;
}

void OptionsDialog::slotApply()
{
 boDebug(210) << k_funcinfo << endl;
 Q3PtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->apply();
 }
 emit signalApply();
 boDebug(210) << k_funcinfo << "done" << endl;
}

