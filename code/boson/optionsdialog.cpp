/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "optionsdialog.h"
#include "bosonconfig.h"
#include "optionswidgets.h"

#include "bosoncursor.h"
#include "defines.h"
#include "bodebug.h"

#include <klocale.h>

#include <qvbox.h>
#include <qptrlist.h>

#include "optionsdialog.moc"

class OptionsDialog::OptionsDialogPrivate
{
public:
	OptionsDialogPrivate()
	{
		mCursorOptions = 0;
	}
	QPtrList<OptionsWidget> mOptionsWidgets;
	CursorOptions* mCursorOptions;
};

OptionsDialog::OptionsDialog(bool editor, QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("Boson Options"), Ok|Apply|Default,
		Cancel, parent, "bosonoptionsdialog", modal, true)
{
 d = new OptionsDialogPrivate;

 if (!editor) {
	initGeneralPage();
	initCursorPage();
 }
 initScrollingPage();
 if (!editor) {
	initSoundsPage();
 }
 initOpenGLPage();
 initChatPage(); // in editor for chat messages
 initToolTipPage();
}

OptionsDialog::~OptionsDialog()
{
 boDebug(210) << k_funcinfo << endl;
 delete d;
}

void OptionsDialog::initGeneralPage()
{
 QVBox* vbox = addVBoxPage(i18n("&General"));
 GeneralOptions* o = new GeneralOptions(vbox);
 connect(o, SIGNAL(signalCmdBackgroundChanged(const QString&)),
		this, SIGNAL(signalCmdBackgroundChanged(const QString&)));
 addOptions(o);
}

void OptionsDialog::initCursorPage()
{
 QVBox* vbox = addVBoxPage(i18n("C&ursor"));
 CursorOptions* o = new CursorOptions(vbox);
 connect(o, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SIGNAL(signalCursorChanged(int, const QString&)));
 addOptions(o);
 d->mCursorOptions = o;
}

void OptionsDialog::initScrollingPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Scrolling"));
 ScrollingOptions* o = new ScrollingOptions(vbox);
 addOptions(o);
}

void OptionsDialog::initSoundsPage()
{
 QVBox* vbox = addVBoxPage(i18n("S&ounds"));
 SoundOptions* o = new SoundOptions(vbox);
 addOptions(o);
}

void OptionsDialog::initOpenGLPage()
{
 QVBox* vbox = addVBoxPage(i18n("&OpenGL"));
 OpenGLOptions* o = new OpenGLOptions(vbox);
 connect(o, SIGNAL(signalGroundRendererChanged(int)),
		this, SIGNAL(signalGroundRendererChanged(int)));
 addOptions(o);
}

void OptionsDialog::initChatPage()
{
 QVBox* vbox = addVBoxPage(i18n("C&hat"));
 ChatOptions* o = new ChatOptions(vbox);
 addOptions(o);
}

void OptionsDialog::initToolTipPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Tool Tips"));
 ToolTipOptions* o = new ToolTipOptions(vbox);
 addOptions(o);
}

void OptionsDialog::addOptions(OptionsWidget* o)
{
 d->mOptionsWidgets.append(o);
}

void OptionsDialog::setGame(Boson* game)
{
 QPtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->setGame(game);
 }
}

void OptionsDialog::setPlayer(Player* p)
{
 QPtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->setLocalPlayer(p);
 }
}

void OptionsDialog::setCursor(CursorMode mode)
{
 if (!d->mCursorOptions) {
	return;
 }
 d->mCursorOptions->setCursor(mode);
}

void OptionsDialog::slotLoad()
{
 QPtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->load();
 }
}

void OptionsDialog::slotDefault()
{
 QPtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
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
 QPtrListIterator<OptionsWidget> it(d->mOptionsWidgets);
 for (; it.current(); ++it) {
	it.current()->apply();
 }
 emit signalApply();
 boDebug(210) << k_funcinfo << "done" << endl;
}

