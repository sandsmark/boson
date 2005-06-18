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
#include <qvbox.h>

#include "optionsdialog.moc"

class OptionsDialog::OptionsDialogPrivate
{
public:
	OptionsDialogPrivate()
	{
		mArrowSpeed = 0;
		mGameSpeed = 0;
		mCommandFrame = 0;
		mChat = 0;
		mCmdBackground = 0;
		mGroupMove = 0;
		mMiniMapScale = 0;
		
		mCursor = 0;
		mCursorTheme = 0;
	}

	KIntNumInput* mArrowSpeed;
	KIntNumInput* mGameSpeed;
	QComboBox* mCommandFrame;
	QComboBox* mChat;
	QComboBox* mCmdBackground;
	QComboBox* mGroupMove;
	KDoubleNumInput* mMiniMapScale;
	
	QComboBox* mCursor;
	QComboBox* mCursorTheme;
	QStringList mCursorThemes;
	QStringList mCmdBackgrounds;
};

OptionsDialog::OptionsDialog(QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("Boson Options"), Ok|Cancel|Default,
		Cancel, parent, "bosonoptionsdialog", modal, true)
{
 d = new OptionsDialogPrivate;
 
 initGeneralPage();
 initCursorPage();
 initPathfindingPage();
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
 d->mGameSpeed->setRange(MAX_GAME_SPEED, MIN_GAME_SPEED);
 d->mGameSpeed->setLabel(i18n("Game Speed"));
 connect(d->mGameSpeed, SIGNAL(valueChanged(int)), 
		this, SLOT(slotSpeedChanged(int)));

 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Position of Command Frame"), hbox);
 d->mCommandFrame = new QComboBox(hbox);
 d->mCommandFrame->insertItem(i18n("Left"), CmdFrameLeft);
 d->mCommandFrame->insertItem(i18n("Right"), CmdFrameRight);
 d->mCommandFrame->insertItem(i18n("Undocked"), CmdFrameUndocked);
 connect(d->mCommandFrame, SIGNAL(activated(int)), 
		this, SIGNAL(signalCommandFramePositionChanged(int)));

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Position of Chat Frame"), hbox);
 d->mChat = new QComboBox(hbox);
 d->mChat->insertItem(i18n("Top"), ChatFrameTop);
 d->mChat->insertItem(i18n("Bottom"), ChatFrameBottom);
 connect(d->mChat, SIGNAL(activated(int)), 
		this, SIGNAL(signalChatFramePositionChanged(int)));
 
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


 setCommandFramePosition(CmdFrameLeft);
 setChatFramePosition(ChatFrameBottom);
}

void OptionsDialog::initCursorPage()
{
 QVBox* vbox = addVBoxPage(i18n("C&ursor"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Cursor"), hbox);
 d->mCursor = new QComboBox(hbox);
 d->mCursor->insertItem(i18n("Sprite Cursor"), CursorSprite);
 d->mCursor->insertItem(i18n("Normal (X) Cursor"), CursorNormal);
 d->mCursor->insertItem(i18n("KDE Cursor"), CursorKDE);
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
 d->mGroupMove->insertItem(i18n("Experimental follow-style (units follow leader)"), GroupMoveFollow);
 d->mGroupMove->insertItem(i18n("New style (much better, but not fully working yet)"), GroupMoveNew);
 connect(d->mGroupMove, SIGNAL(activated(int)),
		this, SIGNAL(signalGroupMoveChanged(int)));

 setGroupMove(boConfig->readGroupMoveMode());
}

void OptionsDialog::slotSpeedChanged(int value)
{
// value is not actually the new speed but the value supplied by the input
// (1=very slow).
// the actual speed is the value for the QTimer, where 1 would be 1ms and
// therefore *veery* fast. So lets change the value:
 if (value == 0) {
	kdError() << "cannot use speed=0" << endl;
	return;
 }

 int newSpeed = (MIN_GAME_SPEED + MAX_GAME_SPEED) - value;
 emit signalSpeedChanged(newSpeed);
}

void OptionsDialog::setGameSpeed(int ms)
{
 int value = (MIN_GAME_SPEED + MAX_GAME_SPEED) - ms;
 d->mGameSpeed->setValue(value);
}

void OptionsDialog::setArrowScrollSpeed(int value)
{
 d->mArrowSpeed->setValue(value);
}

void OptionsDialog::setCommandFramePosition(CommandFramePosition position)
{
 d->mCommandFrame->setCurrentItem(position);
}

void OptionsDialog::setChatFramePosition(ChatFramePosition position)
{
 d->mChat->setCurrentItem(position);
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