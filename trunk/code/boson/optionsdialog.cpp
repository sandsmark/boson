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
		mMiniMapScale = 0;
		
		mCursor = 0;
		mCursorTheme = 0;

		mRMBScrolling = 0;
		mMMBScrolling = 0;
		mCursorEdgeSensity = 0;
	}

	KIntNumInput* mArrowSpeed;
	KIntNumInput* mGameSpeed;
	KIntNumInput* mUpdateInterval;
	QComboBox* mCmdBackground;
	KDoubleNumInput* mMiniMapScale;
	
	QComboBox* mCursor;
	QComboBox* mCursorTheme;
	QStringList mCursorThemes;
	QStringList mCmdBackgrounds;

	QCheckBox* mRMBScrolling;
	QCheckBox* mMMBScrolling;
	KIntNumInput* mCursorEdgeSensity;

	QMap<QCheckBox*, UnitSoundEvent> mCheckBox2UnitSoundEvent;
};

OptionsDialog::OptionsDialog(QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("Boson Options"), Ok|Default,
		Cancel, parent, "bosonoptionsdialog", modal, true)
{
 d = new OptionsDialogPrivate;

 initGeneralPage();
 initCursorPage();
 initScrollingPage();
 initSoundsPage();

 connect(this, SIGNAL(defaultClicked()), this, SLOT(slotSetDefaults()));
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
 d->mArrowSpeed->setLabel(i18n("Arrow key steps"));
 connect(d->mArrowSpeed, SIGNAL(valueChanged(int)),
		this, SIGNAL(signalArrowScrollChanged(int)));

 d->mGameSpeed = new KIntNumInput(10, vbox);
 d->mGameSpeed->setRange(MIN_GAME_SPEED, MAX_GAME_SPEED);
 d->mGameSpeed->setLabel(i18n("Game speed"));
 connect(d->mGameSpeed, SIGNAL(valueChanged(int)),
		this, SLOT(slotSpeedChanged(int)));

 d->mUpdateInterval = new KIntNumInput(50, vbox);
 d->mUpdateInterval->setRange(2, 400);
 d->mUpdateInterval->setLabel(i18n("Update interval (low values hurt performance)"));
 connect(d->mUpdateInterval, SIGNAL(valueChanged(int)),
		this, SLOT(slotUpdateIntervalChanged(int)));

 QHBox* hbox = new QHBox(vbox);

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Command frame background pixmap"), hbox);
 d->mCmdBackground = new QComboBox(hbox);
 d->mCmdBackgrounds = KGlobal::dirs()->findAllResources("data", "boson/themes/ui/*/cmdpanel*.png");
 d->mCmdBackground->insertItem(i18n("None"));
 //TODO: display filename only... - not the complete path
 d->mCmdBackground->insertStringList(d->mCmdBackgrounds);
 connect(d->mCmdBackground, SIGNAL(activated(int)), 
		this, SLOT(slotCmdBackgroundChanged(int)));

 d->mMiniMapScale = new KDoubleNumInput(1.0, vbox);
 d->mMiniMapScale->setRange(1.0, 5.0, 1);
 d->mMiniMapScale->setLabel(i18n("Mini Map scale factor"));
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
 connect(d->mCursor, SIGNAL(activated(int)),
		this, SLOT(slotCursorChanged(int)));

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Cursor theme"), hbox);
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

void OptionsDialog::initScrollingPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Scrolling"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Enable right mouse button scrolling"), hbox);
 d->mRMBScrolling = new QCheckBox(hbox);
 connect(d->mRMBScrolling, SIGNAL(toggled(bool)), this, SLOT(slotRMBScrollingToggled(bool)));

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Enable middle mouse button scrolling"), hbox);
 d->mMMBScrolling = new QCheckBox(hbox);
 connect(d->mMMBScrolling, SIGNAL(toggled(bool)), this, SLOT(slotMMBScrollingToggled(bool)));

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Sensity of cursor at edge of the window scrolling (0 for disabled)"), hbox);
 d->mCursorEdgeSensity = new KIntNumInput(hbox);
 d->mCursorEdgeSensity->setRange(0, 50);
 connect(d->mCursorEdgeSensity, SIGNAL(valueChanged(int)), this, SLOT(slotCursorEdgeSensityChanged(int)));

}

void OptionsDialog::initSoundsPage()
{
 QVBox* vbox = addVBoxPage(i18n("S&ounds"));
 (void)new QLabel(i18n("Disable the following unit sounds (please send a bug report if you can think of more descriptive names):"), vbox);
 QCheckBox* c;
 c = new QCheckBox(i18n("Shoot"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundShoot);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));
 c = new QCheckBox(i18n("Order Move"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundOrderMove);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));
 c = new QCheckBox(i18n("Order Attack"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundOrderAttack);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));
 c = new QCheckBox(i18n("Order Select"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundOrderSelect);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));
 c = new QCheckBox(i18n("Report Produced"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundReportProduced);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));
 c = new QCheckBox(i18n("Report Destroyed"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundReportDestroyed);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));
 c = new QCheckBox(i18n("Report Under Attack"), vbox);
 d->mCheckBox2UnitSoundEvent.insert(c, SoundReportUnderAttack);
 connect(c, SIGNAL(toggled(bool)), this, SLOT(slotUnitSoundDeactivated(bool)));


}

void OptionsDialog::slotSpeedChanged(int value)
{
 emit signalSpeedChanged(value);
}

void OptionsDialog::slotUpdateIntervalChanged(int value)
{
 emit signalUpdateIntervalChanged((unsigned int)value);
}

void OptionsDialog::setGameSpeed(int speed)
{
 d->mGameSpeed->setValue(speed);
}

void OptionsDialog::setUpdateInterval(int i)
{
kdDebug() << k_funcinfo << i << endl;
 d->mUpdateInterval->setValue(i);
}

void OptionsDialog::setArrowScrollSpeed(int value)
{
 d->mArrowSpeed->setValue(value);
}

void OptionsDialog::setCursor(CursorMode mode)
{
 d->mCursor->setCurrentItem(mode);
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

void OptionsDialog::setUnitSoundsDeactivated(BosonConfig* conf)
{
 if (!conf) {
	return;
 }
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = d->mCheckBox2UnitSoundEvent.begin();
 for (; it != d->mCheckBox2UnitSoundEvent.end(); ++it) {
	it.key()->setChecked(!boConfig->unitSoundActivated(it.data()));
 }
}

void OptionsDialog::slotUnitSoundDeactivated(bool off)
{
 QCheckBox* s = (QCheckBox*)sender(); // a little bit hackiish...
 if (!s) {
	kdWarning() << k_funcinfo << "NULL sender()" << endl;
	return;
 }
 if (!d->mCheckBox2UnitSoundEvent.contains(s)) {
	kdWarning() << k_funcinfo << "Unknown checkbox" << endl;
	return;
 }
 UnitSoundEvent e = d->mCheckBox2UnitSoundEvent[s];
 boConfig->setUnitSoundActivated(e, !off);
}

void OptionsDialog::slotSetDefaults()
{
 // FIXME: these values are copied from BosonConfig
 //  Probably we should have some #define's with default values somewhere
 setGameSpeed(5);
 setArrowScrollSpeed(10);
 setMiniMapScale(2.0);
 setRMBScrolling(true);
 setMMBScrolling(true);
 setCursor(CursorSprite);
 slotCursorChanged((int)CursorSprite);
 setCursorEdgeSensity(20);
 setUpdateInterval(25);
}

