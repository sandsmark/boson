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
#include "optionswidgets.h"
#include "bosonconfig.h"

#include "bosoncursor.h"
#include "boson.h"
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

#include "optionswidgets.moc"

OptionsWidget::OptionsWidget()
{
 mGame = 0;
 mPlayer = 0;
}

OptionsWidget::~OptionsWidget()
{
}

GeneralOptions::GeneralOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 mGameSpeed = new KIntNumInput(DEFAULT_GAME_SPEED, this);
 mGameSpeed->setRange(MIN_GAME_SPEED, MAX_GAME_SPEED);
 mGameSpeed->setLabel(i18n("Game speed"));

 mMiniMapScale = new KDoubleNumInput(DEFAULT_MINIMAP_SCALE, this);
 mMiniMapScale->setRange(1.0, 5.0, 1);
 mMiniMapScale->setLabel(i18n("Mini Map scale factor"));


 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Command frame background pixmap"), hbox);
 mCmdBackground = new QComboBox(hbox);
 mCmdBackgrounds = KGlobal::dirs()->findAllResources("data", "boson/themes/ui/*/cmdpanel*.png");
 mCmdBackground->insertItem(i18n("None"));
 //TODO: display filename only... - not the complete path
 mCmdBackground->insertStringList(mCmdBackgrounds);
}

GeneralOptions::~GeneralOptions()
{
}

void GeneralOptions::apply()
{
 if (!game()) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (mGameSpeed->value() != game()->gameSpeed()) {
	game()->slotSetGameSpeed(mGameSpeed->value());
 }
 emit signalMiniMapScaleChanged(mMiniMapScale->value());
 QString file;
 if (mCmdBackground->currentItem() > 0) {
	file = mCmdBackgrounds[mCmdBackground->currentItem() - 1];
 }
 emit signalCmdBackgroundChanged(file);
}

void GeneralOptions::load()
{
 if (!game()) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 setGameSpeed(game()->gameSpeed());
 setMiniMapScale(boConfig->miniMapScale());
 // TODO: cmdbackground
}

void GeneralOptions::setDefaults()
{
 setGameSpeed(DEFAULT_GAME_SPEED);
 setMiniMapScale(DEFAULT_MINIMAP_SCALE);
 setCmdBackground(QString::null);
}

void GeneralOptions::setGameSpeed(int ms)
{
 mGameSpeed->setValue(ms);
}

void GeneralOptions::setMiniMapScale(double scale)
{
 mMiniMapScale->setValue(scale);
}

void GeneralOptions::setCmdBackground(const QString& file)
{
 if (file.isEmpty() || mCmdBackgrounds.findIndex(file) < 0) {
	mCmdBackground->setCurrentItem(0);
 } else {
	int index = mCmdBackgrounds.findIndex(file);
	mCmdBackground->setCurrentItem(index + 1);
 }
}


CursorOptions::CursorOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor"), hbox);
 mCursor = new QComboBox(hbox);
 mCursor->insertItem(i18n("Sprite Cursor"), CursorSprite);
 mCursor->insertItem(i18n("B/W Cursor"), CursorNormal); // AB do we still support it?
 mCursor->insertItem(i18n("KDE Standard Cursor"), CursorKDE);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor theme"), hbox);
 mCursorTheme = new QComboBox(hbox);
 QStringList list = BosonCursor::availableThemes();
 for (int i = 0; i < (int)list.count(); i++) {
	KSimpleConfig cfg(list[i] + QString::fromLatin1("/index.desktop"));
	if (!cfg.hasGroup("Boson Cursor")) {
		kdWarning() << "invalid cursor " << list[i] << endl;
	} else {
		cfg.setGroup("Boson Cursor");
		QString name = cfg.readEntry("Name", i18n("Unknown"));
		mCursorTheme->insertItem(name);
		mCursorThemes.append(list[i]);
	}
 }
}

CursorOptions::~CursorOptions()
{
}

void CursorOptions::apply()
{
 int mode = mCursor->currentItem();
 if (mode < 0) {
	kdWarning() << k_funcinfo << "Invalid cursor mode " << mode << endl;
	return;
 }
 QString theme;
 if (mCursorTheme->currentItem() >= 0) {
	theme = mCursorThemes[mCursorTheme->currentItem()];
 } else {
	theme = BosonCursor::defaultTheme();
 }
 emit signalCursorChanged(mode, theme);
}

void CursorOptions::setDefaults()
{
 setCursor(DEFAULT_CURSOR);
 mCursorTheme->setCurrentItem(0);
}

void CursorOptions::load()
{
#warning TODO
}

void CursorOptions::setCursor(CursorMode mode)
{
 mCursor->setCurrentItem(mode);
}


