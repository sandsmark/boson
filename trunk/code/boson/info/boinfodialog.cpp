/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann <b_mann@gmx.de>

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

#include "boinfodialog.h"
#include "boinfodialog.moc"

#include "../../bomemory/bodummymemory.h"
#include "boinfo.h"
#include "../boglobal.h"
#include "../bofiledialog.h"
#include "bodebug.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klistview.h>

#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qmap.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qpushbutton.h>

class BoInfoDialogPrivate
{
public:
	BoInfoDialogPrivate()
	{
		mInfo = 0;

		mBosonVersionString = 0;
		mBosonVersionCode = 0;

		mCompileQtVersionString = 0;
		mCompileQtVersionCode = 0;
		mRuntimeQtVersionString = 0;

		mCompileKDEVersionString = 0;
		mCompileKDEVersionCode = 0;
		mRuntimeKDEVersionString = 0;
		mRuntimeKDEVersionCode = 0;

		mOGHaveData = 0;
		mOGRuntimeVersionString = 0;
		mOGRuntimeVersionCode = 0;
		mOGVendorString = 0;
		mOGRendererString = 0;
		mOGExtensions = 0;

		mGLURuntimeVersionString = 0;
		mGLUExtensions = 0;

		mGLXClientVersionString = 0;
		mGLXClientVendorString = 0;
		mGLXClientExtensions = 0;
		mGLXVersionMajor = 0;
		mGLXVersionMinor = 0;
		mGLXServerVersionString = 0;
		mGLXServerVendorString = 0;
		mGLXServerExtensions = 0;


		mXHaveData = 0;
		mXDisplayName = 0;
		mXProtocolVersion = 0;
		mXProtocolRevision = 0;
		mXVendorString = 0;
		mXVendorReleaseNumber = 0;
		mXExtensions = 0;
		mXDefaultScreen = 0;
		mXScreenCount = 0;
		mXScreen = 0;
		mXScreenWidth = 0;
		mXScreenHeight = 0;
		mXScreenWidthMM = 0;
		mXScreenHeightMM = 0;

#if 0
		mNVidiaErrors = 0;
#endif

		mHaveLibs = 0;

		mOSType = 0;
		mOSVersion = 0;
		mOSKernelModuleTDFX = 0;
		mOSKernelModuleNVidia = 0;
		mOSCPUSpeed = 0;
		mOSHaveMTRR = 0;

		mCompleteData = 0;

		mCurrentFile = 0;
	}
	const BoInfo* data() { return mInfo ? mInfo : BoInfo::boInfo(); }

	BoInfo* mInfo;

	QLabel* mBosonVersionString;
	QLabel* mBosonVersionCode;

	QLabel* mCompileQtVersionString;
	QLabel* mCompileQtVersionCode;
	QLabel* mRuntimeQtVersionString;

	QLabel* mCompileKDEVersionString;
	QLabel* mCompileKDEVersionCode;
	QLabel* mRuntimeKDEVersionString;
	QLabel* mRuntimeKDEVersionCode;

	QLabel* mOGHaveData;
	QLabel* mOGRuntimeVersionString;
	QLabel* mOGRuntimeVersionCode;
	QLabel* mOGVendorString;
	QLabel* mOGRendererString;
	KListBox* mOGExtensions;

	QLabel* mGLURuntimeVersionString;
	KListBox* mGLUExtensions;

	QLabel* mGLXClientVersionString;
	QLabel* mGLXClientVendorString;
	KListBox* mGLXClientExtensions;
	QLabel* mGLXVersionMajor;
	QLabel* mGLXVersionMinor;
	QLabel* mGLXServerVersionString;
	QLabel* mGLXServerVendorString;
	KListBox* mGLXServerExtensions;

	QLabel* mIsDirect;

	QLabel* mXHaveData;
	QLabel* mXDisplayName;
	QLabel* mXProtocolVersion;
	QLabel* mXProtocolRevision;
	QLabel* mXVendorString;
	QLabel* mXVendorReleaseNumber;
	KListBox* mXExtensions;
	QLabel* mXDefaultScreen;
	QLabel* mXScreenCount;
	QLabel* mXScreen;
	QLabel* mXScreenWidth;
	QLabel* mXScreenHeight;
	QLabel* mXScreenWidthMM;
	QLabel* mXScreenHeightMM;

#if 0
	KListBox* mNVidiaErrors;
#endif

