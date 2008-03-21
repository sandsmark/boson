/*  This file is part of the Boson game. It was originally part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef BOABSTRACTDEBUGDIALOG__H
#define BOABSTRACTDEBUGDIALOG__H

#include <kdialog.h>

class KConfig;
class QVBoxLayout;

class BoAbstractDebugDialog : public KDialog
{
  Q_OBJECT
public:
  BoAbstractDebugDialog( QWidget *parent=0, const char *name=0, bool modal=true );

  virtual ~BoAbstractDebugDialog();

  virtual void buildButtons(QVBoxLayout * topLayout);

  virtual void save() = 0;
  KConfig * config() { return pConfig; }

protected slots:
  void slotShowHelp();
protected:
  KConfig* pConfig;
  QPushButton* pOKButton;
  QPushButton* pCancelButton;
  QPushButton* pHelpButton;
};

#endif
