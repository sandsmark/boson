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

#include "bosoncursor.h"
#include "defines.h"

#include <klocale.h>
#include <knuminput.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qvbox.h>

#include "optionsdialog.moc"

class OptionsDialog::OptionsDialogPrivate
{
public:
	OptionsDialogPrivate()
	{
		mArrowSpeed = 0;
		mGameSpeed = 0;
		mCmdBackground = 0;
		mGroupMove = 0;
		mMiniMapScale = 0;
		
		mCursor = 0;
		mCursorTheme = 0;

		mRMBScrolling = 0;
		mMMBScrolling = 0;
		mCursorEdgeSensity = 0;
	}

	KIntNumInput* mArrowSpeed;
	KIntNumInput* mGameSpeed;
	QComboBox* mCmdBackground;
	QComboBox* mGroupMove;
	KDoubleNumInput* mMiniMapScale;
	
	QComboBox* mCursor;
	QComboBox* mCursorTheme;
	QStringList mCursorThemes;
	QStringList mCmdBackgrounds;

	QCheckBox* mRMBScrolling;
	QCheckBox* mMMBScrolling;
	KIntNumInput* mCursorEdgeSensity;
};

OptionsDialog::OptionsDialog(QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("Boson Options"), Ok|Cancel|Default,
		Cancel, parent, "bosonoptionsdialog", modal, true)
{
 d = new OptionsDialogPrivate;
 
 initGeneralPage();
 initCursorPage();
 initPathfindingPage();
 initScrollingPage();
}

OptionsDialog::~OptionsDialog()
{
 delete d;
}

void OptionsDialog::initGeneralPage()
{
 QVBox* vbox = addVBoxPage(i18n("&General"));

 d->mArrowSpeed = new KIntNumInput(10, vbox);
 d->mArrowSpeed->setRange(1, 200);
 d->mArrowSpeed->setLabel(i18n("Arrow Key Steps"));
 connect(d->mArrowSpeed, SIGNAL(valueChanged(int)), 
		this, SIGNAL(signalArrowScrollChanged(int)));

 d->mGameSpeed = new KIntNumInput(10, vbox);
 d->mGameSpeed->setRange(MIN_GAME_SPEED, MAX_GAME_SPEED);
 d->mGameSpeed->setLabel(i18n("Game Speed"));
 connect(d->mGameSpeed, SIGNAL(valueChanged(int)), 
		this, SLOT(slotSpeedChanged(int)));

 QHBox* hbox = new QHBox(vbox);

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Command Frame Background Pixmap"), hbox);
 d->mCmdBackground = new QComboBox(hbox);
 d->mCmdBackgrounds = KGlobal::dirs()->findAllResources("data", "boson/themes/ui/*/cmdpanel*.png");
 d->mCmdBackground->insertItem(i18n("None"));
 //TODO: display filename only... - not the complete path
 d->mCmdBackground->insertStringList(d->mCmdBackgrounds);
 connect(d->mCmdBackground, SIGNAL(activated(int)), 
		this, SLOT(slotCmdBackgroundChanged(int)));

 d->mMiniMapScale = new KDoubleNumInput(1.0, vbox);
 d->mMiniMapScale->setRange(1.0, 5.0, 0.5);
 d->mMiniMapScale->setLabel(i18n("Mini Map Scale Factor"));
 connect(d->mMiniMapScale, SIGNAL(valueChanged(double)), 
		this, SIGNAL(signalMiniMapScaleChanged(double)));
}

void OptionsDialog::initCursorPage()
{
 QVBox* vbox = addVBoxPage(i18n("C&ursor"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Cursor"), hbox);
 d->mCursor = new QComboBox(hbox);
 d->mCursor->insertItem(i18n("Sprite Cursor"), CursorSprite);
 d->mCursor->insertItem(i18n("B/W Cursor"), CursorNormal);
 d->mCursor->insertItem(i18n("KDE Standard Cursor"), CursorKDE);
 d->mCursor->insertItem(i18n("Experimental Cursor (*Very* Unstable)"), 
		CursorExperimental);
 connect(d->mCursor, SIGNAL(activated(int)), 
		this, SLOT(slotCursorChanged(int)));

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Cursor Theme"), hbox);
 d->mCursorTheme = new QComboBox(hbox);
 QStringList list = BosonCursor::availableThemes();
 for (int i = 0; i < (int)list.count(); i++) {
	KSimpleConfig cfg(list[i] + QString::fromLatin1("/index.desktop"));
	if (!cfg.hasGroup("Boson Cursor")) {
		kdWarning() << "invalid cursor " << list[i] << endl;
	} else {
		cfg.setGroup("Boson Cursor");
		QString name = cfg.readEntry("Name", i18n("Unknown"));
		d->mCursorTheme->insertItem(name);
		d->mCursorThemes.append(list[i]);
	}
 }
 connect(d->mCursorTheme, SIGNAL(activated(int)),
		this, SLOT(slotCursorThemeChanged(int)));
 

 setCursor(CursorSprite);
}

void OptionsDialog::initPathfindingPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Pathfinding"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Group Movement"), hbox);
 d->mGroupMove = new QComboBox(hbox);
 d->mGroupMove->insertItem(i18n("Old Style (All units move to same position)"), GroupMoveOld);