	KListView* mHaveLibs;
	QValueList<int> mHaveLibsList;

	QLabel* mOSType;
	QLabel* mOSVersion;
	QLabel* mOSKernelModuleTDFX;
	QLabel* mOSKernelModuleNVidia;
	QLabel* mOSCPUSpeed;
	QLabel* mOSHaveMTRR;

	KListView* mCompleteData;

	QLabel* mCurrentFile;
};

BoInfoDialog::BoInfoDialog(QWidget* parent, bool modal)
		: KDialogBase(Tabbed, i18n("BoInfo"), Ok, Ok, parent,
		"boinfodialog", modal, true)
{
 d = new BoInfoDialogPrivate;
 if (!BoGlobal::boGlobal()) {
	BO_NULL_ERROR(BoGlobal::boGlobal());
	// don't return
 } else if (!BoGlobal::boGlobal()->boInfo()) {
	BO_NULL_ERROR(BoGlobal::boGlobal()->boInfo());
	// don't return
 }
 initBosonPage();
 initQtPage();
 initKDEPage();
 initOpenGLPage();
 initXPage();
 initNVidiaPage();
 initOSPage();
 initLibsPage();
 initCompleteDataPage();
 initFilePage();
}

BoInfoDialog::~BoInfoDialog()
{
 delete d->mInfo;
 delete d;
}

void BoInfoDialog::initBosonPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Boson"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Boson version string: "), hbox);
 d->mBosonVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Boson version code: "), hbox);
 d->mBosonVersionCode = new QLabel(hbox);
}

void BoInfoDialog::initQtPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Qt"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Runtime Qt version string: "), hbox);
 d->mRuntimeQtVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Compile time Qt version string: "), hbox);
 d->mCompileQtVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Compile time Qt version code: "), hbox);
 d->mCompileQtVersionCode = new QLabel(hbox);
}

void BoInfoDialog::initKDEPage()
{
 QVBox* vbox = addVBoxPage(i18n("&KDE"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Runtime KDE version string (valid for KDE >= 3.1 only): "), hbox);
 d->mRuntimeKDEVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Compile time KDE version string: "), hbox);
 d->mCompileKDEVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Compile time KDE version code: "), hbox);
 d->mCompileKDEVersionCode = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Runtime KDE version code (not yet valid): "), hbox);
 d->mRuntimeKDEVersionCode = new QLabel(hbox);
}

void BoInfoDialog::initOpenGLPage()
{
 QVBox* vbox = addVBoxPage(i18n("&OpenGL"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Have OpenGL data:"), hbox);
 d->mOGHaveData = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Runtime OpenGL version string:"), hbox);
 d->mOGRuntimeVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Runtime OpenGL version code:"), hbox);
 d->mOGRuntimeVersionCode = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("OpenGL vendor string:"), hbox);
 d->mOGVendorString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("OpenGL renderer string:"), hbox);
 d->mOGRendererString = new QLabel(hbox);
 // TODO OG extensions

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Runtime GLU version string:"), hbox);
 d->mGLURuntimeVersionString = new QLabel(hbox);
 // TODO: GLU extensions


 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Client GLX version string:"), hbox);
 d->mGLXClientVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Client GLX vendor string:"), hbox);
 d->mGLXClientVendorString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("GLX version major:"), hbox);
 d->mGLXVersionMajor = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("GLX version minor:"), hbox);
 d->mGLXVersionMinor = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Server GLX version string:"), hbox);
 d->mGLXServerVersionString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Server GLX vendor string:"), hbox);
 d->mGLXServerVendorString = new QLabel(hbox);
 // TODO: GLX client and server extensions

 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Is Direct:"), hbox);
 d->mIsDirect = new QLabel(hbox);
}

