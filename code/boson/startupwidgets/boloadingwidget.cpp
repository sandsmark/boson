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

#include "boloadingwidget.h"
#include "boloadingwidget.moc"

#include "../../bomemory/bodummymemory.h"

#include <klocale.h>

BoLoadingWidget::BoLoadingWidget(QWidget* parent)
    : QWidget(parent)
{
  mLastGLUpdate.start();
}

BoLoadingWidget::~BoLoadingWidget()
{
}

void BoLoadingWidget::setMaxDuration(unsigned int max)
{
  mProgress->setRange(0.0, (double)max);
}

void BoLoadingWidget::setDuration(unsigned int duration)
{
  mProgress->setValue((double)duration);
  updateGLWidget();
}

void BoLoadingWidget::setCurrentTask(const QString& text)
{
  mLoadingTaskLabel->setText(text);
  // TODO
#if 0
  mLoadingLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ));
#endif
  updateGLWidget(true);
}

void BoLoadingWidget::setCurrentSubTask(const QString& text)
{
  mLoadingSubTaskLabel->setText(text);
  updateGLWidget(); // AB: note: do NOT use force==true here
}

void BoLoadingWidget::resetProgress()
{
  // reset everything
  setMaxDuration(100);
  setDuration(0);
  setCurrentTask("");
  setCurrentSubTask("");
}

void BoLoadingWidget::updateGLWidget(bool force)
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
