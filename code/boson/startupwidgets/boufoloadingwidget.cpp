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

#include "boufoloadingwidget.h"
#include "boufoloadingwidget.moc"

#include <klocale.h>

BoUfoLoadingWidget::BoUfoLoadingWidget()
    : BoUfoWidget()
{
  setLayoutClass(BoUfoWidget::UVBoxLayout);

  BoUfoLabel* header = new BoUfoLabel();
  addWidget(header);
  // TODO
#if 0
  QFont header_font(header->font());
  header_font.setFamily("verdana");
  header_font.setPointSize(30);
  header_font.setBold(TRUE);
  header->setPaletteForegroundColor( QColor( 255, 255, 255 ));
  header->setFont(header_font);
#endif
#if 0
  header->setAlignment( int( QLabel::AlignCenter ) );
#endif
  header->setText( i18n( "Initializing mission data..." ) );

  addSpacing(20);

  BoUfoLabel* pleaseWaitLabel = new BoUfoLabel();
  addWidget(pleaseWaitLabel);
  // TODO
#if 0
  pleaseWaitLabel->setPaletteForegroundColor(QColor(255, 255, 255));
#endif
  pleaseWaitLabel->setText(i18n("Please wait while Boson's data is being loaded. This may take some time..."));
  addSpacing(20);

  mLoadingTaskLabel = new BoUfoLabel();
  addWidget(mLoadingTaskLabel);

  mLoadingSubTaskLabel = new BoUfoLabel();
  addWidget(mLoadingSubTaskLabel);

  mProgress = new BoUfoProgress();
  addWidget(mProgress);
  mProgress->setValue(0.0);

  mProgress->setStartColor(QColor(0, 0, 0));
  mProgress->setEndColor(QColor(222, 222, 222));
  addSpacing(20);
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
  emit signalUpdateGL();
}

void BoUfoLoadingWidget::setCurrentTask(const QString& text)
{
  mLoadingTaskLabel->setText(text);
  // TODO
#if 0
  mLoadingLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ));
#endif
}

void BoUfoLoadingWidget::setCurrentSubTask(const QString& text)
{
  mLoadingSubTaskLabel->setText(text);
}

void BoUfoLoadingWidget::resetProgress()
{
  // reset everything
  setMaxDuration(100);
  setDuration(0);
  setCurrentTask("");
  setCurrentSubTask("");
}

/*
 * vim: et sw=2
 */