ScrollingOptions::ScrollingOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Enable right mouse button scrolling"), hbox);
 mRMBScrolling = new QCheckBox(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Enable middle mouse button scrolling"), hbox);
 mMMBScrolling = new QCheckBox(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Sensity of cursor at edge of the window scrolling (0 for disabled)"), hbox);
 mCursorEdgeSensity = new KIntNumInput(hbox);
 mCursorEdgeSensity->setRange(0, 50);

 mArrowSpeed = new KIntNumInput(DEFAULT_ARROW_SCROLL_SPEED, this);
 mArrowSpeed->setRange(1, 200);
 mArrowSpeed->setLabel(i18n("Arrow key steps"));

}

ScrollingOptions::~ScrollingOptions()
{
}

void ScrollingOptions::apply()
{
 boConfig->setRMBMove(mRMBScrolling->isChecked());
 boConfig->setMMBMove(mMMBScrolling->isChecked());
 if (mCursorEdgeSensity->value() < 0) {
	mCursorEdgeSensity->setValue(0);
 }
 boConfig->setCursorEdgeSensity(mCursorEdgeSensity->value());
 if (mArrowSpeed->value() < 0) {
	mArrowSpeed->setValue(0);
 }
 boConfig->setArrowKeyStep(mArrowSpeed->value());
}

void ScrollingOptions::setDefaults()
{
 setArrowScrollSpeed(DEFAULT_ARROW_SCROLL_SPEED);
 setCursorEdgeSensity(DEFAULT_CURSOR_EDGE_SENSITY);
 setRMBScrolling(DEFAULT_USE_RMB_MOVE);
 setMMBScrolling(DEFAULT_USE_MMB_MOVE);
}

void ScrollingOptions::load()
{
 setArrowScrollSpeed(boConfig->arrowKeyStep());
 setCursorEdgeSensity(boConfig->cursorEdgeSensity());
 setRMBScrolling(boConfig->rmbMove());
 setMMBScrolling(boConfig->mmbMove());
}

void ScrollingOptions::setCursorEdgeSensity(int s)
{
 mCursorEdgeSensity->setValue(s);
}

void ScrollingOptions::setArrowScrollSpeed(int s)
{
 mArrowSpeed->setValue(s);
}

void ScrollingOptions::setRMBScrolling(bool on)
{
 mRMBScrolling->setChecked(on);
}

void ScrollingOptions::setMMBScrolling(bool on)
{
 mMMBScrolling->setChecked(on);
}



SoundOptions::SoundOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 (void)new QLabel(i18n("Disable the following unit sounds (please send a bug report if you can think of more descriptive names):"), this);
 QCheckBox* c;
 c = new QCheckBox(i18n("Shoot"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundShoot);
 c = new QCheckBox(i18n("Order Move"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundOrderMove);
 c = new QCheckBox(i18n("Order Attack"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundOrderAttack);
 c = new QCheckBox(i18n("Order Select"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundOrderSelect);
 c = new QCheckBox(i18n("Report Produced"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundReportProduced);
 c = new QCheckBox(i18n("Report Destroyed"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundReportDestroyed);
 c = new QCheckBox(i18n("Report Under Attack"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundReportUnderAttack);
}

SoundOptions::~SoundOptions()
{
}

void SoundOptions::apply()
{
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	boConfig->setUnitSoundActivated(it.data(), !it.key()->isChecked());
 }
}

void SoundOptions::setDefaults()
{
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	it.key()->setChecked(false);
 }
}

void SoundOptions::load()
{
 setUnitSoundsDeactivated(boConfig);
}

void SoundOptions::setUnitSoundsDeactivated(BosonConfig* conf)
{
 if (!conf) {
	return;
 }
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	it.key()->setChecked(!boConfig->unitSoundActivated(it.data()));
 }
}



OpenGLOptions::OpenGLOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 mUpdateInterval = new KIntNumInput(DEFAULT_UPDATE_INTERVAL, this);
 mUpdateInterval->setRange(2, 400);
 mUpdateInterval->setLabel(i18n("Update interval (low values hurt performance)"));
}

OpenGLOptions::~OpenGLOptions()
{
}

void OpenGLOptions::apply()
{
 emit signalUpdateIntervalChanged((unsigned int)mUpdateInterval->value());
}

void OpenGLOptions::setDefaults()
{
 setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
}

void OpenGLOptions::load()
{
 setUpdateInterval(boConfig->updateInterval());
}

void OpenGLOptions::setUpdateInterval(int ms)
{
 kdDebug() << k_funcinfo << ms << endl;
 mUpdateInterval->setValue(ms);
}

ChatOptions::ChatOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);
 mScreenRemoveTime = new KIntNumInput(DEFAULT_CHAT_SCREEN_REMOVE_TIME, this);
 mScreenRemoveTime->setRange(0, 400);
 mScreenRemoveTime->setLabel(i18n("Remove from screen after seconds (0 to remove never)"));

 hbox = new QHBox(this);
 mScreenMaxItems = new KIntNumInput(DEFAULT_CHAT_SCREEN_REMOVE_TIME, this);
 mScreenMaxItems->setRange(-1, 40);
 mScreenMaxItems->setLabel(i18n("Maximal items on the screen (-1 is unlimited)"));
}

ChatOptions::~ChatOptions()
{
}

void ChatOptions::apply()
{
 boConfig->setChatScreenRemoveTime(mScreenRemoveTime->value());
 boConfig->setChatScreenMaxItems(mScreenMaxItems->value());
}

void ChatOptions::setDefaults()
{
 setScreenRemoveTime(DEFAULT_CHAT_SCREEN_REMOVE_TIME);
 setScreenMaxItems(DEFAULT_CHAT_SCREEN_REMOVE_TIME);
}

void ChatOptions::load()
{
 setScreenRemoveTime(boConfig->chatScreenRemoveTime());
 setScreenMaxItems(boConfig->chatScreenMaxItems());
}

void ChatOptions::setScreenRemoveTime(unsigned int s)
{
 mScreenRemoveTime->setValue(s);
}

void ChatOptions::setScreenMaxItems(int m)
{
 mScreenMaxItems->setValue(m);
}

