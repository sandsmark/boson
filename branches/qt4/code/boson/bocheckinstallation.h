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
#ifndef BOCHECKINSTALLATION_H
#define BOCHECKINSTALLATION_H

class QString;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCheckInstallation
{
public:
	/**
	 * Check the installation.
	 *
	 * This method is supposed to find out whether the data files are
	 * installed in the expected path. A check for a single file should be
	 * sufficient for this.
	 *
	 * @return An i18n'ed error string describing what went wrong (to be
	 * displayed in a message box for example), or QString::null if no
	 * problem was found.
	 **/
	QString checkInstallation();

};

#endif
