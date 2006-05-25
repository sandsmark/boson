/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "boufoloadingwidget.h"
#include "boufoloadingwidget.moc"

#include "../../bomemory/bodummymemory.h"

#include <klocale.h>

BoUfoLoadingWidget::BoUfoLoadingWidget()
    : BoUfoLoadingWidgetBase()
{
  // AB: support for QColor in boufodesigner sucks. better do it in code.
  mProgress->setStartColor(QColor(0, 0, 0));
  mProgress->setEndColor(QColor(222, 222, 222));

  mLastGLUpdate.start();
}

BoUfoLoadingWidget::~BoUfoLoadingWidget()
{
}

void BoUfoLoadingWidget::setMaxDuration(unsigned int max)
{
  mProgress->setRange(0.0, (double)max);
}

void BoUfoLoadingWidget::setDuration(unsigned int duration)
{
  mProgress->setValue((double)duration);
  updateGLWidget();
}

void BoUfoLoadingWidget::setCurrentTask(const QString& text)
{
  mLoadingTaskLabel->setText(text);
  // TODO
#if 0
  mLoadingLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ));
#endif
  updateGLWidget(true);
}

void BoUfoLoadingWidget::setCurrentSubTask(const QString& text)
{
  mLoadingSubTaskLabel->setText(text);
  updateGLWidget(); // AB: note: do NOT use force==true here
}

void BoUfoLoadingWidget::resetProgress()
{
  // reset everything
  setMaxDuration(100);
  setDuration(0);
  setCurrentTask("");
  setCurrentSubTask("");
}

void BoUfoLoadingWidget::updateGLWidget(bool force)
{
  // there is little point in updating the screen more often - all it does is
  // slowing the loading down (every repaint takes a few ms)
  const int maxFPS = 30;
  if(!force && mLastGLUpdate.elapsed() < 1000/maxFPS)
  {
    return;
  }
  emit signalUpdateGL();
  mLastGLUpdate.start();
}

/*
 * vim: et sw=2
 */