// d->mGroupMove->insertItem(i18n("Experimental follow-style (units follow leader)"), GroupMoveFollow);
 d->mGroupMove->insertItem(i18n("New style (much better, but not fully working yet)"), GroupMoveNew);
 connect(d->mGroupMove, SIGNAL(activated(int)),
		this, SIGNAL(signalGroupMoveChanged(int)));

 setGroupMove(boConfig->readGroupMoveMode());
}

void OptionsDialog::initScrollingPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Scrolling"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Enable Right Mouse Button Scrolling"), hbox);
 d->mRMBScrolling = new QCheckBox(hbox);
 connect(d->mRMBScrolling, SIGNAL(toggled(bool)), this, SLOT(slotRMBScrollingToggled(bool)));
 
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Enable Middle Mouse Button Scrolling"), hbox);
 d->mMMBScrolling = new QCheckBox(hbox);
 connect(d->mMMBScrolling, SIGNAL(toggled(bool)), this, SLOT(slotMMBScrollingToggled(bool)));
 
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Sensity of cursor at edge of the windo Scrolling (0 for disabled)"), hbox);
 d->mCursorEdgeSensity = new KIntNumInput(hbox);
 d->mCursorEdgeSensity->setRange(0, 50);
 connect(d->mCursorEdgeSensity, SIGNAL(valueChanged(int)), this, SLOT(slotCursorEdgeSensityChanged(int)));

}

void OptionsDialog::slotSpeedChanged(int value)
{
 emit signalSpeedChanged(value);
}

void OptionsDialog::setGameSpeed(int speed)
{
 d->mGameSpeed->setValue(speed);
}

void OptionsDialog::setArrowScrollSpeed(int value)
{
 d->mArrowSpeed->setValue(value);
}

void OptionsDialog::setCursor(CursorMode mode)
{
 d->mCursor->setCurrentItem(mode);
}

void OptionsDialog::setGroupMove(GroupMoveMode mode)
{
 d->mGroupMove->setCurrentItem(mode);
}

void OptionsDialog::setMiniMapScale(double scale)
{
 d->mMiniMapScale->setValue(scale);
}

void OptionsDialog::slotCursorChanged(int index)
{
 if (!d->mCursorTheme) {
	return;
 }
 if (index < 0) {
	return;
 }
 if (d->mCursorTheme->currentItem() >= 0) {
	emit signalCursorChanged(index, d->mCursorThemes[d->mCursorTheme->currentItem()]);
 } else {
	emit signalCursorChanged(index, BosonCursor::defaultTheme());
 }
}

void OptionsDialog::slotCursorThemeChanged(int index)
{
 if (d->mCursor) {
	return;
 }
 if (index < 0) {
	return;
 }
 if (d->mCursor->currentItem() < 0) {
	return;
 }

 emit signalCursorChanged(d->mCursor->currentItem(), d->mCursorThemes[index]);
}

void OptionsDialog::slotCmdBackgroundChanged(int index)
{
 QString file;
 if (index <= 0) {
	emit signalCmdBackgroundChanged(file);
	return;
 }
 index--;
 emit signalCmdBackgroundChanged(d->mCmdBackgrounds[index]);
}

void OptionsDialog::slotRMBScrollingToggled(bool on)
{
 boConfig->setRMBMove(on);
}

void OptionsDialog::slotMMBScrollingToggled(bool on)
{
 boConfig->setMMBMove(on);
}

void OptionsDialog::slotCursorEdgeSensityChanged(int v)
{
 if (v < 0) {
	v = 0;
 }
 boConfig->setCursorEdgeSensity(v);
}

void OptionsDialog::setRMBScrolling(bool on)
{
 d->mRMBScrolling->setChecked(on);
}

void OptionsDialog::setMMBScrolling(bool on)
{
 d->mMMBScrolling->setChecked(on);
}

void OptionsDialog::setCursorEdgeSensity(int v)
{
 d->mCursorEdgeSensity->setValue(v);
}

