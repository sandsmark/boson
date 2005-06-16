/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOUFOLOADINGWIDGET_H
#define BOUFOLOADINGWIDGET_H

#include <boufo/boufo.h>

class QString;

class BoUfoLoadingWidget : public BoUfoWidget
{
    Q_OBJECT
  public:
    BoUfoLoadingWidget();
    ~BoUfoLoadingWidget();

    void setMaxDuration(unsigned int maxDuration);
    void setDuration(unsigned int duration);
    void setCurrentTask(const QString& text);
    void setCurrentSubTask(const QString& text);

    void resetProgress();

  signals:
    void signalUpdateGL();
  private:
    BoUfoLabel* mLoadingTaskLabel;
    BoUfoLabel* mLoadingSubTaskLabel;
    BoUfoProgress* mProgress;
};

#endif // BOSONLOADINGWIDGET_H

/*
 * vim: et sw=2
 */
