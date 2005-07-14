/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOAPPLICATION_H
#define BOAPPLICATION_H

#include <kapplication.h>

/**
 * A slightly extended @ref KApplication. This class should be used for all
 * programs of the boson project.
 * @short A slightly extended @ref KApplication.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoApplication : public KApplication
{
public:
	BoApplication(bool allowStyles = true, bool enableGUI = true);
	virtual ~BoApplication();
};

#endif