void BoInfoDialog::initXPage()
{
 QVBox* vbox = addVBoxPage(i18n("&X"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Have X data:"), hbox);
 d->mXHaveData = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XDisplayName)), hbox);
 d->mXDisplayName = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XProtocolVersion)), hbox);
 d->mXProtocolVersion = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XProtocolRevision)), hbox);
 d->mXProtocolRevision = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XVendorString)), hbox);
 d->mXVendorString = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XVendorReleaseNumber)), hbox);
 d->mXVendorReleaseNumber = new QLabel(hbox);

 // screen information
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XDefaultScreen)), hbox);
 d->mXDefaultScreen = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XScreenCount)), hbox);
 d->mXScreenCount = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XScreen)), hbox);
 d->mXScreen = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XScreenWidth)), hbox);
 d->mXScreenWidth = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XScreenHeight)), hbox);
 d->mXScreenHeight = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XScreenWidthMM)), hbox);
 d->mXScreenWidthMM = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n(BoInfo::keyToName(BoInfo::XScreenHeightMM)), hbox);
 d->mXScreenHeightMM = new QLabel(hbox);
 // screen information (end)

}

void BoInfoDialog::initNVidiaPage()
{
 // AB: NVidia page has been disabled for now.
 //     it's contents are based on how the NVidia driver looked like a long time
 //     ago - much of it is not valid anymore.
#if 0
 QVBox* vbox = addVBoxPage(i18n("&NVidia"));
 (void)new QLabel(i18n("This page is relevant for users of the proprietary NVidia driver only. All errors listed here are actually GOOD for all other people."), vbox);
 d->mNVidiaErrors = new KListBox(vbox);
#endif
}

void BoInfoDialog::initOSPage()
{
 QVBox* vbox = addVBoxPage(i18n("Operating &System"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Recognized Operating system type:"), hbox);
 d->mOSType = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Operating system version:"), hbox);
 d->mOSVersion = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Kernel Module TDFX (3dfx voodoo):"), hbox);
 d->mOSKernelModuleTDFX = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Kernel Module NVidia:"), hbox);
 d->mOSKernelModuleNVidia= new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("CPU Speed:"), hbox);
 d->mOSCPUSpeed = new QLabel(hbox);
 hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Have MTRR:"), hbox);
 d->mOSHaveMTRR = new QLabel(hbox);
}

void BoInfoDialog::initLibsPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Libs"));
 (void)new QLabel(i18n("This page is a side product of the NVidia page. Some data may be relevant if you have a problem."), vbox);
 d->mHaveLibs = new KListView(vbox);
 d->mHaveLibs->addColumn(i18n("Key"));
 d->mHaveLibs->addColumn(i18n("Name"));
 d->mHaveLibs->addColumn(i18n("Value"));

 d->mHaveLibsList.append(BoInfo::HaveXExtLibGLX_a);
 d->mHaveLibsList.append(BoInfo::HaveXExtLibGLCore_a);
 d->mHaveLibsList.append(BoInfo::HaveXExtLibGLX_so);
 d->mHaveLibsList.append(BoInfo::HaveLibGL_so);
 d->mHaveLibsList.append(BoInfo::HaveLibGLCore_so_1);
 d->mHaveLibsList.append(BoInfo::HaveProprietaryNVidiaXDriver);
}

void BoInfoDialog::initCompleteDataPage()
{
 QVBox* vbox = addVBoxPage(i18n("&Complete Data"));
 (void)new QLabel(i18n("All BoInfo data"), vbox);
 d->mCompleteData = new KListView(vbox);
 d->mCompleteData->addColumn(i18n("Key"));
 d->mCompleteData->addColumn(i18n("Name"));
 d->mCompleteData->addColumn(i18n("Value"));
}

void BoInfoDialog::initFilePage()
{
 QVBox* vbox = addVBoxPage(i18n("&Files"));
 QHBox* hbox = new QHBox(vbox);
 (void)new QLabel(i18n("Current file:"), hbox);
 d->mCurrentFile = new QLabel(i18n("None"), hbox);

 QPushButton* load = new QPushButton(i18n("Load from &file"), vbox);
 connect(load, SIGNAL(clicked()), this, SLOT(slotLoadFromFile()));

 QPushButton* save = new QPushButton(i18n("&Save to file"), vbox);
 connect(save, SIGNAL(clicked()), this, SLOT(slotSaveToFile()));
}

void BoInfoDialog::reset()
{
 resetBosonPage();
 resetQtPage();
 resetKDEPage();
 resetOpenGLPage();
 resetXPage();
 resetNVidiaPage();
 resetOSPage();
 resetLibsPage();
 resetCompleteDataPage();
 resetFilePage();
}

void BoInfoDialog::resetBosonPage()
{
 d->mBosonVersionString->setText(d->data()->bosonVersionString());
 d->mBosonVersionCode->setText("0x" + QString::number(d->data()->bosonVersion(), 16));
}

