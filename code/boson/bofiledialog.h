/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOFILEDIALOG_H
#define BOFILEDIALOG_H

#include <config.h>

#if BOSON_LINK_STATIC
#include <q3filedialog.h>
#else
#include <kfiledialog.h>
#endif



/**
 * @short Small wrapper class for @ref KFileDialog and @ref QFileDialog
 * currently.
 *
 * In the future this might support BoUfo. Currently this class primarily exists
 * to replace all @ref KFileDialog calls by @ref QFileDialog calls when linking
 * a static binary.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoFileDialog
{
public:
	static QString getSaveFileName(const QString& startWith = QString::null,
			const QString& filter = QString::null,
			QWidget* parent = 0,
			const QString& caption = QString::null)
	{
#if BOSON_LINK_STATIC
		return Q3FileDialog::getSaveFileName(startWith, filter, parent, 0, caption);
#else
		return KFileDialog::getSaveFileName(startWith, filter, parent, caption);
#endif
	}

	static QString getOpenFileName(const QString& startWith = QString::null,
			const QString& filter = QString::null,
			QWidget* parent = 0,
			const QString& caption = QString::null)
	{
#if BOSON_LINK_STATIC
		return Q3FileDialog::getOpenFileName(startWith, filter, parent, 0, caption);
#else
		return KFileDialog::getOpenFileName(startWith, filter, parent, caption);
#endif
	}

	static QString getExistingDirectory(const QString& dir = QString::null,
			QWidget* parent = 0,
			const QString& caption = QString::null)
	{
#if BOSON_LINK_STATIC
		return Q3FileDialog::getExistingDirectory(dir, parent, 0, caption);
#else
		return KFileDialog::getExistingDirectory(dir, parent, caption);
#endif
	}
};

#endif

