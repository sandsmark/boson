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

#ifndef BOSONWELCOMEWIDGET_H
#define BOSONWELCOMEWIDGET_H

#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QPushButton;
class KBugReport;

class BosonWelcomeWidget : public QWidget
{
  Q_OBJECT
  public:
    BosonWelcomeWidget(QWidget* parent);
    ~BosonWelcomeWidget();

  signals:
    void signalNewGame();
    void signalLoadGame();
    void signalStartEditor();
    void signalQuit();

  protected slots:
    void slotReportBug();

  private:
    KBugReport* mBugReport;

};

#endif // BOSONWELCOMEWIDGET_H