void BoInfoDialog::resetQtPage()
{
 d->mCompileQtVersionString->setText(d->data()->qtVersionString(false));
 d->mRuntimeQtVersionString->setText(d->data()->qtVersionString(true));
 d->mCompileQtVersionCode->setText("0x" + QString::number(d->data()->qtVersion(), 16));
}

void BoInfoDialog::resetKDEPage()
{
 d->mCompileKDEVersionString->setText(d->data()->kdeVersionString(false));
 d->mRuntimeKDEVersionString->setText(d->data()->kdeVersionString(true));
 d->mCompileKDEVersionCode->setText("0x" + QString::number(d->data()->kdeVersion(false), 16));
 d->mRuntimeKDEVersionCode->setText("0x" + QString::number(d->data()->kdeVersion(true), 16));
}

void BoInfoDialog::resetOpenGLPage()
{
 bool og = d->data()->haveOpenGLData();
 d->mOGHaveData->setText(og ? i18n("Yes") : i18n("No"));
 d->mOGRuntimeVersionString->setText(og ? d->data()->gl()->openGLVersionString() : QString(""));
 d->mOGRuntimeVersionCode->setText("0x" + QString::number(d->data()->gl()->openGLVersion(), 16));
 d->mOGVendorString->setText(og ? d->data()->gl()->openGLVendorString() : QString(""));
 d->mOGRendererString->setText(og ? d->data()->gl()->openGLRendererString() : QString(""));
 // TODO: OG extensions

 d->mGLURuntimeVersionString->setText(og ? d->data()->gl()->gluVersionString() : QString(""));
 // TODO: GLU extensions

 d->mGLXClientVersionString->setText(og ? d->data()->gl()->glXClientVersionString() : QString(""));
 d->mGLXClientVendorString->setText(og ? d->data()->gl()->glXClientVendorString() : QString(""));
 int major, minor;
 d->data()->gl()->glXVersion(&major, &minor);
 d->mGLXVersionMajor->setText(og ? QString::number(major) : QString(""));
 d->mGLXVersionMinor->setText(og ? QString::number(minor) : QString(""));
 d->mGLXServerVersionString->setText(og ? d->data()->gl()->glXServerVersionString() : QString(""));
 d->mGLXServerVendorString->setText(og ? d->data()->gl()->glXServerVendorString() : QString(""));
 // TODO: GLX client/server extensions

 d->mIsDirect->setText(og ? (d->data()->gl()->isDirect() ? i18n("Yes") : i18n("No")) : QString(""));
}

void BoInfoDialog::resetXPage()
{
 bool x = d->data()->haveXData();
 d->mXHaveData->setText(x ? i18n("Yes") : i18n("No"));
 d->mXDisplayName->setText(x ? d->data()->xDisplayName() : QString(""));
 d->mXProtocolVersion->setText(x ? QString::number(d->data()->xProtocolVersion()) : QString(""));
 d->mXProtocolRevision->setText(x ? QString::number(d->data()->xProtocolRevision()) : QString(""));
 d->mXVendorString->setText(x ? d->data()->xVendorString() : QString(""));
 d->mXVendorReleaseNumber->setText(x ? QString::number(d->data()->xVendorReleaseNumber()) : QString(""));
 d->mXDefaultScreen->setText(x ? QString::number(d->data()->xDefaultScreen()) : QString(""));
 d->mXScreenCount->setText(x ? QString::number(d->data()->xScreenCount()) : QString(""));
 d->mXScreen->setText(x ? QString::number(d->data()->xScreen()) : QString(""));
 d->mXScreenWidth->setText(x ? QString::number(d->data()->xScreenWidth()) : QString(""));
 d->mXScreenHeight->setText(x ? QString::number(d->data()->xScreenHeight()) : QString(""));
 d->mXScreenWidthMM->setText(x ? QString::number(d->data()->xScreenWidthMM()) : QString(""));
 d->mXScreenHeightMM->setText(x ? QString::number(d->data()->xScreenHeightMM()) : QString(""));

 // TODO: X extensions
}

void BoInfoDialog::resetNVidiaPage()
{
#if 0
 d->mNVidiaErrors->clear();
 QStringList list = d->data()->checkProprietaryNVidiaDriver();
 if (list.count() == 0) {
	list.append(i18n("No errors found. It seems your proprietary NVidia driver has been installed correctly."));
 }
 d->mNVidiaErrors->insertStringList(list);
#endif
}

