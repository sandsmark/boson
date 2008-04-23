/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2006-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOLOADINGWIDGET_H
#define BOLOADINGWIDGET_H

#include "ui_loadingwidget.h"

#include <QWidget>
#include <QTime>

class QString;

class BoLoadingWidget : public QWidget, Ui::LoadingWidget
{
    Q_OBJECT
  public:
    BoLoadingWidget(QWidget* parent = 0);
    ~BoLoadingWidget();

    void setMaxDuration(unsigned int maxDuration);
    void setDuration(unsigned int duration);
    void setCurrentTask(const QString& text);
    void setCurrentSubTask(const QString& text);

    void resetProgress();

  signals:
    void signalUpdateGL();

  protected:
    void updateGLWidget(bool force = false);

  private:
    QTime mLastGLUpdate;
};

#endif

/*
 * vim: et sw=2
 */
