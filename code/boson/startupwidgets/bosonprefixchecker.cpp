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

#include "bosonprefixchecker.h"
#include "bosonprefixchecker.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kapplication.h>

#include "bodebug.h"


BosonPrefixChecker::BosonPrefixChecker() : QDialog(0, "BosonPrefixChecker", true)
{
  // First try to locate data files using KStandardDirs
  if(locate("data", "boson/pics/boson-startup-bg.png") != QString::null)
  {
    mPrefixFound = true;
    return;
  }

  // If this failed, check for saved prefix
  KConfig* cfg = kapp->config();
  cfg->setGroup("Files");
  if(cfg->hasKey("Data prefix"))
  {
    KGlobal::dirs()->addPrefix(cfg->readEntry("Data prefix"));
    if(locate("data", "boson/pics/boson-startup-logo.png") != QString::null)
    {
      // Correct prefix was loaded
      mPrefixFound = true;
      return;
    }
  }

  // No prefix was found automatically, so we ask user
  // Show prefix choosing dialog

  setCaption(i18n("No data files found!"));
  mPrefixFound = false; // just for case...

  mMainLayout = new QVBoxLayout(this, 20, 6);

  mMessage = new QLabel(this);
  mMessage->setText(i18n("You seem not to have Boson data files installed!<br>"
      "Please install data package of Boson and restart Boson.<br><br>"
      "If you're sure you have data files installed, please enter prefix,"
      "where they we're installed (something like \"/opt/kde\" or \"/usr/local/kde\"), below.\n"
      "If you do not specify prefix, Boson will exit."));
  mMainLayout->addWidget(mMessage);

  mPrefixLayout = new QHBoxLayout(mMainLayout);
  mPrefixLabel = new QLabel(this);
  mPrefixLabel->setText(i18n("Prefix: "));
  mPrefixLayout->addWidget(mPrefixLabel);

  mPrefix = new QLineEdit(this);
  mPrefixLayout->addWidget(mPrefix);

  mPrefixButton = new QPushButton(this);
  mPrefixButton->setText(i18n("Browse..."));
  mPrefixLayout->addWidget(mPrefixButton);

  mOkButton = new QPushButton(this);
  mOkButton->setText(i18n("Ok"));
  mOkButton->setDefault(true);
  mMainLayout->addWidget(mOkButton);

  connect(mPrefixButton, SIGNAL(clicked()), this, SLOT(slotSelectPrefix()));
  connect(mOkButton, SIGNAL(clicked()), this, SLOT(slotOk()));

  exec();
}

BosonPrefixChecker::~BosonPrefixChecker()
{
}

void BosonPrefixChecker::slotSelectPrefix()
{
  QString dir = KFileDialog::getExistingDirectory(QString::null, this, i18n("Please select correct prefix"));
  mPrefix->setText(dir);
}

void BosonPrefixChecker::slotOk()
{
  if(mPrefix->text().isEmpty())
  {
    noPrefixSelected();
    return;
  }
  QFileInfo f(mPrefix->text());
  if(!f.exists() || !f.isDir())
  {
    int ret = KMessageBox::warningYesNo(this, i18n("Prefix must be an existing directory!\n\n"
        "Do you want to specify another prefix?"), i18n("Invalid prefix"));
    if(ret == KMessageBox::Yes)
    {
      return;
    }
    else
    {
      noPrefixSelected();
      return;
    }
  }

  KGlobal::dirs()->addPrefix(mPrefix->text());
  if(locate("data", "boson/pics/boson-startup-logo.png") == QString::null)
  {
    int ret = KMessageBox::warningYesNo(this, i18n("No data files were found in specified prefix!\n\n"
        "Do you want to another prefix?"), i18n("Invalid prefix"));
    if(ret == KMessageBox::Yes)
    {
      return;
    }
    else
    {
      noPrefixSelected();
      return;
    }
  }
  prefixSelected(mPrefix->text());
}

void BosonPrefixChecker::noPrefixSelected()
{
  mPrefixFound = false;
  accept();
}

void BosonPrefixChecker::prefixSelected(QString prefix)
{
  // Prefix is already added to KStandardDirs' list
  KConfig* cfg = kapp->config();
  cfg->setGroup("Files");
  cfg->writeEntry("Data prefix", prefix);

  mPrefixFound = true;
  accept();
}