void BoInfoDialog::resetOSPage()
{
 d->mOSType->setText(d->data()->osType());
 d->mOSVersion->setText(d->data()->osVersion());
 if (d->data()->osKernelModuleTDFXString().isEmpty()) {
	d->mOSKernelModuleTDFX->setText(i18n("(not loaded)"));
 } else {
	d->mOSKernelModuleTDFX->setText(d->data()->osKernelModuleTDFXString());
 }
 if (d->data()->osKernelModuleNVidiaString().isEmpty()) {
	d->mOSKernelModuleNVidia->setText(i18n("(not loaded)"));
 } else {
	d->mOSKernelModuleNVidia->setText(d->data()->osKernelModuleNVidiaString());
 }
 d->mOSCPUSpeed->setText(QString::number(d->data()->cpuSpeed()));
 d->mOSHaveMTRR->setText(d->data()->haveMtrr() ? i18n("Yes") : i18n("No"));
}

void BoInfoDialog::resetLibsPage()
{
 d->mHaveLibs->clear();
 QValueList<int>::Iterator it;
 for (it = d->mHaveLibsList.begin(); it != d->mHaveLibsList.end(); ++it) {
	QListViewItem* item = new QListViewItem(d->mHaveLibs);
	item->setText(0, QString::number(*it));
	item->setText(1, BoInfo::keyToName(*it));
	if (!d->data()->contains(*it)) {
		item->setText(2, i18n("(Unknown)"));
	} else if (d->data()->getString(*it).isEmpty()) {
		item->setText(2, i18n("Not found"));
	} else {
		item->setText(2, d->data()->getString(*it));
	}
 }

 // manually add the items that can't be added in the list (as they are no
 // strings)
 QListViewItem* dependsOnGLCore = new QListViewItem(d->mHaveLibs);
 dependsOnGLCore->setText(0, QString::number(BoInfo::LibGL_so_DependsOnLibGLCore));
 dependsOnGLCore->setText(1, BoInfo::keyToName(BoInfo::LibGL_so_DependsOnLibGLCore));
 if (!d->data()->contains(BoInfo::LibGL_so_DependsOnLibGLCore)) {
	dependsOnGLCore->setText(2, i18n("(Unknown)"));
 } else if (d->data()->getBool(BoInfo::LibGL_so_DependsOnLibGLCore)) {
	dependsOnGLCore->setText(2, i18n("Yes"));
 } else {
	dependsOnGLCore->setText(2, i18n("No"));
 }
}

void BoInfoDialog::resetCompleteDataPage()
{
 d->mCompleteData->clear();
 QMap<int, QVariant> data = d->data()->completeData();
 QMap<int, QVariant>::Iterator it = data.begin();
 for (; it != data.end(); ++it) {
	// FIXME: values on multiple lines are broken
	QListViewItem* item = new QListViewItem(d->mCompleteData);
	item->setMultiLinesEnabled(true);
	item->setText(0, QString::number(it.key()));
	item->setText(1, d->data()->keyToName(it.key()));
	item->setText(2, d->data()->valueToString(it.key()));
 }
}

void BoInfoDialog::resetFilePage()
{
}

void BoInfoDialog::loadFromFile(const QString& file)
{
 delete d->mInfo;
 d->mInfo = new BoInfo();
 d->mInfo->loadFromFile(file);
 reset();
 d->mCurrentFile->setText(file);
}

void BoInfoDialog::slotSaveToFile()
{
 QString file;
 file = BoFileDialog::getSaveFileName();
 if (file.isNull()) {
	return;
 }
 QFileInfo fileInfo(file);
 if (fileInfo.exists()) {
	int ok = KMessageBox::questionYesNoCancel(this,
			i18n("The file %1 does already exists. Do you want to overwrite it?").arg(file));
	if (ok != KMessageBox::Yes) {
		return;
	}
 }
 if (!d->data()->saveToFile(file)) {
	KMessageBox::sorry(this, i18n("Unable to save to file %1").arg(file));
 }
}

void BoInfoDialog::slotLoadFromFile()
{
 QString file;
 file = BoFileDialog::getOpenFileName();
 if (file.isNull()) {
	return;
 }
 loadFromFile(file);
}


